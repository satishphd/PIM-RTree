#include <stddef.h>
#include <stdint.h>

#include <dpu.h>
#include <mram.h>

#include "common/params.h"

// round up to nearest multiple of 8, bitwise style
static inline size_t ceil8 (size_t size) {
    return ~(~(size + 0x07) | 0x07);
}

struct mram_heap_allocator_t {
    size_t totalAllocated;
};

typedef struct mram_allocation_t {
    size_t index;
    size_t size;
} mram_allocation_t;

void init_allocator(struct mram_heap_allocator_t* allocator) {
    allocator->totalAllocated = 0;
}

size_t mram_alloc(struct mram_heap_allocator_t* allocator, size_t size) {
    size_t ret = allocator->totalAllocated;
    allocator->totalAllocated += ceil8(size);
    if(allocator->totalAllocated > DPU_CAPACITY) {
        PRINT_ERROR("        Total memory allocated is %d bytes which exceeds the DPU capacity (%d bytes)!", allocator->totalAllocated, DPU_CAPACITY);
        exit(0);
    }
    return ret;
}

void copyToDPU(struct dpu_set_t dpu, uint8_t* hostPtr, size_t mramIdx, size_t size) {
    DPU_ASSERT(dpu_copy_to(dpu, DPU_MRAM_HEAP_POINTER_NAME, mramIdx, hostPtr, ceil8(size)));
}

void copyFromDPU(struct dpu_set_t dpu, size_t mramIdx, uint8_t* hostPtr, size_t size) {
    DPU_ASSERT(dpu_copy_from(dpu, DPU_MRAM_HEAP_POINTER_NAME, mramIdx, hostPtr, ceil8(size)));
}