#ifndef __PREF_BO__
#define __PREF_BO__

#include <stdio.h>
#include "pref_common.h"

#include "../globals/global_types.h"

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

typedef struct Best_Offset_Struct {
  HWP_Info*             hwp_info;
  BestOffset_Table_Entry* stride_table;
  CacheLevel        type;
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
