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

void pref_bo_ul1_pref_hit(uns8 proc_id, Addr line_addr, Addr load_PC,
                       uns32 global_hist, int lru_position, uns8 prefetcher_id) {
                       pref_bo_train(&bestoffset_prefetcher_array.bestoffset_hwp_core_ul1[proc_id], proc_id, lineAddr, loadPC, FALSE, TRUE);
                        // recent requests update 
                       };

void pref_umlc_pref_hit(uns8 proc_id, Addr line_addr, Addr load_PC,
                        uns32 global_hist, int lru_position,
                        uns8 prefetcher_id) {
                        pref_bo_train(&bestoffset_prefetcher_array.bestoffset_hwp_core_ul1[proc_id], proc_id, lineAddr, loadPC, FALSE, TRUE);
                        }

void pref_bo_ul1_miss(uns8 proc_id, Addr lineAddr, Addr , uns32 global_hist) {
  // to do 
  // l2 prefetcher, don't do anything?
  pref_bo_train(&bestoffset_prefetcher_array.bestoffset_hwp_core_ul1[proc_id], proc_id, lineAddr, loadPC, TRUE, FALSE);
}
void pref_bo_ul1_hit(uns8 proc_id, Addr lineAddr, Addr loadPC, uns32 global_hist) {
  // to do 
  // l2 prefetcher, don't do anything?
  pref_bo_train(&bestoffset_prefetcher_array.bestoffset_hwp_core_ul1[proc_id], proc_id, lineAddr, loadPC, FALSE, FALSE);
}
void pref_bo_umlc_miss(uns8 proc_id, Addr lineAddr, Addr loadPC, uns32 global_hist) {
  // to do
  pref_bo_train(&bestoffset_prefetcher_array.bestoffset_hwp_core_umlc[proc_id], proc_id, lineAddr, loadPC, TRUE, FALSE);
}
void pref_bo_umlc_hit(uns8 proc_id, Addr lineAddr, Addr loadPC, uns32 global_hist) {
  // to do
  pref_bo_train(&bestoffset_prefetcher_array.bestoffset_hwp_core_umlc[proc_id], proc_id, lineAddr, loadPC, FALSE, FALSE); 
}

void pref_bo_train(Pref_BO* bestoffset_hwp, uns8 proc_id, Addr lineAddr, Flag is_hit, Flag was_pref) {
  int ii;
  int best_offset_idx = -1;

  Addr lineIndex = lineAddr >> LOG2(DCACHE_LINE_SIZE);
  BestOffset_RR_Table_Entry* entry = NULL;

  int offset;

  // STEP 1: RR update
  // sus
  if (is_hit && was_pref) {  // train on prefetches at are accessed w demand load
    Addr rr_idx = ((lineAddr - bestoffset_hwp->cur_offset) >> LOG2(DCACHE_LINE_SIZE)) % PREF_BO_RR_TABLE_N;
    pref_bo_insert_to_rr_table(rr_idx, line_addr, bestoffset_hwp->bo_table);
  }
  // STEP 2: Train score table
  // reset scores
  if (bestoffset_hwp->new_round) {
    bestoffset_hwp->new_round = FALSE;
    // reset score table
    pref_bo_reset_scores(bestoffset_hwp->score_table);
  }

  // Test current offset
  Addr candidateLine = lineAddr - bestoffset_hwp->round_offset;
  if (pref_bo_access_rr(bestoffset_hwp, line_addr)) {
    int * score = hash_lib_access(bestoffset_hwp->score_table, bestoffset_hwp->round_offset);
    (*score)++;
  }

  // check for max score
  int best_score = 0;
  int best_offset = 0;
  // potential optimization just check score for current offset
  for (int ii=0; ii<bestoffset_hwp->num_offsets; i++) {
    int offset_i = bestoffset_hwp->offset_list[ii];
    int * score = hash_lib_access(bestoffset_hwp->score_table, offset);
    if (*score > best_score) {
      best_score = *score;
      best_offset = offset_i;
    }
  }
  if(best_score >= MAXSCORE || bestoffset_hwp->round >= MAXROUND) {
    bestoffset_hwp->cur_offset = best_offset;
    new_round = TRUE;
    if(bestoffset_hwp->cur_offset < BADSCORE) {
      bestoffset_hwp->throttle = TRUE;
    }
  }

  bestoffset_hwp->offset_idx++;
  bestoffset_hwp->round_offset = bestoffset_hwp->offset_list[bestoffset_hwp->offset_idx]
  if(bestoffset_hwp->offset_idx + 1 == bestoffset_hwp->num_offsets) {
    bestoffset_hwp->round++;
  }

  // STEP 3 emit prefetches
  if(bestoffset_hwp->throttle)
    return;

  pref_addto_umlc_req_queue(proc_id, (lineAddr >> DCACHE_LINE_SIZE) + bestoffset_hwp->cur_offset, stride_hwp->hwp_info->id)
}

pref_bo_insert_to_rr_table(Addr rr_idx, Addr lineAddr, BestOffset_RR_Table * rr_table) {
  rr_table->entries[rr_idx]->lineAddr = lineAddr;
  rr_table->entries[rr_idx]->cycle_accessed = cycle_count;
  rr_table->entries[rr_idx]->valid = TRUE;
}

pref_bo_reset_scores(Pref_BO* bestoffset_hwp) {
  int* score;
  for (int ii=0; ii<bestoffset_hwp->num_offsets) {
    score = hash_table_access(bestoffset_hwp->score_table, bestoffset_hwp->offset_list[ii]);
    *score = 0;
  }
}




