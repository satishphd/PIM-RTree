
#ifndef MRAM_MGMT_H
#define MRAM_MGMT_H

#include "../support/common.h"
#include "../support/utils.h"

const size_t DPU_CAPACITY;

struct mram_heap_allocator_t {
    uint32_t totalAllocated;
};

void init_allocator(struct mram_heap_allocator_t* allocator);

uint32_t mram_heap_alloc(struct mram_heap_allocator_t* allocator, size_t size);

void copyToDPU(struct dpu_set_t dpu, uint8_t* hostPtr, size_t mramIdx, size_t size);

void copyFromDPU(struct dpu_set_t dpu, size_t mramIdx, uint8_t* hostPtr, size_t size);


#endif

