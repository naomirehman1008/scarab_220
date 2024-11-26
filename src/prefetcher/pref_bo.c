#include <stdlib.h>

#include "pref_bo.h"
#include "pref.param.def"
#include "core.param.h"
#include "statistics.h"
#include "prefetcher/pref_bo.param.def"

bestoffset_prefetchers bestoffset_prefetcher_array;

int potentialBOs[] = [1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 15, 16, 18, 20, 24, 25, 27, 30, 32, 36, 40, 45, 48, 50, 54, 60, 64, 72, 75, 80, 81, 90, 96, 100, 108, 120, 125, 128, 135, 144, 150, 160, 162, 180, 192, 200, 216, 225, 240, 243, 250, 256];
int POTENTIAL_BOS_SIZE = len(potentialBOs); // should be 52

void pref_bo_init(HWP* hwp) {
  if(!PREF_BO_ON) 
    return;

  // no prefetchers prefetch to the dcache??

  if(PREF_UMLC_ON) {
    bestoffset_prefetcher_array.bestoffset_hwp_core_umlc        = (Pref_BO*)malloc(sizeof(Pref_BO) * NUM_CORES);
    bestoffset_prefetcher_array.bestoffset_hwp_core_umlc->type  = UMLC;
    init_bo_core(hwp, bestoffset_prefetcher_array.bestoffset_hwp_core_umlc);
  }
  // I don't think we're prefetching to the UL1?
  if(PREF_UL1_ON){
    bestoffset_prefetcher_array.bestoffset_hwp_core_ul1        = (Pref_BO*)malloc(sizeof(Pref_BO) * NUM_CORES);
    bestoffset_prefetcher_array.bestoffset_hwp_core_ul1->type  = UL1;
    init_bo_core(hwp, bestoffset_prefetcher_array.bestoffset_hwp_core_ul1);
  }
}

void init_bo_core(HWP* hwp, Pref_BO* bo_hwp_core) {
  uns8 proc_id;

  for(proc_id = 0; proc_id < NUM_CORES; proc_id++) {
    bp_hwp_core[proc_id].hwp_info     = hwp->hwp_info;
    bp_hwp_core[proc_id].stride_table = (BestOffset_Table_Entry*)calloc(
      PREF_BO_RR_TABLE_N, sizeof(BestOffset_Table_Entry));
  }
}

// UMLC
void pref_umlc_pref_hit(uns8 proc_id, Addr line_addr, Addr load_PC,
                        uns32 global_hist, int lru_position,
                        uns8 prefetcher_id) {
  pref_bo_emit_prefetch(&bestoffset_prefetcher_array.bestoffset_hwp_core_umlc[proc_id], line_addr, TRUE);
  pref_bo_train(&bestoffset_prefetcher_array.bestoffset_hwp_core_umlc[proc_id], line_addr, TRUE);
}

void pref_bo_umlc_miss(uns8 proc_id, Addr lineAddr, Addr loadPC, uns32 global_hist) {
  // to do
  pref_bo_emit_prefetch(&bestoffset_prefetcher_array.bestoffset_hwp_core_umlc[proc_id], line_addr, TRUE);
  pref_bo_train(&bestoffset_prefetcher_array.bestoffset_hwp_core_umlc[proc_id], line_addr, TRUE);
}

void pref_bo_umlc_hit(uns8 proc_id, Addr lineAddr, Addr loadPC, uns32 global_hist) {
  // to do
  pref_bo_emit_prefetch(&bestoffset_prefetcher_array.bestoffset_hwp_core_umlc[proc_id], line_addr, TRUE);
}

// UL1
void pref_bo_ul1_pref_hit(uns8 proc_id, Addr line_addr, Addr load_PC,
                        uns32 global_hist, int lru_position,
                        uns8 prefetcher_id) {
  pref_bo_emit_prefetch(&bestoffset_prefetcher_array.bestoffset_hwp_core_ul1[proc_id], line_addr, FALSE);
  pref_bo_train(&bestoffset_prefetcher_array.bestoffset_hwp_core_ul1[proc_id], line_addr, FALSE);
}

void pref_bo_ul1_miss(uns8 proc_id, Addr lineAddr, Addr , uns32 global_hist) {
  pref_bo_emit_prefetch(&bestoffset_prefetcher_array.bestoffset_hwp_core_ul1[proc_id], line_addr, FALSE);
  pref_bo_train(&bestoffset_prefetcher_array.bestoffset_hwp_core_ul1[proc_id], line_addr, FALSE);
}

void pref_bo_ul1_hit(uns8 proc_id, Addr lineAddr, Addr loadPC, uns32 global_hist) {
  pref_bo_emit_prefetch(&bestoffset_prefetcher_array.bestoffset_hwp_core_ul1[proc_id], line_addr, FALSE);
}

void pref_bo_train(Pref_BO* bestoffset_hwp, uns8 proc_id, Addr line_addr, Flag is_hit, Flag was_pref) {
  // Train score table
  // reset scores
  if (bestoffset_hwp->new_phase) {
    bestoffset_hwp->new_phase = FALSE;
    bestoffset_hwp->offset_idx = 0;
    bestoffset_hwp->round = 0;
    // reset score table
    pref_bo_reset_scores(bestoffset_hwp->score_table);
  }

  // Test current offset
  Addr candidateLine = line_addr - bestoffset_hwp->train_offset;
  if (pref_bo_access_rr(bestoffset_hwp, line_addr)) {
    // if found increment score
    int * score = hash_lib_access(bestoffset_hwp->score_table, bestoffset_hwp->round_offset);
    (*score)++;
    // if we reach maxscore use this as the new offset and start a new learning pphase
    if((*score) >= PREF_BO_MAXSCORE){
      bestoffset_hwp->cur_offset = best_offset;
      bestoffset_hwp->new_phase = TRUE;
    }
  }
  // if we haven't crossed max score but we've reached the max rounds find best offset
  if(bestoffset_hwp->new_phase == FALSE && bestoffset_hwp->round >= PREF_BO_MAXROUNDS) {
    new_round = TRUE;
    // check for max score
    int best_score = 0;
    int best_offset = 0;
    for (int ii=0; ii<bestoffset_hwp->num_offsets; i++) {
      int offset_i = potentialBOs[ii];
      int * score = hash_lib_access(bestoffset_hwp->score_table, offset);
      if ((*score) > best_score) {
        best_score = *score;
        best_offset = offset_i;
      }
    }
    bestoffset_hwp->cur_offset = best_offset;
    if(bestoffset_hwp->cur_offset < BADSCORE) {
      bestoffset_hwp->throttle = TRUE;
    }
  }

  bestoffset_hwp->offset_idx = (bestoffset_hwp->offset_idx + 1) % POTENTIAL_BOS_SIZE;
  bestoffset_hwp->round_offset = bestoffset_hwp->offset_list[bestoffset_hwp->offset_idx]
  if(bestoffset_hwp->offset_idx + 1 == bestoffset_hwp->num_offsets) {
    bestoffset_hwp->round++;
  }
}

void pref_bo_emit_prefetch(Pref_BO * bestoffset_hwp, Addr lineAddr, Flag is_umlc) {
  if(bestoffset_hwp->throttle)
    return;
  if(is_umlc)
    pref_addto_umlc_req_queue(proc_id, (lineAddr >> DCACHE_LINE_SIZE) + bestoffset_hwp->cur_offset, bestoffset_hwp->hwp_info->id);
  else
    pref_addto_ul1_req_queue(proc_id, (lineAddr >> DCACHE_LINE_SIZE) + bestoffset_hwp->cur_offset, bestoffset_hwp->hwp_info->id);
}

// line inserted into recent requests when prefetched line is inserted into UMLC 
// these are sus, test these
void pref_bo_umlc_pref_line_filled(uns proc_id, Addr line_addr) {
  pref_bo_insert_to_rr_table(&bestoffset_prefetcher_array.bestoffset_hwp_core_umlc[proc_id], line_addr);
}

void pref_bo_ul1_pref_line_filled(uns proc_id, Addr line_addr) {
  pref_bo_insert_to_rr_table(&bestoffset_prefetcher_array.bestoffset_hwp_core_ul1[proc_id], line_addr);
}

void pref_bo_insert_to_rr_table(Pref_BO * bestoffset_hwp, Addr line_addr) {
  Addr rr_idx = ((lineAddr - bestoffset_hwp->cur_offset) >> LOG2(DCACHE_LINE_SIZE)) % PREF_BO_RR_TABLE_N;
  bestoffset_hwp->rr_table->entries[rr_idx]->lineAddr = lineAddr;
  bestoffset_hwp->rr_table->entries[rr_idx]->cycle_accessed = cycle_count;
  bestoffset_hwp->rr_table->entries[rr_idx]->valid = TRUE;
}

void pref_bo_reset_scores(Pref_BO* bestoffset_hwp) {
  int* score;
  for (int ii=0; ii<bestoffset_hwp->num_offsets) {
    score = hash_table_access(bestoffset_hwp->score_table, bestoffset_hwp->offset_list[ii]);
    *score = 0;
  }
}




