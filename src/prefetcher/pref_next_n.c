
#ifdef __cplusplus
extern "C" {
#endif
#include "debug/debug_macros.h"
#include "debug/debug_print.h"
#include "globals/global_defs.h"
#include "globals/global_types.h"
#include "globals/global_vars.h"

#include "globals/assert.h"
#include "globals/utils.h"
#include "op.h"

#include "core.param.h"
#include "dcache_stage.h"
#include "debug/debug.param.h"
#include "general.param.h"
#include "libs/cache_lib.h"
#include "libs/hash_lib.h"
#include "libs/list_lib.h"
#include "memory/memory.h"
#include "memory/memory.param.h"
#include "prefetcher/pref_next_n.h"
#include "pref_next_n.param.h"
#include "prefetcher/l2l1pref.h"
#include "prefetcher/pref.param.h"
#include "prefetcher/pref_common.h"
#include "statistics.h"

#ifdef __cplusplus
}
#endif

/**************************************************************************************/
/* Macros */
#define DEBUG(proc_id, args...) _DEBUG(proc_id, DEBUG_BO, ##args)

next_n_prefetchers next_n_prefetcher_array;


void pref_next_n_init(HWP* hwp) {
  STAT_EVENT(0, BO_PREF_INIT);
  if(!PREF_NEXT_N_ON)
    return;

  hwp->hwp_info->enabled = TRUE;
  if(PREF_UMLC_ON) {
    next_n_prefetcher_array.next_n_hwp_core_umlc        = (Pref_Next_N*)malloc(sizeof(Pref_Next_N) * NUM_CORES);
    next_n_prefetcher_array.next_n_hwp_core_umlc->type  = UMLC;
    init_next_n_core(hwp, next_n_prefetcher_array.next_n_hwp_core_umlc);
  }
  if(PREF_UL1_ON){
    next_n_prefetcher_array.next_n_hwp_core_ul1        = (Pref_Next_N*)malloc(sizeof(Pref_Next_N) * NUM_CORES);
    next_n_prefetcher_array.next_n_hwp_core_ul1->type  = UL1;
    init_next_n_core(hwp, next_n_prefetcher_array.next_n_hwp_core_ul1);
  }
}

void init_next_n_core(HWP* hwp, Pref_Next_N* next_n_hwp_core) {
  uns8 proc_id;
  for(proc_id = 0; proc_id < NUM_CORES; proc_id++) {
    next_n_hwp_core[proc_id].hwp_info     = hwp->hwp_info;
    next_n_hwp_core[proc_id].throttle = FALSE;

  }
}

// UMLC
void pref_next_n_umlc_pref_hit(uns8 proc_id, Addr line_addr, Addr load_PC,
                        uns32 global_hist) {
  if(!PREF_UMLC_ON) return;
  pref_next_n_emit_prefetch(&next_n_prefetcher_array.next_n_hwp_core_umlc[proc_id], line_addr, TRUE, proc_id);
}

void pref_next_n_umlc_miss(uns8 proc_id, Addr line_addr, Addr loadPC, uns32 global_hist) {
  if(!PREF_UMLC_ON) return;
  pref_next_n_emit_prefetch(&next_n_prefetcher_array.next_n_hwp_core_umlc[proc_id], line_addr, TRUE, proc_id);
}

void pref_next_n_umlc_hit(uns8 proc_id, Addr line_addr, Addr loadPC, uns32 global_hist) {
  if(!PREF_UMLC_ON) return;
  pref_next_n_emit_prefetch(&next_n_prefetcher_array.next_n_hwp_core_umlc[proc_id], line_addr, TRUE, proc_id);
}

// UL1
void pref_next_n_ul1_pref_hit(uns8 proc_id, Addr line_addr, Addr load_PC,
                        uns32 global_hist) {
  if(!PREF_UL1_ON) return;
  pref_next_n_emit_prefetch(&next_n_prefetcher_array.next_n_hwp_core_ul1[proc_id], line_addr, FALSE, proc_id);
}

void pref_next_n_ul1_miss(uns8 proc_id, Addr line_addr, Addr loadPC, uns32 global_hist) {
  if(!PREF_UL1_ON) return;
  pref_next_n_emit_prefetch(&next_n_prefetcher_array.next_n_hwp_core_ul1[proc_id], line_addr, FALSE, proc_id);
}

void pref_next_n_ul1_hit(uns8 proc_id, Addr line_addr, Addr loadPC, uns32 global_hist) {
  if(!PREF_UL1_ON) return;
  pref_next_n_emit_prefetch(&next_n_prefetcher_array.next_n_hwp_core_ul1[proc_id], line_addr, FALSE, proc_id);
}

void pref_next_n_emit_prefetch(Pref_Next_N * next_n_hwp, Addr line_addr, Flag is_umlc, uns8 proc_id) {
  if(next_n_hwp->throttle)
    return;
  STAT_EVENT(proc_id, PREF_NEXT_N_EMITTED);
  if(is_umlc){
    for (int ii=0; ii<(int)PREF_NEXT_N_DEGREE; ii++){
      pref_addto_umlc_req_queue(proc_id, (line_addr >> DCACHE_LINE_SIZE) + (ii + 1) * PREF_NEXT_N_N, next_n_hwp->hwp_info->id);
    }
  }
  else{
    for (int ii=0; ii<(int)PREF_NEXT_N_DEGREE; ii++){
      pref_addto_ul1req_queue(proc_id, (line_addr >> DCACHE_LINE_SIZE) + (ii + 1) * PREF_NEXT_N_N, next_n_hwp->hwp_info->id);
    }
  }
}

