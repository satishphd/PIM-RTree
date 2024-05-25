#include <defs.h>
#include <mram.h>
#include <mutex.h>

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "common/point.h"

__mram_noinit char * buffer;
__mram_noinit size_t buffer_size;
__mram_noinit indexedpt_t parsed_pts;

MUTEX_INIT(pt_lock);
__host size_t pt_pos = 0;


// takes a buffer full of csv and converts it to points
// expects /([^\.\n]*\.\d+[^\d\.\n][^\.\n]*\.\d+\n){`NR_TASKLETS`,}/
int main() {
    const size_t W_BUFFER_SIZE = 512;
    __dma_aligned char w_buffer[W_BUFFER_SIZE];

    // magic number here is the minimum length i expect a line to be
    __dma_aligned indexedpt_t w_parsed[W_BUFFER_SIZE / 16];

    for (
        size_t i = me() * W_BUFFER_SIZE;
        i < buffer_size;
        i += NR_TASKLETS * W_BUFFER_SIZE
    ) {
        size_t start = 0;
        size_t populated_len = W_BUFFER_SIZE < (buffer_size - i) ? W_BUFFER_SIZE : (buffer_size - i);
        // dangerous approach if you don't know there'll be a newline soon
        if (i) {
            for ( ; buffer[i+start] != '\n'; start++) populated_len--;
        }
        if (i + W_BUFFER_SIZE < buffer_size) {
            for ( ; buffer[i+populated_len-1] != '\n'; populated_len++);
        }

        mram_read(&buffer[i+start], w_buffer, populated_len);
        bool x_or_y = false; //x if false, y if true
        size_t pt_nr = 0;
        for (
            size_t pos = 0;
            pos < populated_len;
            pos++
        ) {
            if (w_buffer[pos] == '.') {
                if (x_or_y) {
                    //need u32's but dataset only fits in u64
                    w_parsed[pt_nr].pt.y = (atol(w_buffer[pos+1]) >> 21);
                    pt_nr++;
                    x_or_y = false;
                } else {
                    w_parsed[pt_nr].pt.x = (atol(w_buffer[pos+1]) >> 21);
                    x_or_y = true;
                }
            }
        }
        mutex_lock(pt_lock);
        mram_write(w_parsed,&parsed_pts[pt_pos],sizeof(indexedpt_t) * pt_nr)
        pt_pos += pt_nr;
        mutex_unlock(pt_lock);
    }

    return 0;
}