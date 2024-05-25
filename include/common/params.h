#ifndef PARAMS_H
#define PARAMS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

const size_t DPU_CAPACITY;
const size_t NUM_PTS;
const size_t NODE_CHILD_CAP;
const size_t LEAF_ELEM_CAP;
const size_t SLICE_WIDTH;
const size_t MAX_NR_DPUS;
#define BUFFER_SIZE 1 << 10 // DPUs have 64KB WRAM, say 1/32 is reserved for buffer

#ifdef __cplusplus
}
#endif

#endif