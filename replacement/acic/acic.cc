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
#define HASH_TAG         16777215
#define HRT_MASK         15
#define PT_MASK          31

#define PT_UPDATE_SLOTS  10
#define PT_UPDATE_QUEUES 16

#define TAG_SHIFT        12

#define IFL_HIT 1
#define L1I_HIT 0

namespace
{
std::map<CACHE*, std::vector<uint64_t>> last_used_cycles;
}

std::vector<unsigned int> hrt_table (NUM_HRT_ENTRIES, (unsigned int)0);
std::vector<unsigned int> pt_table (NUM_ENTRIES_PT, (unsigned int)16);
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
    //std::cout << "Initializing ACIC replacement" << std::endl;
    for (int i = 0; i < NUM_ENTRIES_CSHR; i++)
    {
        cshr_table[i].valid = false;                // make all valid elements 0 cuz we need to insert tags
        cshr_table[i].lru = 0;                  // also make lru the min val
    }
}

// finding ifilter vitim block
uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
    uint32_t way {};
    //std::cout << "Finding ACIC victim" << std::endl;


    auto begin = std::next(std::begin(::last_used_cycles[this]), set * NUM_WAY);
    auto end = std::next(begin, NUM_WAY);
    auto victim = std::min_element(begin, end);
    assert(begin <= victim);
    assert(victim < end);
    way = static_cast<uint32_t>(std::distance(begin, victim));

    
    // if (set < 64) std::cout << "L1I Victim: " << way << std::endl;
    // else std::cout << "IFL Victim: " << way << std::endl;
    
    return way;
}

// Called by IFL
bool CACHE::compare_victim(uint64_t ifl_tag, uint64_t l1i_tag,  bool iflFull, bool l1iFull)
{
    
    bool replIfl {true}; 
    unsigned int threshold = 16;
    uint8_t smallestLru {cshr_table[0].lru};
    uint64_t ifl_tag_hashed = (ifl_tag>>TAG_SHIFT);
    uint64_t l1i_tag_hashed = (l1i_tag>>TAG_SHIFT);
    int smallestLru_idx {0};
    uint16_t hash = ifl_tag_hashed % NUM_HRT_ENTRIES;      // index for HRT table

    // std::cout << "iflhashedtag: " << ifl_tag_hashed << " l1ihashedtag: " << l1i_tag_hashed << " address ifl: " << ifl_tag << " address l1i: " << l1i_tag << std::endl;
    // std::cout <<"HASH: " << hash << " PTINDEX: " << (hrt_table.at(hash)) << " PT: " << pt_table.at(hrt_table.at(hash)) << " Threshold: " << threshold << std::endl;

    if (iflFull && !l1iFull) replIfl = false; // if l1i is not full we need to default to l1i
    else if (iflFull && l1iFull)
    {
        if (pt_table.at(hrt_table.at(hash)) <= threshold) replIfl = false;
    }

    // insert ifl tag and l1i tag into cshr
    for (int i = 0; i < NUM_ENTRIES_CSHR; i++)
    {
        // find the slot with least LRU value
        if (cshr_table[i].lru < smallestLru && cshr_table[i].valid == false && l1iFull && iflFull)
        {
            // std::cout << "Finding Smallest LRU index" << std::endl;
            smallestLru = cshr_table[i].lru;
            smallestLru_idx = i;
        }
    }

    // insert ifl and l1i tag into slot with least LRU value
    if (l1iFull && iflFull)
    {
        //std::cout << "Placing into CSHR" << " IDX : " << smallestLru_idx <<std::endl;
        cshr_table[smallestLru_idx].ifl_tag = ifl_tag_hashed;
        cshr_table[smallestLru_idx].l1i_tag = l1i_tag_hashed;
        cshr_table[smallestLru_idx].valid = true;
        for (int i = 0; i < NUM_ENTRIES_CSHR; i++) cshr_table[i].lru--;
        cshr_table[smallestLru_idx].lru = (NUM_ENTRIES_CSHR-1);
    }

    // if (iflFull && !l1iFull) replIfl = false; // if l1i is not full we need to default to l1i

    //std::cout << "Placing in " << ((replIfl) ? "IFL" : "L1I") << std::endl;

    return replIfl;
}

void CACHE::update_replacement_state(uint32_t triggering_cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{
    // update counter and stuff
    uint16_t ifl_hash {};
    uint16_t l1i_hash {};
    uint64_t full_addr_hashed = (full_addr>>TAG_SHIFT);
    //std::cout << "Updating ACIC replacement state" << std::endl;
      // Mark the way as being used on the current cycle
    
    if (hit && access_type{type} == access_type::WRITE){
        // Skip this for writeback hits
        //std::cout << "Writeback hit" << std::endl;
    }
    ::last_used_cycles[this].at(set * NUM_WAY + way) = current_cycle;
    if (victim_addr == 2){
        return;
    }
    if (victim_addr != 0 && victim_addr!=2 && !hit){
        for (int i = 0; i < NUM_ENTRIES_CSHR; i++)
        {
            if (full_addr_hashed == cshr_table[i].ifl_tag)
            {
                //std::cout << "IFL tag hit" << std::endl;
                ifl_hash = cshr_table[i].ifl_tag % NUM_HRT_ENTRIES;
                //std::cout << "IFL tag hit1" << std::endl;
                hrt_table.at(ifl_hash) = ((hrt_table.at(ifl_hash) << 1) + (unsigned int)1) & HRT_MASK;
                pt_table.at(hrt_table.at(ifl_hash)) = (pt_table.at(hrt_table.at(ifl_hash))+ (unsigned int)1) % PT_MASK;

                for (int j = 0; j < NUM_ENTRIES_CSHR; j++) cshr_table[j].lru--;
                cshr_table[i].lru = (NUM_ENTRIES_CSHR-1);
                cshr_table[i].valid = false;
                break;
            }
            else if (full_addr_hashed == cshr_table[i].l1i_tag)
            {
                //std::cout << "L1I tag hit" << std::endl;
                l1i_hash = cshr_table[i].ifl_tag % NUM_HRT_ENTRIES;
                hrt_table.at(l1i_hash) = ((hrt_table.at(l1i_hash) << 1)) & HRT_MASK;
                pt_table.at(hrt_table.at(l1i_hash)) = (pt_table.at(hrt_table.at(l1i_hash))-(unsigned int)1) % PT_MASK;
                for (int j = 0; j < NUM_ENTRIES_CSHR; j++) cshr_table[j].lru--;
                cshr_table[i].lru = (NUM_ENTRIES_CSHR-1);
                cshr_table[i].valid = false;
                break;
            } 
        }
    }
    
}

void CACHE::replacement_final_stats() 
{
    
}
