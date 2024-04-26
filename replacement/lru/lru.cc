#include <algorithm>
#include <cassert>
#include <map>
#include <vector>
#include <iostream>

#include "cache.h"

namespace
{
std::map<CACHE*, std::vector<uint64_t>> last_used_cycles;
}

void CACHE::initialize_replacement() { ::last_used_cycles[this] = std::vector<uint64_t>(NUM_SET * NUM_WAY); }

uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
  // std::cout << "Finding LRU victim for " << std::endl;
  auto begin = std::next(std::begin(::last_used_cycles[this]), set * NUM_WAY);
  auto end = std::next(begin, NUM_WAY);

  // Find the way whose last use cycle is most distant
  auto victim = std::min_element(begin, end);
  assert(begin <= victim);
  assert(victim < end);
  // if (NAME == "cpu0_L1I")
  //   std::cout << "L1I REPL " << " SET: " << set << " WAY: " << static_cast<uint32_t>(std::distance(begin, victim)) 
  //   << " IP: " << ip << " ADDR: " << full_addr << std::endl;
  return static_cast<uint32_t>(std::distance(begin, victim)); // cast protected by prior asserts
}

// Called by IFL
// bool CACHE::compare_victim(uint64_t ifl_tag, uint64_t l1i_tag,  bool iflFull, bool l1iFull)
// {
//     bool replIfl {true}; 

//     if (iflFull)
//     {
//         // place ifl in l1i so l1i is victim
//         replIfl = false;
//     }

//     std::cout << "Placing in " << ((replIfl) ? "IFL" : "L1I") << std::endl;

//     return replIfl;
// }

void CACHE::update_replacement_state(uint32_t triggering_cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{
  // Mark the way as being used on the current cycle
  if (!hit || access_type{type} != access_type::WRITE) // Skip this for writeback hits
    ::last_used_cycles[this].at(set * NUM_WAY + way) = current_cycle;
  // if (NAME == "cpu0_L1I")
  //   std::cout << "L1I UPDATE REPL " << "SET: " << set << " WAY: " << way 
  //   << " IP: " << ip << " ADDR: " << full_addr << std::endl;
}

void CACHE::replacement_final_stats() {}
