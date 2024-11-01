#ifndef __SHADOW_CACHE_H__
#define __SHADOQ_CACHE_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "globals/assert.h"
#include "globals/global_defs.h"
#include "globals/global_types.h"
#include "globals/global_vars.h"
#include "globals/utils.h"
//naomi stuff
void init_sc(int cache_size, int line_size);
int sc_find_line(Addr req_line_addr);
Flag sc_insert_line(Addr req_line_addr);
#ifdef __cplusplus
}
#endif
#endif