#include <algorithm>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include <iostream>

#include "cache.h"

#define NUM_ENTRIES_HRT  1024
#define NUM_ENTRIES_PT   16
#define NUM_ENTRIES_CSHR 256

#define PT_UPDATE_SLOTS  10
#define PT_UPDATE_QUEUES 16
#define IFILTER_ENTRIES  16

namespace
{
std::map<CACHE*, std::vector<uint64_t>> last_used_cycles;
}

std::vector<uint8_t> hrt_table (NUM_ENTRIES_HRT);
std::vector<uint8_t> pt_table (NUM_ENTRIES_PT);

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
    std::cout << "Initializing ACIC replacement" << std::endl;
    // push to the pt_update_queue
    for (int i = 0; i < PT_UPDATE_QUEUES; i++)
    {
        pt_update_block pt_update_block;
        pt_update_queue.push(pt_update_block);
    }
}

// finding ifilter vitim block
uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
    std::cout << "Finding ACIC victim" << std::endl;

    // if set is 64 it is ifl

    // if ifl contender has greater reuse distance than l1i contender send set 64, way_num

    // if l1i cache contender has greater reuse distance send set of l1i, way_num

    // check if ifl is full first
    for (int i = 0; i < IFILTER_ENTRIES; i++)
    {
        // if ifl is not full push to ifl
        if (!ifilter_table[i].valid)
        {
            ifilter_table[i].tag = full_addr;
            ifilter_table[i].valid = true;
            // return the way that just got filled into ifl so new instruction can be placed in l1i
            return i;
        }
    }

    // if ifl is full, check if the entry is in the ifl
    for (int i = 0; i < IFILTER_ENTRIES; i++)
    {
        if (ifilter_table[i].tag == full_addr)
        {
            std::cout << "Entry found in ifl" << std::endl;
            // if the entry is in the ifl, hash ifl tag to get index of hrt
            uint32_t hrt_idx = ifilter_table[i].tag % NUM_ENTRIES_HRT;
            if (hrt_table[hrt_idx] < 16)
                hrt_table[hrt_idx]++;
            else
                hrt_table[hrt_idx] = 0;
        }
    }
    // once ifl full do stuff with the tables cshr and all

    return 0;
}

void CACHE::update_replacement_state(uint32_t triggering_cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{
    // update counter and stuff
    std::cout << "Updating ACIC replacement state" << std::endl;
}

void CACHE::replacement_final_stats() 
{
    
}
