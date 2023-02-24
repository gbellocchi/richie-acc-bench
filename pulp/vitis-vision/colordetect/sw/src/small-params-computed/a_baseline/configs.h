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

#define SUM(a, b) ((float)a+b)
#define DIV(a, b) ((float)a/b)
#define ROUND_UP(N, S) (((((int)N) + ((int)S) - 1) / ((int)S)) * ((int)S))
#define ROUND_DOWN(N,S) (((int)N / (int)S) * (int)S)

/* =====================================================================
 * DSE parameters --> Application
 * ===================================================================== */

// Application version
#define _profile_baseline_

// Application optimizations
// #define l2_pipelining
// #define l1_pipelining
// #define l2_double_buffering
// #define l1_double_buffering

/* =====================================================================
 * DSE parameters --> L1
 * ===================================================================== */

// - L1
#define l1_size_B                           128*1024 // bytes, real dimension
#define l1_size_B_emulated                  256*1024 // bytes, emulated dimension (>=l1_size_B, must be a power of 2)
#define n_l1_banks                          4
#define l1_bank_stride                      1 // to emulate small port utilization (can also be done in HW reducing the number of ports)

// - L1 number of buffers (This depends on the type of optimization being adopted, e.g. pipelining, double buffering)
#ifndef l1_pipelining
#define l1_n_buffers                        2
#else
#ifndef l1_double_buffering
#define l1_n_buffers                        n_acc_stages+1
#else
#define l1_n_buffers                        2*(n_acc_stages+1)
#endif
#endif

// - L1 image buffer
#define l1_buffer_dim                       ((int) (l1_size_B) / (4 * l1_n_buffers)) // Dimension of allocated buffer
                                    // This is also the dimension of the data tile executed by each processing stage of the cluster
                                    // Data are read row-by-row, so they can sequentialized with 1D DMA transfers
#define l1_n_buffer_reps                    ((img_dim == img_tile) ? 1 : ((int) (l1_size_B_emulated) / (l1_size_B))) // Number of repetitions of L1 buffer  
                                    // This parameter is used to emulate bigger L1 memory by:
                                        // 1. Looping the HWPE streamer over the same L1 buffer
                                        // 2. Looping the DMA over the same L1 buffer
                                        // 3. Affecting the dimensioning of the L2 data tile

/* =====================================================================
 * DSE parameters --> System
 * ===================================================================== */

// System architecture
#define n_clusters                          1

// Accelerator-rich
#define n_acc_total                         4
#define n_acc_active                        n_acc_total
#define n_acc_stages                        6 // Total number of processing stages

// Application
#define n_img                               4 // Number of input images to be processed
#define img_rows                            128 
#define img_cols                            128 
#define img_dim                             img_rows*img_cols // SMALL: 128x128, FHD: 1920x1080, 4K: 3840x2160
#define img_tile                            ((l1_buffer_dim >= img_dim) ? (img_dim) : (l1_buffer_dim))
#define img_rows_min                        128
#define img_cols_min                        128

/* =====================================================================
 * DSE parameters --> L2
 * ===================================================================== */

// - L2
#define l2_size_B                           512*1024 // bytes
#define n_l2_ports_phy                      1
#define n_l2_ports_virt                     1 // <= n_l2_ports_phy
#define l2_cl_port_id_offset                0 // Offset on L2 port starting from port 0 (optional, default: 0)

// - L2 multi-port
#define l2_n_cl_per_port                    ((int) (n_clusters) / (n_l2_ports_virt))
#define l2_n_bytes_per_port                 ((int) (l2_size_B) / (n_l2_ports_phy))
#define l2_n_words_per_port                 ((int) (l2_n_bytes_per_port) / (sizeof(uint32_t)))

// - L2 number of buffers (This depends on the type of optimization being adopted, e.g. pipelining, double buffering)
#ifndef l2_pipelining
#define l2_n_buffers                        2
#else
#ifndef l2_double_buffering
#define l2_n_buffers                        n_acc_stages+1
#else
#define l2_n_buffers                        2*(n_acc_stages+1)
#endif
#endif

// - L2 image buffer
#define l2_buffer_dim                       img_tile // Equals dimension of tile passed to L1 memory
#define l2_n_tiles                          ((int) (img_dim) / ((img_tile) * (l1_n_buffer_reps))) // Number of tiles that are passed to L1 memory

/* =====================================================================
 * DSE parameters --> Cluster peripherals
 * ===================================================================== */

// - DMA 
#define dma_payload_dim                     img_tile // Payload dimension
#define dma_n_tx                            l1_n_buffer_reps // Number of transfers (for one outer transfer)

// Event unit
#define max_num_sw_evt                      8

/* =====================================================================
 * DSE parameters --> Runtime execution
 * ===================================================================== */

// - Length of job queue (cache warm-up)
#define exp_len_job_queue                   1

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

#define PRINT_LOG
// #define DEBUG_SYNCH_EU
// #define INPUT_INIT

#endif