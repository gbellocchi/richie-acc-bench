/* =====================================================================
 * Project:      System model
 * Title:        configs.h
 * Description:  Multi cluster scaling
 *
 * $Date:        8.2.2023
 * ===================================================================== */
/*
 * Copyright (C) 2022 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#ifndef __CONFIGS_H__
#define __CONFIGS_H__

/* ===================================================================== */

/* DSE parameters */

// System architecture
#define n_clusters                          1
#define n_hwpe_total                        4
#define n_hwpe_active                       n_hwpe_total

// - Workload dimension (DMA and COMPUTE)
// Typical values: [1: FINE-GRAINED, 16: COARSE-GRAINED, 64: MAX]
#define dim_exp_workload                    1

// - Length of job queue
#define exp_len_job_queue                   8

// - L2
#define l2_size_B                           128*1024*16 // bytes
#define n_l2_ports_phy                      1
#define n_l2_ports_virt                     1 // <= n_l2_ports_phy
#define l2_cl_port_id_offset                0 // Offset on L2 port starting from port 0 (optional, default: 0)

// - L2 image buffer
#define l2_img_rows                         128
#define l2_img_cols                         128
#define l2_img_dim                          l2_img_rows*l2_img_cols
#define l2_buffer_dim                       l2_img_dim

// - DMA 
#define n_dma_tx                            16 // Number of transfers (for one outer transfer)
#define dma_payload_dim                     l2_img_dim/n_dma_tx // Payload dimension

// - L1
#define l1_size_B                           128*1024 // bytes
#define n_l1_ports                          16

// - L1 tiled image buffer
// Decided on top of worst-case scenario (16 accelerators in 1 cluster)
#define l1_n_img_tiles                      16 // Number of repetitions of L1 buffer (to emulate bigger L1 memory)
#define l1_buffer_dim                       l2_buffer_dim/l1_n_img_tiles

// - L2/L1 optimizations
// #define l2_double_buffering
// #define l1_double_buffering

// Event unit
#define max_num_sw_evt                      8

/* ===================================================================== */

/* ========================== */
/*  L2 application profiling  */
/* ========================== */

/* Profiling types */

#define _profile_1cl_small_

/* Monitored components */

// #define _profile_dma_in_
// #define _profile_compute_
// #define _profile_dma_out_

/* ===================================================================== */

/* Performance counters */

#define _allocate_use_perf_event_cycle_
#define _allocate_perf_event_instr_retired_
// #define _allocate_perf_event_stall_load_
// #define _allocate_perf_event_load_
// #define _allocate_perf_event_store_
// #define _allocate_perf_event_load_external_
// #define _allocate_perf_event_store_external_

/* ===================================================================== */

/* PULP SDK custom configurations */

// #define _pulp_rt_
#define _pulp_bare_

/* ===================================================================== */

/* Other macros */

// #define PRINT_LOG
// #define DEBUG_SYNCH_EU
// #define INPUT_INIT

/* ===================================================================== */

#endif