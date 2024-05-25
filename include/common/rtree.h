
#ifndef RTREE_H
#define RTREE_H

#include <stddef.h>
#include <stdint.h>

#include <attributes.h>

#include "common/point.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mbr_t {
    ordpair_t min;
    ordpair_t max;
} mbr_t;

typedef struct node_t {
    mbr_t mbr;
    bool is_leaf; 
    size_t start; // first index of child nodes or leaves
    size_t nr_children;
} node_t;

typedef struct rtree_params_t {
    size_t max_children;
    size_t nr_leaves;
    size_t nr_nodes;
}

typedef struct query_t {
    mbr_t mbr;
    size_t nr_pts_found;
} query_t;

#ifdef __cplusplus
}
#endif

#endif