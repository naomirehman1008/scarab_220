#ifndef __PREF_BO__
#define __PREF_BO__

#include <stdio.h>

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



#endif
