
#ifndef HILBERT_H
#define HILBERT_H

#include <stddef.h>
#include <stdint.h>

#include "common/point.h"

#ifdef __cplusplus
extern "C" {
#endif

void rot(int n, ordpair_t *pt, ordpair_t rpt);

int64_t h_xy2d (const size_t hilbert_order, ordpair_t pt);

int compare (const void * lhs, const void * rhs);

#ifdef __cplusplus
}
#endif

#endif