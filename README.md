# PIM-RTree
RTree designed for UPMEM Processing-in-Memory Architecture.

The programs in this GitHub directory has been written by Leah Edwards, an undergraduate student and research assistant at Missouri S&T, under the guidance of Dr. Satish Puri.

This program takes a collection of ordered pairs and arranges them into an R-tree data structure. Two methods of sorting the geospatial data for bulk loading are considered here: z-ordering and placement on a hilbert curve. Hilbert curve implementation is not complete.

This project depends on the UPMEM SDK. To build this project, it is first required that you set up your environment by sourcing upmem-env.sh. You can then run make all.

The build directory will appear with a number of subdirectories. The host executable in build/bin has the DRAM Processing Unit (DPU) executables embedded within it and can be run standalone, but the DPU executables are available in build/bin/dpu if you are interested in running them with UPMEM's lldb.
