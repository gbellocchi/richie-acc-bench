

PULP benchmarks
==================================
Here are collected five experiments that will guide you through the design of a matrix multiplication algorithm, at first implementing it as a software function, and then as hardware IP. Speaking of the hardware designs, through the use of HLS pragmas and software-based hardware component design, incremental hardware design optimizations are achievable. The projects compilation flow permits the generation of the RTL description, as well as a packetized IP version that can be subsequently integrated into a block-level design project in Xilinx Vivado.

This design exploits a double buffering technique (also known as the ping-pong buffer technique) to further improve performance. This is achieved by exploiting the accelerator prefetcher. Data blocks are continuously transferred from DRAM to a set of buffers (let's call them *receive buffers*), while, at the same time, the second set of buffers (let's call them *processing buffers*) already contain previously transferred data to fed the accelerator. The existence of two sets of buffers is compulsory to prevent data coherency problems. Implementing this technique on top of the same block-based hardware function of the previous example allows to effectively hide either the prefetching or execution times, thus allowing for increased performance levels.

## Proposed designs
Design | Description |
---------------|-----------------------|
[01_sw/][]|Matrix multiplication is performed on hardware without specifying any optimizations, thus the HLS code is the same as it would be if run as a software function. This results in default random data access and DMA transfers the data to BRAM blocks [1]. This experiment settles a baseline for the others below.|
[02_hw_baseline/][]|Data locality allows for improving performance. This design exploits a hardware prefetcher to transfer blocks of input data from DRAM to the instantiated accelerator local memory (implemented using BRAMs). The host processor is in charge of calculating the DRAM offsets for the prefetcher and control the accelerator execution.|
[03_hw_double_prefetching/][]|This design exploits array partitioning technique and pipeline kernel optimization on top of the same hardware function from previous example to achieve better performance. Array partitioning results in increased number of effective read/write ports for a local BRAM array by partitioning the array into smaller arrays [1]. Increase in number of read/write ports leads to parallel processing of the data elements. Moreover, pipelining a loop results in lower initiation interval (II), which is the number of clock cycles between the start times of consecutive loop iterations by allowing multiple iterations of a loop to run in parallel.|
[04_tcdm_parallelism/][]|This design demonstrates that performance are further improvable implementing both the calculation of the DRAM offsets and the control of accelerator execution with a hardware looper. The HLS code is the same as it would be for the loop to be implemented in software (as it happens for design *02_prefetch*.|

[.]:.
[01_sw/]:01_sw/
[02_hw_baseline/]:02_hw_baseline/
[03_hw_double_prefetching/]:03_hw_double_prefetching/
[04_tcdm_parallelism/]:04_tcdm_parallelism/

## Experimental results
Design | Clock cycles [ck] | Execution time [ms] | 
---------------|-----------------------|-----------------------|
[01_sw/][]|||
[02_hw_baseline/][]|||
[03_hw_double_prefetching/][]|||
[04_tcdm_parallelism/][]|||

[.]:.
[01_sw/]:01_sw/
[02_hw_baseline/]:02_hw_baseline/
[03_hw_double_prefetching/]:03_hw_double_prefetching/
[04_tcdm_parallelism/]:04_tcdm_parallelism/
  

# References
[1] Kurth, A., Capotondi, A., Vogel, P., Benini, L., & Marongiu, A. (2018, November). HERO: An open-source research platform for HW/SW exploration of heterogeneous manycore systems. In \textit{Proceedings of the 2nd Workshop on AutotuniNg and aDaptivity AppRoaches for Energy efficient HPC Systems} (pp. 1-6). 