/* =====================================================================
 * Project:      System model
 * Title:        configs.h
 * Description:  Multi cluster scaling
 *
 * $Date:        17.7.2022
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
#define n_hwpe_total                        1

// Event unit
#define max_num_sw_evt                      8

// - Workload dimension (DMA and COMPUTE)
// Typical values: [1: FINE-GRAINED, 16: COARSE-GRAINED, 64: MAX]
#define dim_exp_workload                    1

// - Length of job queue
#define exp_len_job_queue                   1

// - HWPE traffic generator
#define n_read_ports                        1
#define n_write_ports                       1

// - L2
#define l2_size_B                           128*1024*16 // bytes
#define n_l2_ports_phy                      1
#define n_l2_ports_virt                     1 // <= n_l2_ports_phy
#define l2_cl_port_id_offset                0 // Offset on L2 port starting from port 0 (optional, default: 0)
// #define l2_n_cl_per_port                    n_clusters/n_l2_ports_virt
//                                         // --> HW has 32 ports so that cluster DMAs 
//                                         //     do not use port 0 (cache interference)
// #define l2_n_bytes_per_port         l2_size_B/n_l2_ports_phy
// #define l2_n_words_per_port         l2_n_bytes_per_port/sizeof(uint32_t)
// #define l2_cluster_buffer_dim       (n_read_ports + n_write_ports) * l1_buffer_dim_stop

// - L1
#define l1_size_B                           128*1024 // bytes
#define n_l1_ports                          16
#define l1_fast_corner_detect_buffer_dim    (n_read_ports + n_write_ports) * l1_buffer_dim_stop
#define l1_arov_buffer_dim                  l1_fast_corner_detect_buffer_dim * n_hwpe_stop

// - DMA number of transfers (for one outer transfer)
#define n_dma_tx                            1

// - DMA payload dimension
#define dma_payload_dim_start               512*((int) (dim_exp_workload) / (n_dma_tx))
#define dma_payload_dim_stop                512*((int) (dim_exp_workload) / (n_dma_tx))

// - L1 circular buffer dimension
// Decided on top of worst-case scenario (16 accelerators in 1 cluster)
#define l1_buffer_dim_start                 128*128
#define l1_buffer_dim_stop                  128*128

// - Number of traffic generators
#define n_hwpe_start                        ((int) (n_hwpe_total) / (n_clusters))
#define n_hwpe_stop                         ((int) (n_hwpe_total) / (n_clusters))

// - Buffer reuse factor
#define n_reps_start                        1*dim_exp_workload
#define n_reps_stop                         1*dim_exp_workload

/* ===================================================================== */

/* ========================== */
/*  L2 application profiling  */
/* ========================== */

/* Profiling types */

#define _profile_l2_

/* Monitored components */

// #define _profile_dma_in_
// #define _profile_compute_
// #define _profile_dma_out_

/* ===================================================================== */

/* ========================== */
/*  L1 application profiling  */
/* ========================== */

/* Profiling types */

// #define _profile_l1_

/* ===================================================================== */

/* =============== */
/*  API profiling  */
/* =============== */

/* Profiling types */

// #define _profile_api_

/* Monitored components */

// #define _profile_arov_init_
// #define _profile_arov_map_params_fast_corner_detect_
// #define _profile_arov_activate_
// #define _profile_arov_program_
// #define _profile_arov_compute_
// #define _profile_arov_free_
// #define _profile_hero_memcpy_host2dev_async_
// #define _profile_hero_l3malloc_
// #define _profile_hero_l1malloc_
// #define _profile_hero_l3free_
// #define _profile_hero_l1free_

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
#define INPUT_INIT

/* ===================================================================== */

#endif