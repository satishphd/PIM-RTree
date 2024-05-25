#ifndef READWRITE_H
#define READWRITE_H

#include <mram.h>

#define PRINT_ERROR(fmt, ...) printf("\033[0;31mERROR:\033[0m   "fmt"\n", ##__VA_ARGS__)

uint64_t load8B(uint32_t ptr_m, uint32_t idx, uint64_t* cache_w);

void store8B(uint64_t val, uint32_t ptr_m, uint32_t idx, uint64_t* cache_w);

uint32_t load4B(uint32_t ptr_m, uint32_t idx, uint64_t* cache_w);

void store4B(uint32_t val, uint32_t ptr_m, uint32_t idx, uint64_t* cache_w);

#endif