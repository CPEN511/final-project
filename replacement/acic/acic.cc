#include <algorithm>
#include <cassert>
#include <map>
#include <vector>
#include <queue>

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

class CSHR
{
    public:
        uint32_t tag;
        bool valid;
        uint8_t lru;
};

class pt_update_entry
{
    public:
        uint8_t pt_idx;
        bool update_req;
};

std::vector<pt_update_entry> pt_update_block (PT_UPDATE_SLOTS);

std::queue<pt_update_block> pt_update_queue (PT_UPDATE_QUEUES);

std::vector<CSHR> cshr_table (NUM_ENTRIES_CSHR);

void CACHE::initialize_replacement() 
{

}

uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
    // till ifl is not full just keep filling it

    // once ifl full do stuff with the tables cshr and all
}

void CACHE::update_replacement_state(uint32_t triggering_cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{
    // update counter and stuff
}

void CACHE::replacement_final_stats() 
{
    
}
