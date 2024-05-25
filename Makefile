# VARIABLES
###############

NAME := rtree_load

SHELL := bash

UPMEM_LD_FLAGS = $(shell dpu-pkg-config --cflags --libs dpu)
DPU_CC := dpu-upmem-dpurte-clang
DPU_CC_FLAGS := 
HOST_CC := gcc
HOST_CC_FLAGS = --std=c99 -ldl -lm -fopenmp

HOST_OBJS := $(patsubst src/host/%.c,build/obj/host/%.o,$(wildcard host/*.c))
DPU_OBJS := $(patsubst src/dpu/%.c,build/obj/dpu/%.o,$(wildcard dpu/*.c))

DPU_BINS := $(patsubst src/dpu/%.c,build/bin/dpu/%,$(wildcard src/dpu/*.c))
DPU_BIN_OBJS := $(patsubst build/bin/dpu/%,build/obj/dpu_bin/%.o,${DPU_BINS})


# PHONY RULES
###############

.PHONY: clean all setup host dpu

all: host

dpu: ${DPU_BINS}

host: build/bin/${NAME}


setup:
	mkdir -p build/lib
	mkdir    build/bin
	mkdir -p build/obj/host
	mkdir    build/obj/dpu

clean:
	rm -rf build


# EXECUTABLES
###############

# dpu executables
build/bin/dpu/%: build/obj/dpu/%.o
	${DPU_CC} $^ ${DPU_CC_FLAGS} -o $@

# host executable
build/bin/${NAME}: ${HOST_OBJS} ${DPU_BIN_OBJS}
	${HOST_CC} ${HOST_CC_FLAGS} $^ -o $@ ${UPMEM_LD_FLAGS}


# OBJECT FILES
###############

# host objects
build/obj/host/%.o: host/%.c
	${HOST_CC} ${HOST_CC_FLAGS} $^ -c -o $@ ${UPMEM_LD_FLAGS}

# dpu objects
build/obj/dpu/%.o: dpu/%.c
	${DPU_CC} $^ ${DPU_CC_FLAGS} -c -o $@

# dpu bin objects
build/obj/dpu_bin/%.o: src/dpu_bin.c include/dpu_bin/%.h build/bin/dpu/%
	${HOST_CC} ${HOST_CC_FLAGS} -include include/dpu_bin/$*.h -D EXEC_PATH=build/bin/dpu/$* src/dpu_bin.c -c -o $@ ${UPMEM_LD_FLAGS}