#ifndef __PREF_BO__
#define __PREF_BO__

#include <stdio.h>
#include "pref_common.h"

#include "../globals/global_types.h"
#include "../libs/hash_lib.h"

#define GET_INDEX(addr, num_entries) addr % num_entries
#define GET_TAG(addr, num_entries, mask) (addr / num_entries) & mask

typedef struct BestOffset_Table_Entry_Struct {
  Flag trained;
  Flag valid;

  Addr last_addr;
  Addr load_addr;
  Addr start_index;
  Addr pref_last_index;
  int  stride;

  Counter train_num;
  Counter pref_sent;
  Counter last_access;  // for lru
} BestOffset_Table_Entry;

// may need more?
typedef struct BestOffset_RR_Table_Entry_Struct {
  //maybe just to tag?
  Addr lineAddr;
  Counter cycle_accessed;
  Flag valid;
} BestOffset_RR_Table_Entry;

typedef struct Best_Offset_Struct {
  HWP_Info*               hwp_info;
  BestOffset_Table_Entry* bo_table;
  Hash_Table *            score_table;
  int                     num_offsets;
  uint *                  offsets;
  Flag                    new_round; // are we starting a new round
  int                     round;    // what round are we on
  int                     test_offset;   // what offset are we testing
  int                     cur_offset; // what round are we using currently
  CacheLevel              type;
  BestOffset_Recent_Requests_Table_Entry * rr_table;
  int                     offset_idx;
  int potentialBOs[52];
  Flag                    throttle;
} Pref_BO;

typedef struct {
  Pref_BO* bestoffset_hwp_core_ul1;
  Pref_BO* bestoffset_hwp_core_umlc;
} bestoffset_prefetchers;

/********************************************/

void init_pref_bo(HWP* hwp);

void pref_bo_ul1_miss(uns8 proc_id, Addr lineAddr, Addr loadPC,
                          uns32 global_hist);
void pref_bo_ul1_hit(uns8 proc_id, Addr lineAddr, Addr loadPC,
                         uns32 global_hist);
void pref_bo_umlc_miss(uns8 proc_id, Addr lineAddr, Addr loadPC,
                          uns32 global_hist);
void pref_bo_umlc_hit(uns8 proc_id, Addr lineAddr, Addr loadPC,
                         uns32 global_hist);
/*************************************************************/
/* Internal Function */
void init_bo(HWP* hwp, Pref_Stride* stride_hwp);
void pref_bo_train(Pref_Stride* stride_hwp, Addr lineAddr, Addr loadPC, Flag is_hit);
/*************************************************************/
/* Misc functions */
void pref_bo_create_newentry(Pref_Stride* stride_hwp, int idx, Addr line_addr, Addr region_tag);
#endif
