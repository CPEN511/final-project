#include <algorithm>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include <iostream>

#include "cache.h"

#define NUM_HRT_ENTRIES  1024
#define NUM_ENTRIES_PT   16
#define NUM_ENTRIES_CSHR 256
#define HASH_TAG         16777216
#define HRT_MASK         15
#define PT_MASK          31

#define PT_UPDATE_SLOTS  10
#define PT_UPDATE_QUEUES 16

#define IFL_HIT 1
#define L1I_HIT 0

namespace
{
std::map<CACHE*, std::vector<uint64_t>> last_used_cycles;
}

std::vector<int> hrt_table (NUM_HRT_ENTRIES, 0);
std::vector<int> pt_table (NUM_ENTRIES_PT, 15);
class CSHR
{
    public:
        uint64_t ifl_tag;
        uint64_t l1i_tag;
        bool valid;
        uint8_t lru;
};

std::vector<CSHR> cshr_table (NUM_ENTRIES_CSHR);

void CACHE::initialize_replacement() 
{
    ::last_used_cycles[this] = std::vector<uint64_t>(NUM_SET * NUM_WAY); 
    std::cout << "Initializing ACIC replacement" << std::endl;
    for (int i = 0; i < NUM_ENTRIES_CSHR; i++)
    {
        cshr_table[i].valid = 0;                // make all valid elements 0 cuz we need to insert tags
        cshr_table[i].lru = 0;                  // also make lru the min val
    }
}

// finding ifilter vitim block
uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
    uint32_t way {};
    std::cout << "Finding ACIC victim" << std::endl;

    if (set < 64) std::cout << "L1I Victim" << std::endl;
    else std::cout << "IFL Victim" << std::endl;

    auto begin = std::next(std::begin(::last_used_cycles[this]), set * NUM_WAY);
    auto end = std::next(begin, NUM_WAY);
    auto victim = std::min_element(begin, end);
    assert(begin <= victim);
    assert(victim < end);
    way = static_cast<uint32_t>(std::distance(begin, victim));
    
    return way;
}

// Called by IFL
bool CACHE::compare_victim(uint64_t ifl_tag, uint64_t l1i_tag,  bool iflFull, bool l1iFull)
{
    
    bool replIfl {true}; 
    int threshold = 16;
    uint8_t smallestLru {cshr_table[0].lru};
    uint64_t ifl_tag_hashed = (ifl_tag>>OFFSET_BITS) % HASH_TAG;
    uint64_t l1i_tag_hashed = (l1i_tag>>OFFSET_BITS) % HASH_TAG;
    int smallestLru_idx {0};
    uint32_t replSetNum {64};
    uint32_t replWayNum;
    uint16_t hash = ifl_tag_hashed % NUM_HRT_ENTRIES;      // index for HRT table

    std::cout << "PTINDEX: " << (hrt_table.at(hash)) << " PT: " << pt_table.at(hrt_table.at(hash)) << " Threshold: " << threshold << std::endl;
    if (iflFull && !l1iFull) replIfl = false; // if l1i is not full we need to default to l1i
    else if (iflFull && l1iFull)
    {
        if (pt_table.at(hrt_table.at(hash)) >= threshold) replIfl = false;
    }

    // insert ifl tag and l1i tag into cshr
    for (int i = 0; i < NUM_ENTRIES_CSHR; i++)
    {
        // find the slot with least LRU value
        if (cshr_table[i].lru < smallestLru && cshr_table[i].valid == 0 && l1iFull && iflFull)
        {
            // std::cout << "Finding Smallest LRU index" << std::endl;
            smallestLru = cshr_table[i].lru;
            smallestLru_idx = i;
        }
    }

    // insert ifl and l1i tag into slot with least LRU value
    if (l1iFull && iflFull)
    {
        // std::cout << "Placing into CSHR" << std::endl;
        cshr_table[smallestLru_idx].ifl_tag = ifl_tag_hashed;
        cshr_table[smallestLru_idx].l1i_tag = l1i_tag_hashed;
        cshr_table[smallestLru_idx].valid = 1;
    }

    // if (iflFull && !l1iFull) replIfl = false; // if l1i is not full we need to default to l1i

    std::cout << "Placing in " << ((replIfl) ? "IFL" : "L1I") << std::endl;

    return replIfl;
}

void CACHE::update_replacement_state(uint32_t triggering_cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{
    // update counter and stuff
    uint16_t ifl_hash {};
    uint16_t l1i_hash {};
    int64_t full_addr_hashed = (full_addr>>OFFSET_BITS)%HASH_TAG;
    std::cout << "Updating ACIC replacement state" << std::endl;
      // Mark the way as being used on the current cycle
    if (!hit || access_type{type} != access_type::WRITE) // Skip this for writeback hits
        ::last_used_cycles[this].at(set * NUM_WAY + way) = current_cycle;

    for (int i = 0; i < NUM_ENTRIES_CSHR; i++)
    {
        if (full_addr_hashed == cshr_table[i].ifl_tag)
        {
            std::cout << "IFL tag hit" << std::endl;
            ifl_hash = cshr_table[i].ifl_tag % NUM_HRT_ENTRIES;
            //std::cout << "IFL tag hit1" << std::endl;
            hrt_table.at(ifl_hash) = ((hrt_table.at(ifl_hash) << 1) | (IFL_HIT)) & HRT_MASK;

            //std::cout << "IFL tag hit2" << std::endl;
            pt_table.at(hrt_table.at(ifl_hash)) = (pt_table.at(hrt_table.at(ifl_hash))+1) & PT_MASK;
            //std::cout << "IFL tag hit3" << std::endl;
            cshr_table[i].lru++;
            cshr_table[i].valid = 0;
            break;
        }
        else if (full_addr_hashed == cshr_table[i].l1i_tag)
        {
            std::cout << "L1I tag hit" << std::endl;
            l1i_hash = cshr_table[i].l1i_tag % NUM_HRT_ENTRIES;
            hrt_table.at(l1i_hash) = ((hrt_table.at(l1i_hash) << 1) | (L1I_HIT)) & HRT_MASK;
            pt_table.at(hrt_table.at(l1i_hash)) = (pt_table.at(hrt_table.at(l1i_hash))-1) & PT_MASK;
            cshr_table[i].lru++;
            cshr_table[i].valid = 0;
            break;
        }
    }
}

void CACHE::replacement_final_stats() 
{
    
}
