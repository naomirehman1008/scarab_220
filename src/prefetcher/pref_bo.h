#ifndef __PREF_BO__
#define __PREF_BO__

#include <stdio.h>
#include "pref_common.h"

#include "../globals/global_types.h"
#include "../libs/hash_lib.h"

#define GET_INDEX(addr, num_entries) addr % num_entries
#define GET_TAG(addr, num_entries, mask) (addr / num_entries) & mask

uns * potentialBOs = [1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 15, 16, 18, 20, 24, 25, 27, 30, 32, 36, 40, 45, 48, 50, 54, 60, 64, 72, 75, 80, 81, 90, 96, 100, 108, 120, 125, 128, 135, 144, 150, 160, 162, 180, 192, 200, 216, 225, 240, 243, 250, 256];

typedef struct BO_Table_Entry_Struct {
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
} BO_Table_Entry;

// may need more?
typedef struct BO_RR_Table_Entry_Struct {
  //maybe just to tag?
  Addr lineAddr;
  Counter cycle_accessed;
  Flag valid;
} BO_RR_Table_Entry;

typedef struct BO_Struct {
  int                     type;
  HWP_Info*               hwp_info;
  // static stuff
  CacheLevel              type;
  // training stuff
  Hash_Table *            score_table;
  Flag                    new_phase; // are we starting a new round
  int                     round;    // what round are we on
  int                     train_offset;   // what offset are we testing
  BO_RR_Table_Entry *     rr_table;
  // prefetching
  int                     cur_offset; // what round are we using currently
  int                     offset_idx;
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
