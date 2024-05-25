#include <stdint.h>
#include <stdlib.h>

#include "common/hilbert.h"

void rot(int n, ordpair_t *pt, ordpair_t rpt) {
    if (rpt.y == 0) {
        if (rpt.x == 1) {
            pt->x = n-1 - pt->x;
            pt->y = n-1 - pt->y;
        }
        //Swap x and y
        int t  = pt->x;
        pt->x = pt->y;
        pt->y = t;
    }
}

// hilbert order must be a power of two
// divides grid into hilbert_order^2 cells
int64_t xy2d (const size_t hilbert_order, ordpair_t pt) {
    ordpair_t rpt;
    int s, d=0;
    for (s=hilbert_order/2; s>0; s/=2) {
        rpt.x = (pt.x & s) > 0;
        rpt.y = (pt.y & s) > 0;
        d += s * s * ((3 * rpt.x) ^ rpt.y);
        rot(hilbert_order, &pt, rpt);
    }
    return d;
}

// where lhs and rhs are ordpair_t *
int hil_compare (const void * lhs, const void * rhs) {
    int64_t ld, rd;
    const size_t order = 16;
    ld = xy2d(order, * (ordpair_t *) lhs);
    rd = xy2d(order, * (ordpair_t *) rhs);
    return (rd > ld) - (rd < ld);
}
