#include <vector>
#include <climits>

#include "shadow_cache.h"
#include "globals/assert.h"
#include "globals/global_defs.h"
#include "globals/global_types.h"
#include "globals/global_vars.h"
#include "globals/utils.h"

using namespace std;

typedef struct Shadow_Entry_struct {
    Flag valid;
    Addr line_addr;
    Counter cycle_accessed;
} Shadow_Entry;

vector<Shadow_Entry> shadow_cache;

void init_sc(int cache_size, int line_size) {
    shadow_cache.resize(cache_size / line_size);
    for (Shadow_Entry& entry : shadow_cache) {
        entry.valid = 0;
        entry.line_addr = 0;
        entry.cycle_accessed = 0;
    }
}

int sc_find_line(Addr req_line_addr) {
    for (uint i=0; i<shadow_cache.size(); i++) {
        if(shadow_cache[i].line_addr == req_line_addr)
            return (int)i;
    }
    return -1;
}

int sc_find_oldest() {
    Counter oldest_cycle = MAX_CTR;
    int oldest = -1;
    for(uint i=0; i<shadow_cache.size(); i++) {
        if(shadow_cache[i].valid) {
            if(shadow_cache[i].cycle_accessed < oldest_cycle){
                oldest_cycle = shadow_cache[i].cycle_accessed;
                oldest = (int)i;
            }
        }
        else {
            return (int)i;
        }
    }
    return oldest;
}

Flag sc_insert_line(Addr req_line_addr) {
    int entry_idx = sc_find_line(req_line_addr);
    // hit, update LRU
    if(entry_idx >= 0) {
        shadow_cache[entry_idx].cycle_accessed = cycle_count;
    }
    //miss, evict something and put the new line in
    else {
        entry_idx = sc_find_oldest();
        shadow_cache[entry_idx].line_addr = req_line_addr;
        shadow_cache[entry_idx].cycle_accessed = cycle_count;
        shadow_cache[entry_idx].valid = 1;
    }
    return 1;
}