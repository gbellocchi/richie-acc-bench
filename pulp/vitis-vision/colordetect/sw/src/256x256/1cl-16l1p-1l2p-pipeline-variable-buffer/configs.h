/* =====================================================================
 * Project:      System model
 * Title:        configs.h
 * Description:  Color detect DSE configuration.
 *
 * $Date:        8.2.2023
 * ===================================================================== */
/*
 * Copyright (C) 2023 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#ifndef __CONFIGS_H__
#define __CONFIGS_H__

/* =====================================================================
 * DSE parameters --> Benchmark
 * ===================================================================== */

// #define _profile_l1_baseline_
// #define _profile_l1_pipeline_
// #define _profile_l2_baseline_
#define _profile_l2_pipeline_single_cl_
// #define _profile_l2_pipeline_multi_cl_

// #define _implement_const_single_buffer_
#define _implement_variable_multi_buffer_

/* =====================================================================
 * DSE parameters --> Application
 * ===================================================================== */

// Pipeline stages IDs
#define RGB2HSV_CV_0_0    0
#define THRESHOLD_CV_0_1  1
#define ERODE_CV_0_2      2
#define DILATE_CV_0_3     3
#define DILATE_CV_0_4     4
#define ERODE_CV_0_5      5

/* =====================================================================
 * DSE parameters --> System
 * ===================================================================== */

// System architecture
#define n_clusters                          1

// Accelerator-rich
#define n_acc_total                         6
#define n_acc_active                        n_acc_total
#define n_acc_stages                        6 // Total number of processing stages

// Application
#define n_img                               4 // Number of input images to be processed
#define img_rows                            256 
#define img_cols                            256 
#define img_dim                             img_rows * img_cols

/* =====================================================================
 * DSE parameters --> L1
 * ===================================================================== */

// - L1
#define l1_size_B                           128*1024 // bytes, real dimension
#define l1_size_B_emulated                  256*1024 // bytes, emulated dimension (>=l1_size_B, must be a power of 2)
#define n_l1_ports                          4
#define l1_bank_stride                      1 // to emulate small port utilization (can also be done in HW reducing the number of ports)

// - L1 number of buffers (This depends on the type of optimization being adopted, e.g. pipelining, double buffering)
#define l1_n_buffers                        16

// L1 stored image tile
#define l1_img_rows                         64 // do not change, designed on L1=256kB (emulated)
#define l1_img_cols                         64 // do not change, designed on L1=256kB (emulated)
#define l1_img_tile                         ((int) (l1_size_B) / ((sizeof(uint32_t)) * (l1_n_buffers))) // designed on L1=128kB (real)
#define l1_n_buffer_reps                    ((int) ((l1_img_rows) * (l1_img_cols)) / (l1_img_tile))

// - L1 image buffer
#define l1_buffer_dim                       ((int) (l1_size_B) / (4 * (l1_n_buffers))) // Dimension of allocated buffer
                                    // This is also the dimension of the data tile executed by each processing stage of the cluster
                                    // Data are read row-by-row, so they can sequentialized with 1D DMA transfers

/* =====================================================================
 * DSE parameters --> L2
 * ===================================================================== */

// - L2
#define l2_size_B                           4*1024*1024 // bytes
#define n_l2_ports_phy                      1
#define n_l2_ports_virt                     1 // <= n_l2_ports_phy
#define l2_cl_port_id_offset                0 // Offset on L2 port starting from port 0 (optional, default: 0)

// - L2 multi-port
#define l2_n_cl_per_port                    ((int) (n_clusters) / (n_l2_ports_virt))
#define l2_n_bytes_per_port                 ((int) (l2_size_B) / (n_l2_ports_phy))
#define l2_n_words_per_port                 ((int) (l2_n_bytes_per_port) / (sizeof(uint32_t)))

// - L2 number of buffers (This depends on the type of optimization being adopted, e.g. pipelining, double buffering)
#define l2_n_buffers                        2 * (n_acc_stages + 1)

// - L2 image buffer
#define l2_buffer_dim                       l1_img_tile // Equals dimension of tile passed to L1 memory
#define l2_n_tiles                          ((int) (img_dim) / (l1_img_tile)) // Number of tiles that are passed to L1 memory

/* =====================================================================
 * DSE parameters --> Cluster peripherals
 * ===================================================================== */

// - DMA 
#define dma_payload_dim                     l1_img_tile // Payload dimension
#define dma_n_tx                            ((int) (img_dim) / (l1_img_tile)) // Number of transfers, designed on L1=128kB (real)

// Event unit
#define max_num_sw_evt                      8

/* =====================================================================
 * DSE parameters --> Runtime execution
 * ===================================================================== */

// - Length of job queue (cache warm-up)
#define exp_len_job_queue                   2

/* =====================================================================
 * Additional configurations
 * ===================================================================== */

/* Performance counters */

#define _allocate_use_perf_event_cycle_
#define _allocate_perf_event_instr_retired_
// #define _allocate_perf_event_stall_load_
// #define _allocate_perf_event_load_
// #define _allocate_perf_event_store_
// #define _allocate_perf_event_load_external_
// #define _allocate_perf_event_store_external_

/* PULP SDK custom configurations */

// #define _pulp_rt_
#define _pulp_bare_

/* Other macros */

// #define PRINT_LOG
// #define DEBUG_SYNCH_EU
// #define INPUT_INIT

#endif