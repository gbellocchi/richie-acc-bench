/* =====================================================================
 * Project:      Color detect
 * Title:        run_l1_pipeline.c
 * Description:  Color detect benchmarks.
 *
 * $Date:        24.2.2023
 * ===================================================================== */
/*
 * Copyright (C) 2023 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#include <experiment.h>
#include <cluster_synch.h>
#include <configs.h>

#if defined(_profile_l1_pipeline_)
#include <stimuli.h>
#endif
 
#define RGB2HSV_CV_0_0    0
#define THRESHOLD_CV_0_1  1
#define ERODE_CV_0_2      2
#define DILATE_CV_0_3     3
#define DILATE_CV_0_4     4
#define ERODE_CV_0_5      5

void run_l1_pipeline(const int cluster_id, const int core_id) {

  /* Runtime IDs */

  int run_id; // Pipeline execution run

  /* Define cache stats */

  int hit[2], trns[2], miss[2];
  int reg_hit, reg_trns, reg_miss;

  /* Cycle counters */

  pulp_clk_struct t_experiment_sys_pov;

  /* ===================================================================== */

  /* Initialize L1 memory */

  // L1 variables
  const unsigned l1_n_buffers     = 16;
  const unsigned l1_buffer_dim    = l1_size_B / (4 * l1_n_buffers);

  // Declare L1 image buffers
  DEVICE_PTR l1_img[l1_n_buffers];

  for(int i_buffer=0; i_buffer<l1_n_buffers; i_buffer++){
    l1_img[i_buffer] = (!i_buffer) ? arov_l1_heap(cluster_id) : arov_l1_heap(cluster_id) + i_buffer * l1_buffer_dim;
  }

  /* ===================================================================== */

  /* System */

  pulp_dma_struct dma_in[n_acc_active], dma_out[n_acc_active], dma_wait[n_acc_active];

  uint8_t l1_sel = 0;

  /* Accelerators */

  // Custom registers
  unsigned rows, cols;

  int offload_id[n_acc_active];

  /* Allocate accelerator-rich overlay */

  arov_struct arov;

  /* ===================================================================== */

  /* Cluster steady state condition */
  
  cluster_barrier_eu_soc_evt(cluster_id, 0);
  if(!cluster_id) cluster_slv_restart_eu_soc_evt(cluster_id, 0);

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
    cluster_barrier_eu_soc_evt(cluster_id, 0);

    // Cluster timer
    if(!cluster_id) t_experiment_sys_pov.cnt_0 = hero_get_clk_counter();

    // Activate clusters simultaneously
    // NB: Activation must be after the starting of time measurements
    if(!cluster_id) cluster_slv_restart_eu_soc_evt(cluster_id, 0);

    /* ===================================================================== */

    /*  Profiling - Color detection  */

    // Analyze image by image
    for(int i_img=0; i_img<n_img; i_img++){

      run_id = i_img;

      if(!run_id){

        /* Runtime experiment parameters */

        // - Number of image rows
        rows = 32; 

        // - Number of image columns
        cols = 32;  

        /* Program accelerators */

        for(int acc_id=0; acc_id<n_acc_active; acc_id++){

          // Initialization
          arov_init(&arov, cluster_id, acc_id);
          
          // -- RGB2HSV_CV_0_0 - Map parameters
          if (cluster_id==0 && acc_id==0) 
            arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img[0], l1_img[1], l1_buffer_dim, rows, cols);

          // -- THRESHOLD_CV_0_1 - Map parameters
          if (cluster_id==0 && acc_id==1) 
            arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img[2], l1_img[3], l1_buffer_dim, rows, cols);

          // -- ERODE_CV_0_2 - Map parameters
          if (cluster_id==0 && acc_id==2) 
            arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img[4], l1_img[5], l1_buffer_dim, rows, cols);

          // -- DILATE_CV_0_3 - Map parameters
          if (cluster_id==0 && acc_id==3) 
            arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img[6], l1_img[7], l1_buffer_dim, rows, cols);

          // -- DILATE_CV_0_4 - Map parameters
          if (cluster_id==0 && acc_id==DILATE_CV_0_4) 
            arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img[8], l1_img[9], l1_buffer_dim, rows, cols);

          // -- ERODE_CV_0_5 - Map parameters
          if (cluster_id==0 && acc_id==ERODE_CV_0_5) 
            arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img[10], l1_img[11], l1_buffer_dim, rows, cols);

          // Activation and programming
          offload_id[acc_id] = arov_activate(&arov, cluster_id, acc_id);
          arov_program(&arov, cluster_id, acc_id);

        }
      }

      /* Pipeline */

      arov_compute(&arov, cluster_id, RGB2HSV_CV_0_0);        
      arov_compute(&arov, cluster_id, THRESHOLD_CV_0_1);
      arov_compute(&arov, cluster_id, ERODE_CV_0_2);
      arov_compute(&arov, cluster_id, DILATE_CV_0_3);
      arov_compute(&arov, cluster_id, DILATE_CV_0_4);
      arov_compute(&arov, cluster_id, ERODE_CV_0_5);
      
      while(!arov_is_finished(&arov, cluster_id, 0)){
        arov_wait_eu(&arov, cluster_id, 0);
      }
      while(!arov_is_finished(&arov, cluster_id, 1)){
        arov_wait_eu(&arov, cluster_id, 1);
      }
      while(!arov_is_finished(&arov, cluster_id, 2)){
        arov_wait_eu(&arov, cluster_id, 2);
      }
      while(!arov_is_finished(&arov, cluster_id, 3)){
        arov_wait_eu(&arov, cluster_id, 3);
      }
      while(!arov_is_finished(&arov, cluster_id, 4)){
        arov_wait_eu(&arov, cluster_id, 4);
      }
      while(!arov_is_finished(&arov, cluster_id, 5)){
        arov_wait_eu(&arov, cluster_id, 5);
      }

    } // n_img

    /* ===================================================================== */

    /* MEASUREMENT - END */

    // Cluster synchronization barrier
    cluster_barrier_eu_soc_evt(cluster_id, 0);

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
      "run_l1_pipeline", 
      // Cache
      &reg_hit,
      &reg_trns, 
      &reg_miss, 
      // Clock counters
      0xFFFFFFFF,
      &t_experiment_sys_pov
    );

    /* Restart clusters */

    if(!cluster_id) cluster_slv_restart_eu_soc_evt(cluster_id, 0);

  } // job_id

  /* Cleaning accelerators */

  for(int acc_id=0; acc_id<n_acc_active; acc_id++){
    arov_free(&arov, cluster_id, acc_id);
  }

  /* Cleaning L1 */
  
  #if defined(_pulp_rt_)
    hero_l1free(l1_base_address);
  #endif
}