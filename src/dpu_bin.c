#include <dpu.h>

// dpu_bin.c exists to bundle the dpu binaries into the host
// ELF. It is compiled once for every header in include/dpu_bin
// in the provided Makefile, providing a linkage for the
// `dpu_incbin_t` declared by each respective header

// this approach means only the Makefile has to be aware of the
// build directory's structure

// this file is meant to be compiled in one of two ways
//  1.  With the -include argument supplying any one header
//      in include/dpu_bin
//  2.  With -D EXEC_NAME=<identifier>
//
// in either case, the path to the executable must be given

// _EXEC_NAME comes from any header file in include/dpu_bin
// -D EXEC_NAME=... overrides it
#ifndef EXEC_NAME
#ifdef _EXEC_NAME
#define EXEC_NAME _EXEC_NAME
#endif
#endif

#ifdef EXEC_NAME
#ifdef EXEC_PATH
DPU_INCBIN( EXEC_NAME , EXEC_PATH );
#else
#error "Path to dpu binary must be given via -D EXEC_PATH=<path-to-bin>"
#endif
#else
#error "dpu_incbin_t identifier as used in host source must be given via -D EXEC_NAME=<identifier>"
#endif