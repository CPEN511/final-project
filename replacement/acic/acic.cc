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

#define PT_UPDATE_SLOTS  10
#define PT_UPDATE_QUEUES 16
#define IFILTER_ENTRIES  16

namespace
{
std::map<CACHE*, std::vector<uint64_t>> last_used_cycles;
}

std::vector<int> hrt_table (NUM_HRT_ENTRIES, 0);
std::vector<int> pt_table (NUM_ENTRIES_PT, 0);

class ifilter
{
    public:
        uint64_t tag;
        bool valid;
};

class CSHR
{
    public:
        uint64_t ifl_tag;
        uint64_t l1i_tag;
        bool valid;
        uint8_t lru;
};

class pt_update_entry
{
    public:
        uint8_t pt_idx;
        bool update_req;
};

class pt_update_block
{
    public:
        std::vector<pt_update_entry> pt_update_entries = std::vector<pt_update_entry> (PT_UPDATE_SLOTS);
};

std::queue<pt_update_block> pt_update_queue;

std::vector<CSHR> cshr_table (NUM_ENTRIES_CSHR);

std::vector<ifilter> ifilter_table (IFILTER_ENTRIES);

void CACHE::initialize_replacement() 
{
    ::last_used_cycles[this] = std::vector<uint64_t>(NUM_SET * NUM_WAY); 
    std::cout << "Initializing ACIC replacement" << std::endl;
    // std::fill(hrt_table.begin(), hrt_table.end(), 0); // fill hrt w 0s initially
    // std::fill(pt_table.begin(), pt_table.end(), 0); // fill hrt w 0s initially
    // push to the pt_update_queue
    // for (int i = 0; i < PT_UPDATE_QUEUES; i++)
    // {
    //     pt_update_block pt_update_block;
    //     pt_update_queue.push(pt_update_block);
    // }
}

// finding ifilter vitim block
uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
    uint32_t way {};
    std::cout << "Finding ACIC victim" << std::endl;

    if (set < 64) std::cout << "L1I Victim" << std::endl;
    else std::cout << "IFL Victim" << std::endl;

    // if set is 64 it is ifl
    // if ifl contender has greater reuse distance than l1i contender send set 64, way_num
    // if l1i cache contender has greater reuse distance send set of l1i, way_num
    auto begin = std::next(std::begin(::last_used_cycles[this]), set * NUM_WAY);
    auto end = std::next(begin, NUM_WAY);

    // Find the way whose last use cycle is most distant
    auto victim = std::min_element(begin, end);
    assert(begin <= victim);
    assert(victim < end);
    way = static_cast<uint32_t>(std::distance(begin, victim));
    
    return way;
}

// Called by IFL
bool CACHE::compare_victim(uint64_t tag)
{
    uint16_t hash = tag % NUM_HRT_ENTRIES;      // index for HRT table
    bool replIfl {true}; 
    int threshold = 2;
    uint32_t replSetNum {64};
    uint32_t replWayNum;

    std::cout << "HASH: " << hash << " HRT: " << hrt_table.at(hash) << " PT: " << pt_table.at(hrt_table.at(hash)) << " Threshold: " << threshold << std::endl;
    if (pt_table.at(hrt_table.at(hash)) >= threshold)
    {
        // place ifl in l1i so l1i is victim
        std::cout << "WHY HEREEEE" << std::endl;
        replIfl = false;
    }

    std::cout << "Placing in " << ((replIfl) ? "IFL" : "L1I") << std::endl;

    return replIfl;
}

void CACHE::update_replacement_state(uint32_t triggering_cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{
    // update counter and stuff
    std::cout << "Updating ACIC replacement state" << std::endl;
      // Mark the way as being used on the current cycle
    if (!hit || access_type{type} != access_type::WRITE) // Skip this for writeback hits
        ::last_used_cycles[this].at(set * NUM_WAY + way) = current_cycle;
}

void CACHE::replacement_final_stats() 
{
    
}
