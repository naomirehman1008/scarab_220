#include <stdlib.h>

#include "Pref_bo.h"
#include "pref.param.def"
#include "core.param.h"
#include "statistics.h"
#include "prefetcher//Pref_bo.param.def"

bestoffset_prefetchers bestoffset_prefetcher_array;
int POTENTIAL_BOS_SIZE = 52;

void init_pref_bo(HWP* hwp) {
  if(PREF_UMLC_ON) {
    bestoffset_prefetcher_array.bestoffset_hwp_core_umlc        = (Pref_BO*)malloc(sizeof(Pref_BO) * NUM_CORES);
    bestoffset_prefetcher_array.bestoffset_hwp_core_umlc->type  = UMLC;
    init_bp(hwp, bestoffset_prefetcher_array.bestoffset_hwp_core_umlc);
  }
  if(PREF_UL1_ON){
    bestoffset_prefetcher_array.bestoffset_hwp_core_ul1        = (Pref_BO*)malloc(sizeof(Pref_BO) * NUM_CORES);
    bestoffset_prefetcher_array.bestoffset_hwp_core_ul1->type  = UL1;
    init_bp(hwp, bestoffset_prefetcher_array.bestoffset_hwp_core_ul1);
  }
}

void init_pref_bo(HWP* hwp, Pref_BO* bp_hwp_core) {
  uns8 proc_id;

  for(proc_id = 0; proc_id < NUM_CORES; proc_id++) {
    bp_hwp_core[proc_id].hwp_info     = hwp->hwp_info;
    bp_hwp_core[proc_id].stride_table = (BestOffset_Table_Entry*)calloc(
      PREF_BO_RR_TABLE_N, sizeof(BestOffset_Table_Entry));
  }
}

void pref_bo_ul1_miss(uns8 proc_id, Addr lineAddr, Addr , uns32 global_hist) {
  // to do 
  // l2 prefetcher, don't do anything?
  pref_bo_train(&bestoffset_prefetcher_array.bestoffset_hwp_core_ul1[proc_id], proc_id, lineAddr, loadPC, TRUE);
}
void pref_bo_ul1_hit(uns8 proc_id, Addr lineAddr, Addr loadPC, uns32 global_hist) {
  // to do 
  // l2 prefetcher, don't do anything?
  pref_bo_train(&bestoffset_prefetcher_array.bestoffset_hwp_core_ul1[proc_id], proc_id, lineAddr, loadPC, FALSE);
}
void pref_bo_umlc_miss(uns8 proc_id, Addr lineAddr, Addr loadPC, uns32 global_hist) {
  // to do
  pref_bo_train(&bestoffset_prefetcher_array.bestoffset_hwp_core_umlc[proc_id], proc_id, lineAddr, loadPC, TRUE);
}
void pref_bo_umlc_hit(uns8 proc_id, Addr lineAddr, Addr loadPC, uns32 global_hist) {
  // to do
  pref_bo_train(&bestoffset_prefetcher_array.bestoffset_hwp_core_umlc[proc_id], proc_id, lineAddr, loadPC, FALSE); 
}

void pref_bo_train(Pref_BO* bestoffset_hwp, uns8 proc_id, Addr lineAddr, Flag is_hit) {
  int ii;
  int best_offset_idx = -1;
  bestoffset_hwp->potentialBOs = [1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 15, 16, 18, 20, 24, 25, 27, 30, 32, 36, 40, 45, 48, 50, 54, 60, 64, 72, 75, 80, 81, 90, 96, 100, 108, 120, 125, 128, 135, 144, 150, 160, 162, 180, 192, 200, 216, 225, 240, 243, 250, 256];

  Addr lineIndex = lineAddr >> LOG2(DCACHE_LINE_SIZE);
  BestOffset_RR_Table_Entry* entry = NULL;

  int offset;

  // STEP 1: RR update
  // sus
  if (is_hit) {  // ONLY train on hits.
    Addr rr_idx = (lineAddr >> LOG2(DCACHE_LINE_SIZE)) % PREF_BO_RR_TABLE_N;
    pref_bo_insert_to_rr_table(rr_idx, line_addr, bestoffset_hwp->bo_table);
  }
  // STEP 2: Train score table
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
//
  // Found the entry, start training with it.
  entry = &bestoffset_hwp->BestOffset_RR_Table[best_offset_idx];
  entry->last_access = cycle_count;

  // Iterate through possible offsets and test their validity.
  for (ii = 0; ii < POTENTIAL_BOS_SIZE; ii++) {
    offset = bestoffset_hwp->potentialBOs[ii];  // Each offset to test.

    // Check if lineAddr - offset has been accessed recently in the RR table.
    if (rr_idx - offset > 0 & recently_accessed_in_rr_table(rr_idx - offset)) {
      // If the offset is a good candidate, increment its score.
      entry->score_table[ii]++;
      if(entry->score_table[ii])
    }
  }

  // After testing all offsets in the list, determine the best one.
  int max_score = 0;
  int best_offset = -1;
  for (ii = 0; ii < POTENTIAL_BOS_SIZE; ii++) {
    if (entry->score_table[ii] > max_score) {
      max_score = entry->score_table[ii];
      best_offset = bestoffset_hwp->offset_list[ii];
    }
  }

  // If a valid offset is found, update it.
  if (best_offset != -1) {
    entry->best_offset = best_offset;
  }

  // If a good offset is found, issue prefetches using this offset.
  if (entry->best_offset != -1) {
    Addr pref_index = lineAddr + entry->best_offset;
    
    // Add the prefetch request to the appropriate queue.
    if (!pref_add_to_req_queue(proc_id, pref_index, bestoffset_hwp->hwp_info->id)) {
      return;  // If queue is full, stop prefetching.
    }

    // Update RR table with the base address that triggered the prefetch.
    if (is_hit) {
      insert_to_rr_table(entry->load_addr - entry->best_offset);
    }
  }

  // Determine if we should switch to a new learning phase based on score.
  if (max_score >= PREF_BESTOFFSETPC_SCOREMAX) {
    // Best offset found, continue training.
    entry->trained = TRUE;
  } else if (entry->score_table[best_offset_idx] < PREF_BESTOFFSETPC_BAD_SCORE) {
    // If no good offset is found, consider turning off prefetching.
    entry->trained = FALSE;
  }
  
  // Reset the score table at the end of the learning phase.
  if (entry->trained) {
    reset_score_table(entry);
  }

  // Update the last address to the current line address.
  entry->last_addr = lineAddr;
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




