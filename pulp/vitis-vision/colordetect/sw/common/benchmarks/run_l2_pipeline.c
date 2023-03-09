/* =====================================================================
 * Project:      Color detect
 * Title:        run_l2_pipeline.c
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

#include <configs.h>

#ifdef _profile_l2_pipeline_

#include <experiment.h>
#include <cluster_synch.h>

#include <stimuli.h>

void run_l2_pipeline(const int cluster_id, const int core_id) {

  /* Runtime IDs */

  int sel_id; // Buffer selector
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

      if(run_id==0){

        /* Initialize selector */

        sel_id = 0;

        /* Program accelerators */

        for(int acc_id=0; acc_id<n_acc_active; acc_id++){

          // Initialization
          arov_init(&arov, cluster_id, acc_id);
          
          // -- RGB2HSV_CV_0_0 - Map parameters
          if (cluster_id==0 && acc_id==RGB2HSV_CV_0_0) 
            arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img[0], l1_img[2], l1_buffer_dim);

          // -- THRESHOLD_CV_0_1 - Map parameters
          if (cluster_id==0 && acc_id==THRESHOLD_CV_0_1) 
            arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img[3], l1_img[5], l1_buffer_dim);

          // -- ERODE_CV_0_2 - Map parameters
          if (cluster_id==0 && acc_id==ERODE_CV_0_2) 
            arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img[4], l1_img[6], l1_buffer_dim);

          // -- DILATE_CV_0_3 - Map parameters
          if (cluster_id==0 && acc_id==DILATE_CV_0_3) 
            arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img[7], l1_img[9], l1_buffer_dim);

          // -- DILATE_CV_0_4 - Map parameters
          if (cluster_id==0 && acc_id==DILATE_CV_0_4) 
            arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img[8], l1_img[10], l1_buffer_dim);

          // -- ERODE_CV_0_5 - Map parameters
          if (cluster_id==0 && acc_id==ERODE_CV_0_5) 
            arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img[11], l1_img[13], l1_buffer_dim);

          // Activation and programming
          offload_id[acc_id] = arov_activate(&arov, cluster_id, acc_id);
          arov_program(&arov, cluster_id, acc_id);
        
        } // n_acc_active

      } else {

        // Swap buffer

        sel_id = !sel_id;

        // Swap buffer pointers for RGB2HSV_CV
        
        if((run_id>0)&&(run_id%2)){ // odd run_id

          arov.rgb2hsv_cv_0_0.img_in.tcdm.ptr = l1_img[1]; 
          arov.rgb2hsv_cv_0_0.img_out.tcdm.ptr = l1_img[3]; 
          arov_update_buffer_addr(&arov, cluster_id, RGB2HSV_CV_0_0);

          // Swap buffer pointers for THRESHOLD_CV
          arov.threshold_cv_0_1.img_in.tcdm.ptr = l1_img[2]; 
          arov.threshold_cv_0_1.img_out.tcdm.ptr = l1_img[4]; 
          arov_update_buffer_addr(&arov, cluster_id, THRESHOLD_CV_0_1);

          // Swap buffer pointers for ERODE_CV
          arov.erode_cv_0_2.img_in.tcdm.ptr = l1_img[5]; 
          arov.erode_cv_0_2.img_out.tcdm.ptr = l1_img[7]; 
          arov_update_buffer_addr(&arov, cluster_id, ERODE_CV_0_2);

          // Swap buffer pointers for DILATE_CV
          arov.dilate_cv_0_3.img_in.tcdm.ptr = l1_img[6]; 
          arov.dilate_cv_0_3.img_out.tcdm.ptr = l1_img[8]; 
          arov_update_buffer_addr(&arov, cluster_id, DILATE_CV_0_3);
          
          // Swap buffer pointers for DILATE_CV
          arov.dilate_cv_0_4.img_in.tcdm.ptr = l1_img[9]; 
          arov.dilate_cv_0_4.img_out.tcdm.ptr = l1_img[11]; 
          arov_update_buffer_addr(&arov, cluster_id, DILATE_CV_0_4);

          // Swap buffer pointers for DILATE_CV
          arov.erode_cv_0_5.img_in.tcdm.ptr = l1_img[10]; 
          arov.erode_cv_0_5.img_out.tcdm.ptr = l1_img[12]; 
          arov_update_buffer_addr(&arov, cluster_id, ERODE_CV_0_5);
        
        } else { // even run_id

          arov.rgb2hsv_cv_0_0.img_in.tcdm.ptr = l1_img[0]; 
          arov.rgb2hsv_cv_0_0.img_out.tcdm.ptr = l1_img[2]; 
          arov_update_buffer_addr(&arov, cluster_id, RGB2HSV_CV_0_0);

          // Swap buffer pointers for THRESHOLD_CV
          arov.threshold_cv_0_1.img_in.tcdm.ptr = l1_img[3]; 
          arov.threshold_cv_0_1.img_out.tcdm.ptr = l1_img[5]; 
          arov_update_buffer_addr(&arov, cluster_id, THRESHOLD_CV_0_1);

          // Swap buffer pointers for ERODE_CV
          arov.erode_cv_0_2.img_in.tcdm.ptr = l1_img[4]; 
          arov.erode_cv_0_2.img_out.tcdm.ptr = l1_img[6]; 
          arov_update_buffer_addr(&arov, cluster_id, ERODE_CV_0_2);

          // Swap buffer pointers for DILATE_CV
          arov.dilate_cv_0_3.img_in.tcdm.ptr = l1_img[7]; 
          arov.dilate_cv_0_3.img_out.tcdm.ptr = l1_img[9]; 
          arov_update_buffer_addr(&arov, cluster_id, DILATE_CV_0_3);
          
          // Swap buffer pointers for DILATE_CV
          arov.dilate_cv_0_4.img_in.tcdm.ptr = l1_img[8]; 
          arov.dilate_cv_0_4.img_out.tcdm.ptr = l1_img[10]; 
          arov_update_buffer_addr(&arov, cluster_id, DILATE_CV_0_4);

          // Swap buffer pointers for DILATE_CV
          arov.erode_cv_0_5.img_in.tcdm.ptr = l1_img[11]; 
          arov.erode_cv_0_5.img_out.tcdm.ptr = l1_img[13]; 
          arov_update_buffer_addr(&arov, cluster_id, ERODE_CV_0_5);

        }
      }

      /* Transfer input tile from L2 */
      
      for (int i = 0; i < dma_n_tx; i++) {
        // Launch transactions
        dma_in[sel_id].job = hero_memcpy_host2dev_async(l1_img[sel_id], l2_img[sel_id], dma_payload_dim * sizeof(uint32_t));

        // Wait DMAs from previous round
        if(run_id>0) hero_dma_wait(dma_in[!sel_id].job);
        // if(run_id>0) hero_dma_wait(dma_out[!sel_id].job); // use when we'll have bi-directional DMA
      }

      /* Wait for DMA, then run pipeline stages */

      arov_compute(&arov, cluster_id, RGB2HSV_CV_0_0); 
      arov_compute(&arov, cluster_id, THRESHOLD_CV_0_1);
      arov_compute(&arov, cluster_id, ERODE_CV_0_2);
      arov_compute(&arov, cluster_id, DILATE_CV_0_3);
      arov_compute(&arov, cluster_id, DILATE_CV_0_4);
      arov_compute(&arov, cluster_id, ERODE_CV_0_5);

      /* Wait for final stage, then transfer output tile to L2 */

      while(!arov_is_finished(&arov, cluster_id, ERODE_CV_0_5)){
        arov_wait_eu(&arov, cluster_id, ERODE_CV_0_5);
      }

      for (int i = 0; i < dma_n_tx; i++) {
        // Launch transactions (only programming because DMA is currently not bi-directional)
        if((run_id>0)&&(run_id%2))
          dma_out[sel_id].job = hero_memcpy_dev2host_async_no_trigger(l2_img[12], l1_img[12], dma_payload_dim * sizeof(uint32_t));
        else
          dma_out[sel_id].job = hero_memcpy_dev2host_async_no_trigger(l2_img[13], l1_img[13], dma_payload_dim * sizeof(uint32_t));
      }

      /* Wait for pipeline stages */
      
      while(!arov_is_finished(&arov, cluster_id, RGB2HSV_CV_0_0)){
        arov_wait_eu(&arov, cluster_id, RGB2HSV_CV_0_0);
      }
      while(!arov_is_finished(&arov, cluster_id, THRESHOLD_CV_0_1)){
        arov_wait_eu(&arov, cluster_id, THRESHOLD_CV_0_1);
      }
      while(!arov_is_finished(&arov, cluster_id, ERODE_CV_0_2)){
        arov_wait_eu(&arov, cluster_id, ERODE_CV_0_2);
      }
      while(!arov_is_finished(&arov, cluster_id, DILATE_CV_0_3)){
        arov_wait_eu(&arov, cluster_id, DILATE_CV_0_3);
      }
      while(!arov_is_finished(&arov, cluster_id, DILATE_CV_0_4)){
        arov_wait_eu(&arov, cluster_id, DILATE_CV_0_4);
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

  /* Cleaning L1 */
  
  #if defined(_pulp_rt_)
    hero_l1free(l1_base_address);
  #endif
}

#endif