#include <stdbool.h>

#include <defs.h>
#include <stdlib.h>
#include <barrier.h>
#include <mutex.h>
#include <mram.h>
#include <alloc.h>

#include "common/rtree.h"
#include "common/params.h"
#include "common/point.h"

__mram_noinit mbr_t query;

__mram_noinit_keep indexedpt_t points[BUFFER_SIZE];
__mram_noinit_keep node_t nodes[NR_TASKLETS * 2];
__mram_noinit_keep rtreeparams_t rparams;

MUTEX_INIT(result_write);
indexedpt_t __mram_ptr * results = DPU_MRAM_HEAP_POINTER;
__host size_t nr_results;

__dma_aligned node_t w_nodes[NR_TASKLETS * 2];

MUTEX_INIT(work_claim);
int work_queue[NR_TASKLETS];
size_t work_queue_crsr = 0;

int main() {
    node_t * node = NULL;
    size_t in_progess = 1;
    indexedpt_t * w_results = 
        mem_alloc(rparams.max_children * sizeof(indexedpt_t));

    if (me() == 0) {
        mram_read(nodes, w_nodes, NR_TASKLETS * 2 * sizeof(node_t));
        node = &w_nodes[rparams.nr_nodes - 1];
    } else {
        work_queue[me()] = -1;
    }

    while (in_progress || work_queue_crsr >= 0) {
        
        mutex_lock(work_claim);
        {
            if (work_queue_crsr >= 0) {
               node = &w_nodes[work_queue[work_queue_crsr--]];
               in_progress++;
            }
        }
        mutex_unlock(work_claim);
        
        if (node != NULL) {
            if ( 
                (node->mbr.min.x <= query.max.x && node->mbr.min.y <= query.max.y) ||
                (node->mbr.max.x >= query.min.x && node->mbr.max.y <= query.min.y)
            ) {
                if (node->is_leaf) {
                    mram_read(
                        &points[node_start],
                        w_results,
                        node->nr_children * sizeof(indexedpt_t)
                    );
                    size_t nr_matches = 0;
                    for (
                        size_t i = 0;
                        i < node->nr_children;
                        i++
                    ) {
                        if (
                            w_results[i].pt.x <= query.mbr.max.x &&
                            w_results[i].pt.x >= query.mbr.min.x &&
                            w_results[i].pt.y <= query.mbr.max.y &&
                            w_results[i].pt.y >= query.mbr.min.y
                        ) {
                            w_results[nr_matches++] = w_results[i];
                        }
                    }
                    if (nr_matches) {
                        size_t write_to;

                        mutex_lock(result_write);
                        {
                            write_to = nr_results;
                            nr_results += nr_matches;
                        }
                        mutex_unlock(result_write);

                        mram_write(w_results, &results[write_to], nr_matches*sizeof(indexedpt_t));
                    }
                    node = NULL;

                    mutex_lock(work_claim);
                    {
                        in_progess--;
                    }
                    mutex_unlock(work_claim);
                } else {
                    mutex_lock(work_claim);
                    {
                        for (size_t i = 1; i < node->nr_children; i++)
                           work_queue[work_queue_crsr++] = node->start + i;
                    }
                    mutex_unlock(work_claim);

                    node = &w_nodes[node->start];
                }
            }

        }
    }

    return 0;
}