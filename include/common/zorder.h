#ifndef _ZORDER_H_
#define _ZORDER_H_

#include <stdint.h>

#include "common/point.h"

#ifdef __cplusplus
extern "C" {
#endif

uint64_t z_xy2d(const ordpair_t xy);

#ifdef __cplusplus
}
#endif

#endif