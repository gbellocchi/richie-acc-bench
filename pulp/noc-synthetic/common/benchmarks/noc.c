/* =====================================================================
 * Project:      Color detect
 * Title:        l2_pipeline_mcl_const_tile.c
 * Description:  Implementation of a multi-cluster accelerator-rich vision  
 *               pipeline using PULP clusters. Each accelerator is given a 
 *               constant buffer dimension, even when spreading accelerators  
 *               in different clusters. This means the L1 dimension is 
 *               homogeneously distributed when scaling at multi-cluster,
 *               with no increase in size at system-level.
 *
 * $Date:        24.2.2023
 * ===================================================================== */
/*
 * Copyright (C) 2023 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#include <configs.h>
#include <list_benchmarks.h>

#if (BENCHMARK_TYPE == NOC)

#include <experiment.h>
#include <cluster_synch.h>

#include <stimuli.h>

#include "noc.h"

// -------------------------------------------- //

/* NoC SW reference */

void noc_sw_ref(
  uint32_t traffic_dim,
  noc_id src_id,
  noc_id dst_id
) {
  uint32_t local_buffer[traffic_dim];
  uint32_t base_addr = RICHIE_FLOONOC_VERIF_BASE_ADDR;

  // Read
  for (int i = 0; i < traffic_dim; i++) {
    local_buffer[i] = NOC_READ(base_addr, xy_to_int(dst_id) * RICHIE_FLOONOC_OFFS_ADDR + i * sizeof(uint32_t));
  }

  // Compute
  for (int i = 0; i < traffic_dim; i++) {
    local_buffer[i] = i + xy_to_int(src_id);
  }

  // Write
  for (int i = 0; i < traffic_dim; i++) {
    NOC_WRITE(base_addr, xy_to_int(dst_id) * RICHIE_FLOONOC_OFFS_ADDR + i * sizeof(uint32_t), local_buffer[i]);
  }
}

// -------------------------------------------- //

/* NoC DUT */

void noc_dut() {
  // Source ID
  noc_id src_id_xy, dst_id_xy;
  int src_id_int, dst_id_int;
  uint64_t src_addr, dst_addr;

  // Operational intensity
  uint32_t traffic_dim = TRAFFIC_CFG_SIZE;
  uint32_t compute_dim = COMPUTE_CFG_SIZE;

  // Loop over source X coordinate
  hw_test_src_x: for (src_id_xy.x = 0; src_id_xy.x < NOC_N_TILES_X; src_id_xy.x++) {
    // Loop over source Y coordinate
    hw_test_src_y: for (src_id_xy.y = 0; src_id_xy.y < NOC_N_TILES_Y; src_id_xy.y++) {
      
      // Check all paths toward other tiles
      hw_test_check_tile_paths: for (int dst_id_int = 0; dst_id_int < NOC_N_TILES; dst_id_int++) {

        int src_id_int = xy_to_int(src_id_xy);

        // Destination cannot correspond to source
        if(src_id_int == dst_id_int){
          ;
        } else {

          // Calculate destination as integer
          dst_id_xy = int_to_xy(dst_id_int);

          // Source address calculation
          src_addr = RICHIE_FLOONOC_BASE_ADDR + src_id_int * RICHIE_FLOONOC_OFFS_ADDR;

          // Destination address calculation
          dst_addr = RICHIE_FLOONOC_BASE_ADDR + dst_id_int * RICHIE_FLOONOC_OFFS_ADDR;

          // Launch test
          noc_dut_program(traffic_dim, compute_dim, src_id_xy, dst_id_xy, src_addr, dst_addr);
          noc_dut_execute(src_id_xy, src_addr);
          noc_dut_wait_termination(src_id_xy, src_addr);
        }
      }
    }
  }
}

// -------------------------------------------- //

void run_benchmark(const int cluster_id, const int core_id) {

  /* Runtime IDs */

  int buffer_id; // Buffer selector
  int run_id; // Execution run

  /* Runtime indicators */

  int n_busy_buffer_in;
  int n_busy_buffer_out;

  /* Define cache stats */

  int hit[2], trns[2], miss[2];
  int reg_hit, reg_trns, reg_miss;

  /* Cycle counters */

  pulp_clk_struct t_experiment_sys_pov;

  /* ===================================================================== */

  /* Initialize L2 memory */

  // Parameters
  unsigned l2_cl_port_id = cluster_id/(l2_n_cl_per_port + l2_cl_port_id_offset); // Calculate port ID (Optional: L2 cluster port offset)

  // Declare L2 cluster base address  
  DEVICE_PTR_CONST l2_cl_base = (l2_cl_port_id==0) ? \
                                  arov_l2_heap() : \
                                    // bank 0 holds also the program, so buffers are allocated starting from the heap
                                  arov_l2_base() + l2_cl_port_id * l2_n_bytes_per_port;
                                    // same as the HW of the SoC bus 

  // Declare L2 cluster buffer address  
  DEVICE_PTR_CONST l2_cl_addr = l2_cl_base + (cluster_id - (l2_cl_port_id - l2_cl_port_id_offset) * l2_n_cl_per_port) * dma_payload_dim;

  // Declare L2 image buffers
  DEVICE_PTR l2_img[l2_n_buffers];

  for(int i_buffer=0; i_buffer<l2_n_buffers; i_buffer++){
    l2_img[i_buffer] = (!i_buffer) ? l2_cl_addr : l2_cl_addr + i_buffer * l2_buffer_dim;
  }

  #ifdef INPUT_INIT
    // Initialize input buffer with input image
    for(int i=0; i<l2_buffer_dim; i++) pulp_write32(l2_img_a+i*sizeof(int32_t), in_img_small[i]);
  #endif

  /* ===================================================================== */

  /* Initialize L1 memory */

  // Declare L1 image buffers
  DEVICE_PTR l1_img[l1_n_buffers];

  for(int i_buffer=0; i_buffer<l1_n_buffers; i_buffer++){
    l1_img[i_buffer] = (!i_buffer) ? arov_l1_heap(cluster_id) : arov_l1_heap(cluster_id) + i_buffer * l1_buffer_dim;
  }

  /* ===================================================================== */

  /* System */

  pulp_dma_struct dma_in[2], dma_out[2], dma_wait[2];

  /* Accelerators */

  // Custom registers
  unsigned rows, cols;

  int offload_id[n_acc_active];

  /* Allocate accelerator-rich overlay */

  arov_struct arov;

  /* ===================================================================== */

  /* Cluster steady state condition */
  
  cluster_barrier_all_eu_soc_evt(cluster_id, 0, 0xFFFFFFFF);
  if(!cluster_id) cluster_slv_all_restart_eu_soc_evt(cluster_id, 0);

  /* ===================================================================== */

  /* Flush cache (cold-cache condition) */

  icache_flush_all();

  /* Launch profiling jobs */

  for(int job_id=0; job_id<exp_len_job_queue; job_id++){

    /* ===================================================================== */

    /* Initialize counters */

    // Reset and start PULP counter
    hero_reset_clk_counter();
    hero_start_clk_counter();

    if(cluster_id==0 && core_id==0){

      // Reset cache stats
      icache_stats_reset();

      reg_hit   = 0;
      reg_trns  = 0;
      reg_miss  = 0;

      // Reset performance counters
      hero_perf_reset_all();

    }

    /* ===================================================================== */

    /* MEASUREMENT - START */

    // cache statistics and performance counters
    start_measurement(cluster_id, core_id, hit, trns, miss);

    // Cluster synchronization barrier
    cluster_barrier_all_eu_soc_evt(cluster_id, 0, 0xFFFFFFFF); /* -- CMD_TYPE: STAGE INVOCATION -- */ 
    if(!cluster_id) cluster_slv_all_restart_eu_soc_evt(cluster_id, 0);

    // Cluster timer
    if(!cluster_id) t_experiment_sys_pov.cnt_0 = hero_get_clk_counter();

    /* ===================================================================== */

    /*  Profiling -   */

    // Source ID
    noc_id src_id_xy, dst_id_xy;
    int src_id_int, dst_id_int;
    uint64_t src_addr, dst_addr;

    // NoC DUT
    printf("NoC DUT using PULP cluster\n");
    noc_dut();

    /* ===================================================================== */

    /* MEASUREMENT - END */

    // Cluster synchronization barrier
    cluster_barrier_all_eu_soc_evt(cluster_id, 0, 0xFFFFFFFF); /* -- CMD_TYPE: STAGE INVOCATION -- */ 

    // Cluster timer
    if(!cluster_id) t_experiment_sys_pov.cnt_1 = hero_get_clk_counter();

    // Cache statistics and performance counters
    stop_measurement(cluster_id, core_id, hit, trns, miss);

    // Update measured cache stats
    reg_hit  += hit[1] - hit[0];
    reg_trns += trns[1] - trns[0];
    reg_miss += miss[1] - miss[0];

    /* ===================================================================== */

    /* Print statistics */

    print_job_stats(
      // System
      cluster_id, 
      core_id, 
      // Experiment
      0, 
      job_id, 
      BENCHMARK_NAME, 
      // Cache
      &reg_hit,
      &reg_trns, 
      &reg_miss, 
      // Clock counters
      0xFFFFFFFF,
      &t_experiment_sys_pov
    );

    /* Restart clusters */

    if(!cluster_id) cluster_slv_all_restart_eu_soc_evt(cluster_id, 0);

    /* ===================================================================== */

  } // job_id

  /* Cleaning L1 */
  
  #if defined(_pulp_rt_)
    hero_l1free(l1_base_address);
  #endif
}

#endif