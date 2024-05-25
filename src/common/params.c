#include <stddef.h>
#include <stdint.h>

const size_t DPU_CAPACITY   = (64 << 20); // A DPU's capacity is 64 MiB

const size_t NUM_PTS        = 65536;

const size_t NODE_CHILD_CAP = 2;
const size_t LEAF_ELEM_CAP  = 2;

const size_t SLICE_WIDTH    = NUM_PTS / LEAF_ELEM_CAP;

const size_t MAX_NR_DPUS    = 512;

