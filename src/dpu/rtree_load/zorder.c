#include <defs.h>
#include <mram.h>
#include <barrier.h>

#include <stdlib.h>
#include <stdbool.h>

#include "common/rtree.h"
#include "common/point.h"
#include "common/zorder.h"
//#include "common/params.h"

__host size_t buffer_size;

BARRIER_INIT(barrier_all, NR_TASKLETS);

// Note: DPUs are 32b native. if possible, u32's would be more performant
__mram_noinit rtreeparams_t rparams;

__mram_noinit indexedpt_t * buffer;
__dma_aligned indexedpt_t * buffer_w;
__dma_aligned indexedpt_t * sorted_w;

__mram_noinit node_t nodes[NR_TASKLETS * 2];

const size_t NR_PIVOTS = NR_TASKLETS * NR_TASKLETS;
uint64_t pivots[NR_PIVOTS];

// index in buffer of [first index]th partition in bucket [second index]
size_t part_idx[NR_TASKLETS][NR_TASKLETS];
size_t part_size[NR_TASKLETS];

const size_t PTS_PER_TASKLET = buffer_size / NR_TASKLETS;
const uint8_t OVERFLOW = buffer_size % NR_TASKLETS;

int comparept(const void * a, const void * b) {
    return ((indexedpt_t *) a)->idx > ((indexedpt_t *) b)->idx;
}

int compare64(const void * a, const void * b) {
    return *((uint64_t *) a) > *((uint64_t *) b);
}

void swap (void * a, void * b, size_t type_size) {
    do {
        uint8_t tmp = *a;
        *a = *b;
        *b = *tmp;
    } while (--type_size < 0);
}

void merge(void * list, size_t lo, size_t mid, size_t hi, size_t end, size_t working,
    size_t type_size, bool (*cmp) (const void *, const void *)
) {
    while (lo < mid && hi < end) {
        if (cmp(&list[hi*type_size], &list[lo*type_size])) {
            swap(&list[working*type_size], &list[lo*type_size], type_size);
            working++; lo++;
        } else {
            swap(&list[working*type_size], &list[hi*type_size], type_size);
            working++; hi++;
        }
    }
    while (lo < mid) {
        swap(&list[working*type_size], &list[lo*type_size], type_size);
        working++; lo++;
    }
    while (hi < end) {
        swap(&list[working*type_size], &list[hi*type_size], type_size);
        working++; hi++;
    }
}  

void merge_sort (
    void* list, size_t lo, size_t hi, size_t type_size,
    bool (*cmp) (const void *, const void *)
);

void wsort (
    void * list, size_t lo, size_t hi, size_t working, size_t type_size,
    bool (*cmp) (const void *, const void *)
) {
    size_t mid;
    if (hi - lo > 1) {
        mid = (hi + lo) / 2;
        merge_sort(list, lo, mid, type_size, cmp);
        merge_sort(list, mid, hi, type_size, cmp);
        merge(list, lo, mid, mid, hi, working, type_size, cmp);
    }
    else {
        while (lo < hi) {
            swap(&list[lo*type_size], &list[working*type_size], type_size);
            lo++; working++;
        }
    }
}

void merge_sort (
    void* list, size_t lo, size_t hi, size_t type_size,
    bool (*cmp) (const void *, const void *)
) {
    size_t mid, n, working;
    if (hi - lo > 1) {
        mid = (hi + lo) / 2;
        working = lo + hi - mid;
        wsort(list, lo, mid, working, type_size, cmp);
        while (working - lo > 2) {
            n = working;
            working = (n + lo + 1) / 2;
            wsort(list, working, n, lo, type_size, cmp);  
            merge(list, lo, lo + n - working, n, hi, working, type_size, cmp);
        }
        for (n = working; n > lo; --n) {
            for (mid = n; mid < hi && cmp(&list[(mid-1)*type_size], &list[mid*type_size]); ++mid) {
                swap(&list[(mid-1)*type_size], &list[mid*type_size], type_size);
            }
        }
    }
}

int main() {
    const size_t me();
    const uint8_t HANDLE_OVERFLOW = me() < OVERFLOW;
    const size_t BUCKET_START = me() * PTS_PER_TASKLET + min(me(), OVERFLOW);

    mram_read(buffer, buffer_w, sizeof(buffer));
    
    // convert buffer to zorder indices
    for (
        size_t i = BUCKET_START;
        i < BUCKET_START + PTS_PER_TASKLET + HANDLE_OVERFLOW;
        i+=PTS_PER_TASKLET
    ) {
        buffer[i].idx = z_xy2d((ordpair_t) buffer[i].pt);
    }

    barrier_wait(&barrier_all);

    // sort buckets
    qsort(
        &buffer_w[BUCKET_START],
        PTS_PER_TASKLET + HANDLE_OVERFLOW,
        sizeof(indexedpt_t), 
        comparept
    );

    // select pivots in buckets
    const size_t PIVOT_STEP = PTS_PER_TASKLET / NR_TASKLETS;
    for (
        size_t i = me() * NR_TASKLETS, j = BUCKET_START;
        i < (me() * NR_TASKLETS) + NR_TASKLETS;
        i++
    ) {
        pivots[i] = buffer_w[j].idx;
        j += PIVOT_STEP;
    }

    barrier_wait(&barrier_all);

    // sort the pivots
    if (me() == 0) {
        qsort(pivots, NR_PIVOTS, sizeof(uint64_t), compare64);
    }

    barrier_wait(&barrier_all);

    // partition the buckets
    for (
        size_t i = BUCKET_START, part_no = 0;
        i < BUCKET_START + PTS_PER_TASKLET + HANDLE_OVERFLOW;
        i++;
    ) {
        if (buffer_w[i].idx > pivots[NR_TASKLETS * (part_no + 1)])
        {
            part_idx[part_no][me()] = i;

            if (part_no == 0) part_size[part_no] = i - BUCKET_START;
            else part_size[part_no] = i - part_idx[part_no-1][me()];

            part_no++;
        }
    }

    barrier_wait(&barrier_all);

    // merge and concat partitions
    size_t sorted_idx = 0;
    for (size_t i = 0; i+1 < me(); i++) sorted_idx += part_size[i];

    size_t buck_part_idx[NR_TASKLETS];
    size_t buck_part_bound[NR_TASKLETS];
    for (size_t i = 0; i<NR_TASKLETS; i++) {
        buck_part_idx[i] = part_idx[me()][i];
        buck_part_bound[i] = 
            (me() == NR_TASKLETS-1) ?
            bucket * PTS_PER_TASKLET + min(bucket, OVERFLOW) :
            part_idx[me()+1][bucket] ;
    }

    const size_t PART_BOUND = sorted_idx + part_size[me()];
    while (sorted_idx < PART_BOUND) {
        uint64_t minval = 0;
        size_t mindex;
        for (
            size_t bucket = 0;
            bucket < NR_TASKLETS;
            bucket++
        ) {
            if (buck_part_idx[bucket] < buck_part_bound[bucket]) {
                if (buffer_w[part_buck_idx[bucket]].idx <= minval) {
                    minval = buffer_w[buck_part_idx[bucket]].idx;
                    mindex = bucket;
                }
            }
        }
        sorted_w[sorted_idx] = buffer_w[part_buck_idx[mindex]];
        buck_part_idx[mindex]++;
        sorted_idx++;
    }

    node_t leafnode;
    leafnode.start = BUCKET_START;
    leafnode.is_leaf = true;
    leafnode.mbr.min = sorted_w[BUCKET_START].pt;
    leafnode.mbr.max = sorted_w[BUCKET_START].pt;
    for (
        size_t i = BUCKET_START+1;
        i < BUCKET_START + PTS_PER_TASKLET + HANDLE_OVERFLOW;
        i++
    ) {
        if (sorted_w[i].pt.x < leafnode.mbr.min.x) leafnode.mbr.min.x = sorted_w[i].pt.x;
        else if (sorted_w[i].pt.x > leafnode.mbr.max.x) leafnode.mbr.max.x = sorted_w[i].pt.x;
        if (sorted_w[i].pt.y < leafnode.mbr.min.y) leafnode.mbr.min.y = sorted_w[i].pt.y;
        else if (sorted_w[i].pt.y > leafnode.mbr.max.y) leafnode.mbr.max.y = sorted_w[i].pt.y;
    }
    nodes[me()] = leafnode;
    
    size_t parents_start = NR_TASKLETS;
    size_t nodes_in_level = 
        NR_TASKLETS / rparams.max_children 
        + (NR_TASKLETS % rparams.max_children > 0);
    size_t idx = (me() * rparams.max_children);
    while (idx < parents_start) {
        node_t parent;
        parent.start = idx;
        parent.is_leaf = false;
        parent.mbr = nodes[idx].mbr;
        size_t i;
        for (
            i = 1;
            i < rparams.max_children
            && (idx+i) < parents_start;
            i++
        ) {            
            if (nodes[idx + i].mbr.min.x < parent.mbr.min.x) {
                parent.mbr.min.x = nodes[idx + i].mbr.min.x;
            }
            if (nodes[idx + i].mbr.max.x > parent.mbr.max.x) {
                parent.mbr.max.x = nodes[idx + i].mbr.max.x;
            }
            if (nodes[idx + i].mbr.min.y < parent.mbr.min.y) {
                parent.mbr.min.y = nodes[idx + i].mbr.min.y;
            }
            if (nodes[idx + i].mbr.max.y > parent.mbr.max.y) {
                parent.mbr.max.y = nodes[idx + i].mbr.max.y;
            }
        }
        parent.nr_children = i;
        nodes[parents_start+me()] = parent;

        idx = parents_start + (me() * rparams.max_children);
        parents_start += nodes_in_level;
        if (nodes_in_level != 1) {
            nodes_in_level /= (nodes_in_level / rparams.max_children)
                + (nodes_in_level % rparams.max_children) > 0;
        } else {
            break; // edge case for root node
        }
    }
    rparams.nr_nodes = parents_start;

    barrier_wait(&barrier_all);

    mram_write(sorted_w, buffer, sizeof(buffer));
}