/* =====================================================================
 * Project:      Color detect
 * Title:        run_l1_baseline.c
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

#if defined(_profile_l2_pipeline_)
#include <stimuli.h>
#endif

void run_l2_pipeline(const int cluster_id, const int core_id) {

  /* Runtime IDs */

  int run_id; // Pipeline execution run

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
  DEVICE_PTR_CONST l2_cl_addr             = l2_cl_base + (cluster_id - (l2_cl_port_id - l2_cl_port_id_offset) * l2_n_cl_per_port) * dma_payload_dim;

  // Declare L2 image buffers
  DEVICE_PTR l2_img[l2_n_buffers];

  l2_img[0] = l2_cl_addr; // input image
  l2_img[1] = l2_img[0] + l2_buffer_dim; // rgb2hsv
  l2_img[2] = l2_img[1] + l2_buffer_dim; // threshold
  l2_img[3] = l2_img[2] + l2_buffer_dim; // erode 1
  l2_img[4] = l2_img[3] + l2_buffer_dim; // dilate 1
  l2_img[5] = l2_img[4] + l2_buffer_dim; // dilate 2
  l2_img[6] = l2_img[5] + l2_buffer_dim; // erode 2 (output image)

  #ifdef INPUT_INIT
    // Initialize input buffer with input image
    for(int i=0; i<l2_buffer_dim; i++) pulp_write32(l2_img_a+i*sizeof(int32_t), in_img_small[i]);
  #endif

  /* ===================================================================== */

  /* Initialize L1 memory */

  #if defined(_pulp_rt_)
    __device uint32_t* l1_arov_buffer   = (__device uint32_t*)hero_l1malloc(sizeof(uint32_t) * l1_arov_buffer_dim);
  #elif defined(_pulp_bare_)
    DEVICE_PTR_CONST l1_arov_buffer    = arov_l1_heap(cluster_id);
  #endif

  // Declare L1 image buffers
  DEVICE_PTR l1_img[l1_n_buffers];

  l1_img[0] = l1_arov_buffer; // input image
  l1_img[1] = l1_img[0] + l1_buffer_dim; // rgb2hsv
  l1_img[2] = l1_img[1] + l1_buffer_dim; // threshold
  l1_img[3] = l1_img[2] + l1_buffer_dim; // erode 1
  l1_img[4] = l1_img[3] + l1_buffer_dim; // dilate 1
  l1_img[5] = l1_img[4] + l1_buffer_dim; // dilate 2
  l1_img[6] = l1_img[5] + l1_buffer_dim; // erode 2 (output image)

  /* ===================================================================== */

  /* System */

  pulp_dma_struct dma_in[n_acc_active], dma_out[n_acc_active], dma_wait[n_acc_active];

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

      // Analyze tile by tile
      for(int i_l2_tile=0; i_l2_tile<l2_n_tiles; i_l2_tile++){

        /* Runtime experiment parameters */

        // - Number of image rows
        rows = l1_img_rows; 

        // - Number of image columns
        cols = l1_img_cols;  

        /* Program accelerators */

        for(int acc_id=0; acc_id<n_acc_active; acc_id++){

          // Initialization
          arov_init(&arov, cluster_id, acc_id);
          
          // -- RGB2HSV_CV - Map parameters
          if (cluster_id==0 && acc_id==0) arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img[0], l1_img[1], l1_buffer_dim, rows, cols);
          // -- THRESHOLD_CV - Map parameters
          if (cluster_id==0 && acc_id==1) arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img[1], l1_img[2], l1_buffer_dim, rows, cols);
          // -- ERODE_CV - Map parameters
          if (cluster_id==0 && acc_id==2) arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img[2], l1_img[3], l1_buffer_dim, rows, cols);
          // -- DILATE_CV - Map parameters
          if (cluster_id==0 && acc_id==3) arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img[3], l1_img[4], l1_buffer_dim, rows, cols);

          // Activation and programming
          offload_id[acc_id] = arov_activate(&arov, cluster_id, acc_id);
          arov_program(&arov, cluster_id, acc_id);

        }

        /* Transfer input image from L2 */

        for (int i = 0; i < dma_n_tx; i++) {
          dma_in[RGB2HSV_CV].job = hero_memcpy_host2dev_async(l1_img[0], l2_img[0], dma_payload_dim * sizeof(uint32_t));
        }

        /* Wait DMA to terminate data transfer */

        hero_dma_wait(dma_in[RGB2HSV_CV].job);

        /* Pipeline */

        // RGB2HSV_CV

        arov_compute(&arov, cluster_id, RGB2HSV_CV);

        while(!arov_is_finished(&arov, cluster_id, RGB2HSV_CV)){
          arov_wait_eu(&arov, cluster_id, RGB2HSV_CV);
        }

        // THRESHOLD_CV
        
        arov_compute(&arov, cluster_id, THRESHOLD_CV);
        
        while(!arov_is_finished(&arov, cluster_id, THRESHOLD_CV)){
          arov_wait_eu(&arov, cluster_id, THRESHOLD_CV);
        }

        // ERODE_CV

        arov_compute(&arov, cluster_id, ERODE_CV);
        
        while(!arov_is_finished(&arov, cluster_id, ERODE_CV)){
          arov_wait_eu(&arov, cluster_id, ERODE_CV);
        }

        // DILATE_CV

        arov_compute(&arov, cluster_id, DILATE_CV);
        
        while(!arov_is_finished(&arov, cluster_id, DILATE_CV)){
          arov_wait_eu(&arov, cluster_id, DILATE_CV);
        }

        // Update buffer pointers for DILATE_CV
        arov.dilate_cv_0_3.img_in.tcdm.ptr = l1_img[4]; 
        arov.dilate_cv_0_3.img_out.tcdm.ptr = l1_img[5]; 
        arov_update_buffer_addr(&arov, cluster_id, DILATE_CV); // Necessary because DILATE_CV is instantiated once, but used more times in the processing pipeline

        // Update buffer pointers for ERODE_CV
        arov.erode_cv_0_2.img_in.tcdm.ptr = l1_img[5]; 
        arov.erode_cv_0_2.img_out.tcdm.ptr = l1_img[6]; 
        arov_update_buffer_addr(&arov, cluster_id, ERODE_CV); // Necessary because DILATE_CV is instantiated once, but used more times in the processing pipeline

        // DILATE_CV

        arov_compute(&arov, cluster_id, DILATE_CV);
        
        while(!arov_is_finished(&arov, cluster_id, DILATE_CV)){
          arov_wait_eu(&arov, cluster_id, DILATE_CV);
        }

        // ERODE_CV

        arov_compute(&arov, cluster_id, ERODE_CV);
        
        while(!arov_is_finished(&arov, cluster_id, ERODE_CV)){
          arov_wait_eu(&arov, cluster_id, ERODE_CV);
        }

        /* Transfer output image to L2 */

        for (int i = 0; i < dma_n_tx; i++) {
          dma_out[ERODE_CV].job = hero_memcpy_dev2host_async(l2_img[6], l1_img[6], dma_payload_dim * sizeof(uint32_t));
        }

        /* Wait DMA to terminate data transfer */

        hero_dma_wait(dma_in[ERODE_CV].job);

      } // l2_n_tiles
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
      "run_l2_pipeline", 
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

  /* Cleaning L1 and L2 */
  
  #if defined(_pulp_rt_)
    hero_l3free(l2_img_a);
    
    // TO-DO: Add all buffers

    hero_l1free(l1_arov_buffer);
  #endif
}