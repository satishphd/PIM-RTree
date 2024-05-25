#ifndef POINT_H
#define POINT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


//TODO: consider changing type of x and y
//      dpu's are 32 bit and this produces a 64b z index
typedef struct OrdPair {
    uint32_t x;
    uint32_t y;
} ordpair_t;

typedef struct indexedpt_t {
    ordpair_t pt;
    uint64_t idx;
} indexedpt_t;

#ifdef __cplusplus
}
#endif

#endif