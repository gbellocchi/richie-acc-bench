/* =====================================================================
 * Project:      Color detect
 * Title:        run_l2_pipeline_const_buffer.c
 * Description:  Implementation of an accelerator-rich vision pipeline 
 *               in a PULP cluster. Each accelerator is given a constant
 *               buffer dimension, even when spreading accelerators in 
 *               different clusters. This means the L1 dimension is 
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

#if ((n_clusters) == 1) && defined(_profile_l2_pipeline_) && defined(_implement_const_single_buffer_) 

#include <experiment.h>
#include <cluster_synch.h>

#include <stimuli.h>

void run_l2_pipeline(const int cluster_id, const int core_id) {

  /* Runtime IDs */

  int buffer_id; // Buffer selector
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
  
  cluster_barrier_all_eu_soc_evt(cluster_id, 0);
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
    cluster_barrier_all_eu_soc_evt(cluster_id, 0);

    // Cluster timer
    if(!cluster_id) t_experiment_sys_pov.cnt_0 = hero_get_clk_counter();

    // Activate clusters simultaneously
    // NB: Activation must be after the starting of time measurements
    if(!cluster_id) cluster_slv_all_restart_eu_soc_evt(cluster_id, 0);

    /* ===================================================================== */

    /*  Profiling - Color detection  */

    // Analyze image by image
    for(int i_img=0; i_img<n_img; i_img++){

      // Analyze tile by tile
      for(int i_tile=0; i_tile<l2_n_tiles; i_tile++){

        run_id = i_img * l2_n_tiles + i_tile;

        if(run_id==0){

          /* Initialize selector */

          buffer_id = 0;

          /* Program accelerators */

          // -- RGB2HSV_CV_0 
          if(cluster_id == get_cid(RGB2HSV_CV_0)){
            arov_init(&arov, get_cid(RGB2HSV_CV_0), get_aid(RGB2HSV_CV_0));
            arov_map_params_color_detect(&arov, get_cid(RGB2HSV_CV_0), get_aid(RGB2HSV_CV_0), l1_img[0], l1_img[2], l1_buffer_dim);
            offload_id[get_aid(RGB2HSV_CV_0)] = arov_activate(&arov, get_cid(RGB2HSV_CV_0), get_aid(RGB2HSV_CV_0));
            arov_program(&arov, get_cid(RGB2HSV_CV_0), get_aid(RGB2HSV_CV_0));
          }

          // -- THRESHOLD_CV_1 
          if(cluster_id == get_cid(THRESHOLD_CV_1)){
            arov_init(&arov, get_cid(THRESHOLD_CV_1), get_aid(THRESHOLD_CV_1));
            arov_map_params_color_detect(&arov, get_cid(THRESHOLD_CV_1), get_aid(THRESHOLD_CV_1), l1_img[0], l1_img[2], l1_buffer_dim);
            offload_id[get_aid(THRESHOLD_CV_1)] = arov_activate(&arov, get_cid(THRESHOLD_CV_1), get_aid(THRESHOLD_CV_1));
            arov_program(&arov, get_cid(THRESHOLD_CV_1), get_aid(THRESHOLD_CV_1));
          }
            
          // -- ERODE_CV_2
          if(cluster_id == get_cid(ERODE_CV_2)){
            arov_init(&arov, get_cid(ERODE_CV_2),   get_aid(ERODE_CV_2));
            arov_map_params_color_detect(&arov, get_cid(ERODE_CV_2), get_aid(ERODE_CV_2), l1_img[0], l1_img[2], l1_buffer_dim);
            offload_id[get_aid(ERODE_CV_2)] = arov_activate(&arov, get_cid(ERODE_CV_2), get_aid(ERODE_CV_2));
            arov_program(&arov, get_cid(ERODE_CV_2), get_aid(ERODE_CV_2));
          }

          // -- DILATE_CV_3
          if(cluster_id == get_cid(DILATE_CV_3)){
            arov_init(&arov, get_cid(DILATE_CV_3),  get_aid(DILATE_CV_3));
            arov_map_params_color_detect(&arov, get_cid(DILATE_CV_3), get_aid(DILATE_CV_3), l1_img[0], l1_img[2], l1_buffer_dim);
            offload_id[get_aid(DILATE_CV_3)] = arov_activate(&arov, get_cid(DILATE_CV_3), get_aid(DILATE_CV_3));
            arov_program(&arov, get_cid(DILATE_CV_3), get_aid(DILATE_CV_3));
          }

          // -- DILATE_CV_4
          if(cluster_id == get_cid(DILATE_CV_4)){
            arov_init(&arov, get_cid(DILATE_CV_4),  get_aid(DILATE_CV_4));
            arov_map_params_color_detect(&arov, get_cid(DILATE_CV_4), get_aid(DILATE_CV_4), l1_img[0], l1_img[2], l1_buffer_dim);
            offload_id[get_aid(DILATE_CV_4)] = arov_activate(&arov, get_cid(DILATE_CV_4), get_aid(DILATE_CV_4));
            arov_program(&arov, get_cid(DILATE_CV_4), get_aid(DILATE_CV_4));
          }

          // -- ERODE_CV_5
          if(cluster_id == get_cid(ERODE_CV_5)){
            arov_init(&arov, get_cid(ERODE_CV_5),   get_aid(ERODE_CV_5));
            arov_map_params_color_detect(&arov, get_cid(ERODE_CV_5), get_aid(ERODE_CV_5), l1_img[0], l1_img[2], l1_buffer_dim);  
            offload_id[get_aid(ERODE_CV_5)] = arov_activate(&arov, get_cid(ERODE_CV_5), get_aid(ERODE_CV_5));
            arov_program(&arov, get_cid(ERODE_CV_5), get_aid(ERODE_CV_5));
          }

        } else {

          // Swap buffer ID
          buffer_id = !buffer_id;

          // Swap buffer pointers
          if(run_id>0){
            arov_swap_buffers_color_detect(&arov, cluster_id, buffer_id, l1_img);
          }

        }

        /* Transfer input tile from L2 */

        // During first round, you cannot hide DMA programming
        if(run_id==0) {
          for (int i = 0; i < dma_n_tx; i++) {
            // Launch transactions
            dma_in[buffer_id].job = hero_memcpy_host2dev_async(l1_img[buffer_id], l2_img[buffer_id], dma_payload_dim * sizeof(uint32_t));
          }
        }

        /* Wait for DMA */

        if(run_id>0) 
          // Wait DMAs issued in previous round
          hero_dma_wait(dma_in[buffer_id].job);
        else
          // During first round, you cannot hide DMA transfer
          hero_dma_wait(dma_in[buffer_id].job);

        // Wait for output buffer from previous round to be freed
        // if(run_id>0) hero_dma_wait(dma_out[!buffer_id].job); // use when we'll have bi-directional DMA

        /* Run pipeline stages */

        if(cluster_id == get_cid(RGB2HSV_CV_0)) 
          arov_compute(&arov, get_cid(RGB2HSV_CV_0), get_aid(RGB2HSV_CV_0)); 

        if(cluster_id == get_cid(THRESHOLD_CV_1)) 
          arov_compute(&arov, get_cid(THRESHOLD_CV_1), get_aid(THRESHOLD_CV_1));

        if(cluster_id == get_cid(ERODE_CV_2)) 
          arov_compute(&arov, get_cid(ERODE_CV_2),   get_aid(ERODE_CV_2));

        if(cluster_id == get_cid(DILATE_CV_3)) 
          arov_compute(&arov, get_cid(DILATE_CV_3),  get_aid(DILATE_CV_3));

        if(cluster_id == get_cid(DILATE_CV_4)) 
          arov_compute(&arov, get_cid(DILATE_CV_4),  get_aid(DILATE_CV_4));

        if(cluster_id == get_cid(ERODE_CV_5)) 
          arov_compute(&arov, get_cid(ERODE_CV_5),   get_aid(ERODE_CV_5));

        /* Program input DMA for next round */

        // Programming of successive round is to be hidden during computation

        if(!(run_id == (n_img * l2_n_tiles - 1))) {
          for (int i = 0; i < dma_n_tx; i++) {
            // Launch transactions
            dma_in[!buffer_id].job = hero_memcpy_host2dev_async(l1_img[!buffer_id], l2_img[!buffer_id], dma_payload_dim * sizeof(uint32_t));
          }
        }

        /* Wait for final stage, then transfer output tile to L2 */

        while(!arov_is_finished(&arov, cluster_id, get_aid(ERODE_CV_5))){
          arov_wait_eu(&arov, cluster_id, get_aid(ERODE_CV_5));
        }

        /* Transfer output tile to L2 */

        for (int i = 0; i < dma_n_tx; i++) {
          // Launch transactions (only programming because DMA is currently not bi-directional)
          if((run_id>0)&&(run_id%2))
            dma_out[buffer_id].job = hero_memcpy_dev2host_async_no_trigger(l2_img[3], l1_img[3], dma_payload_dim * sizeof(uint32_t));
          else
            dma_out[buffer_id].job = hero_memcpy_dev2host_async_no_trigger(l2_img[2], l1_img[2], dma_payload_dim * sizeof(uint32_t));
        }

        /* Wait for pipeline stages */
        
        while(!arov_is_finished(&arov, cluster_id, get_aid(RGB2HSV_CV_0))){
          arov_wait_eu(&arov, cluster_id, get_aid(RGB2HSV_CV_0));
        }
        while(!arov_is_finished(&arov, cluster_id, get_aid(THRESHOLD_CV_1))){
          arov_wait_eu(&arov, cluster_id, get_aid(THRESHOLD_CV_1));
        }
        while(!arov_is_finished(&arov, cluster_id, get_aid(ERODE_CV_2))){
          arov_wait_eu(&arov, cluster_id, get_aid(ERODE_CV_2));
        }
        while(!arov_is_finished(&arov, cluster_id, get_aid(DILATE_CV_3))){
          arov_wait_eu(&arov, cluster_id, get_aid(DILATE_CV_3));
        }
        while(!arov_is_finished(&arov, cluster_id, get_aid(DILATE_CV_4))){
          arov_wait_eu(&arov, cluster_id, get_aid(DILATE_CV_4));
        }
      } // n_tiles  
    } // n_img

    /* ===================================================================== */

    /* MEASUREMENT - END */

    // Cluster synchronization barrier
    cluster_barrier_all_eu_soc_evt(cluster_id, 0);

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

    if(!cluster_id) cluster_slv_all_restart_eu_soc_evt(cluster_id, 0);

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

// #if ((n_clusters) > 1) && defined(_profile_l2_pipeline_) && defined(_implement_const_single_buffer_) 

#include <experiment.h>
#include <cluster_synch.h>

#include <stimuli.h>

void run_l2_pipeline(const int cluster_id, const int core_id) {

  /* Runtime IDs */

  int buffer_id; // Buffer selector
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
  
  cluster_barrier_all_eu_soc_evt(cluster_id, 0);
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
    if((!cluster_id) || (cluster_id >= (n_clusters/2))) cluster_barrier_all_eu_soc_evt(cluster_id, 0); /* -- CMD_TYPE: STAGE INVOCATION -- */ 

    // Cluster timer
    if(!cluster_id) t_experiment_sys_pov.cnt_0 = hero_get_clk_counter();

    /* ===================================================================== */

    /*  Profiling - Color detection  */

    // Analyze image by image
    for(int i_img=0; i_img<n_img; i_img++){

      // Analyze tile by tile
      for(int i_tile=0; i_tile<l2_n_tiles; i_tile++){

        run_id = i_img * l2_n_tiles + i_tile;

                              /* DMA out clusters */

        if(cluster_id >= (n_clusters/2)){

            /* Transfer output tile to L2 */

            // This implementation is like a tightly-coupled bounding between the output DMA 
            // and a proxy core inside an accelerator-rich cluster because the processor of the
            // additional cluster waits for the DMA, then notifies the former about transfer completion.

            for (int i = 0; i < dma_n_tx; i++) {
              // Launch transactions (only programming because DMA is currently not bi-directional)
              if((run_id > 0) && (run_id % 2))
                hero_memcpy_dev2host(l2_img[3], l1_img[3], dma_payload_dim * sizeof(uint32_t));
              else
                hero_memcpy_dev2host(l2_img[2], l1_img[2], dma_payload_dim * sizeof(uint32_t));
            }

            /* Send feedback about end of DMA transaction to MASTER CLUSTER */

            send_cmd_eu_sw_evt(cluster_id, cluster_id - (n_clusters/2), CMD_TYPE_DMA_OUT_TERMINATE); /* -- CMD_TYPE: DMA OUT TERMINATE -- */

            /* Go back to sleep until next invocation */

            if(!(run_id == (n_img * l2_n_tiles - 1)))
              wait_cmd_eu_sw_evt(cluster_id, cluster_id - (n_clusters/2), CMD_TYPE_DMA_OUT_START); /* -- CMD_TYPE: DMA OUT START -- */

        } else {

                              /* Accelerator-rich cluster routine */

          if(!run_id){

            /* Initialize selector */
            
            buffer_id = 0;

            /* Program accelerators */

            // -- RGB2HSV_CV_0 
            if(cluster_id == get_cid(RGB2HSV_CV_0)){
              arov_init(&arov, get_cid(RGB2HSV_CV_0), get_aid(RGB2HSV_CV_0));
              arov_map_params_color_detect(&arov, get_cid(RGB2HSV_CV_0), get_aid(RGB2HSV_CV_0), l1_img[0], l1_img[2], l1_buffer_dim);
              offload_id[get_aid(RGB2HSV_CV_0)] = arov_activate(&arov, get_cid(RGB2HSV_CV_0), get_aid(RGB2HSV_CV_0));
              arov_program(&arov, get_cid(RGB2HSV_CV_0), get_aid(RGB2HSV_CV_0));
            }

            // -- THRESHOLD_CV_1 
            if(cluster_id == get_cid(THRESHOLD_CV_1)){
              arov_init(&arov, get_cid(THRESHOLD_CV_1), get_aid(THRESHOLD_CV_1));
              arov_map_params_color_detect(&arov, get_cid(THRESHOLD_CV_1), get_aid(THRESHOLD_CV_1), l1_img[0], l1_img[2], l1_buffer_dim);
              offload_id[get_aid(THRESHOLD_CV_1)] = arov_activate(&arov, get_cid(THRESHOLD_CV_1), get_aid(THRESHOLD_CV_1));
              arov_program(&arov, get_cid(THRESHOLD_CV_1), get_aid(THRESHOLD_CV_1));
            }
              
            // -- ERODE_CV_2
            if(cluster_id == get_cid(ERODE_CV_2)){
              arov_init(&arov, get_cid(ERODE_CV_2),   get_aid(ERODE_CV_2));
              arov_map_params_color_detect(&arov, get_cid(ERODE_CV_2), get_aid(ERODE_CV_2), l1_img[0], l1_img[2], l1_buffer_dim);
              offload_id[get_aid(ERODE_CV_2)] = arov_activate(&arov, get_cid(ERODE_CV_2), get_aid(ERODE_CV_2));
              arov_program(&arov, get_cid(ERODE_CV_2), get_aid(ERODE_CV_2));
            }

            // -- DILATE_CV_3
            if(cluster_id == get_cid(DILATE_CV_3)){
              arov_init(&arov, get_cid(DILATE_CV_3),  get_aid(DILATE_CV_3));
              arov_map_params_color_detect(&arov, get_cid(DILATE_CV_3), get_aid(DILATE_CV_3), l1_img[0], l1_img[2], l1_buffer_dim);
              offload_id[get_aid(DILATE_CV_3)] = arov_activate(&arov, get_cid(DILATE_CV_3), get_aid(DILATE_CV_3));
              arov_program(&arov, get_cid(DILATE_CV_3), get_aid(DILATE_CV_3));
            }

            // -- DILATE_CV_4
            if(cluster_id == get_cid(DILATE_CV_4)){
              arov_init(&arov, get_cid(DILATE_CV_4),  get_aid(DILATE_CV_4));
              arov_map_params_color_detect(&arov, get_cid(DILATE_CV_4), get_aid(DILATE_CV_4), l1_img[0], l1_img[2], l1_buffer_dim);
              offload_id[get_aid(DILATE_CV_4)] = arov_activate(&arov, get_cid(DILATE_CV_4), get_aid(DILATE_CV_4));
              arov_program(&arov, get_cid(DILATE_CV_4), get_aid(DILATE_CV_4));
            }

            // -- ERODE_CV_5
            if(cluster_id == get_cid(ERODE_CV_5)){
              arov_init(&arov, get_cid(ERODE_CV_5),   get_aid(ERODE_CV_5));
              arov_map_params_color_detect(&arov, get_cid(ERODE_CV_5), get_aid(ERODE_CV_5), l1_img[0], l1_img[2], l1_buffer_dim);  
              offload_id[get_aid(ERODE_CV_5)] = arov_activate(&arov, get_cid(ERODE_CV_5), get_aid(ERODE_CV_5));
              arov_program(&arov, get_cid(ERODE_CV_5), get_aid(ERODE_CV_5));
            }

          } else {

            // Swap buffer ID
            buffer_id = !buffer_id;

            // Swap buffer pointers
            if(run_id>0) arov_swap_buffers_color_detect(&arov, cluster_id, buffer_id, l1_img);

          }

          /* Wait for invocation from previous pipeline stage */

          // Cluster synchronization barrier
          // This way pipeline stages will anticipate the initial accelerator programming phase 
          // and will be ready for the input DMA transfer right after their invocation
          
          if(cluster_id > 0){
            if(!run_id)
              cluster_barrier_all_eu_soc_evt(cluster_id, 0); /* -- CMD_TYPE: STAGE INVOCATION -- */ 
            else
              wait_cmd_eu_sw_evt(cluster_id, cluster_id - 1, CMD_TYPE_STAGE_INVOCATION);  /* -- CMD_TYPE: STAGE INVOCATION -- */ 
          }

          /* Handle first input tile transfer from L2 */

          // During first round, you cannot hide DMA programming

          if((!run_id) || (cluster_id > 0)) {
            for (int i = 0; i < dma_n_tx; i++) {
              dma_in[buffer_id].job = hero_memcpy_host2dev_async(l1_img[buffer_id], l2_img[buffer_id], dma_payload_dim * sizeof(uint32_t));
            }
          }

          /* Wait for output DMA to terminate data transfer */

          if(run_id > 0) 
            wait_cmd_eu_sw_evt(cluster_id, cluster_id + (n_clusters/2), CMD_TYPE_DMA_OUT_TERMINATE);  /* -- CMD_TYPE: DMA OUT TERMINATE -- */  

          /* Wake up next pipeline stage */

          if((run_id > 0) && !(cluster_id == (n_clusters/2 - 1)))
            send_cmd_eu_sw_evt(cluster_id, cluster_id + 1, CMD_TYPE_STAGE_INVOCATION);  /* -- CMD_TYPE: STAGE INVOCATION -- */ 

          /* Wait for input DMA */

          // Either issued in previous round or during the first one

          hero_dma_wait(dma_in[buffer_id].job);
          
          /* Notify the stage[i-1] about the successful input DMA transfer, so as to free a buffer from already sunk data */

          if((run_id > ((l2_n_buffers / 2) - 1)) && (cluster_id > 0))
            send_cmd_eu_sw_evt(cluster_id, cluster_id - 1, CMD_TYPE_DMA_IN_TERMINATE);  /* -- CMD_TYPE: DMA IN TERMINATE -- */ 

          /* Run pipeline stages */

          if(cluster_id == get_cid(RGB2HSV_CV_0)) 
            arov_compute(&arov, get_cid(RGB2HSV_CV_0), get_aid(RGB2HSV_CV_0)); 

          if(cluster_id == get_cid(THRESHOLD_CV_1)) 
            arov_compute(&arov, get_cid(THRESHOLD_CV_1), get_aid(THRESHOLD_CV_1));

          if(cluster_id == get_cid(ERODE_CV_2)) 
            arov_compute(&arov, get_cid(ERODE_CV_2),   get_aid(ERODE_CV_2));

          if(cluster_id == get_cid(DILATE_CV_3)) 
            arov_compute(&arov, get_cid(DILATE_CV_3),  get_aid(DILATE_CV_3));

          if(cluster_id == get_cid(DILATE_CV_4)) 
            arov_compute(&arov, get_cid(DILATE_CV_4),  get_aid(DILATE_CV_4));

          if(cluster_id == get_cid(ERODE_CV_5)) 
            arov_compute(&arov, get_cid(ERODE_CV_5),   get_aid(ERODE_CV_5));

          /* Program input DMA for next round */

          // Programming of successive round is to be hidden during computation

          if(!cluster_id){
            if(!(run_id == (n_img * l2_n_tiles - 1))) {
              for (int i = 0; i < dma_n_tx; i++) {
                // Launch transactions
                dma_in[!buffer_id].job = hero_memcpy_host2dev_async(l1_img[!buffer_id], l2_img[!buffer_id], dma_payload_dim * sizeof(uint32_t));
              }
            }
          }

          /* Wait for final stage, then transfer output tile to L2 */

          switch (cluster_id){
            case get_cid(RGB2HSV_CV_0): while(!arov_is_finished(&arov, cluster_id, get_aid(RGB2HSV_CV_0))){arov_wait_eu(&arov, cluster_id, get_aid(RGB2HSV_CV_0));} break;
            case get_cid(THRESHOLD_CV_1): while(!arov_is_finished(&arov, cluster_id, get_aid(THRESHOLD_CV_1))){arov_wait_eu(&arov, cluster_id, get_aid(THRESHOLD_CV_1));} break;
            case get_cid(ERODE_CV_2): while(!arov_is_finished(&arov, cluster_id, get_aid(ERODE_CV_2))){arov_wait_eu(&arov, cluster_id, get_aid(ERODE_CV_2));} break;
            case get_cid(DILATE_CV_3): while(!arov_is_finished(&arov, cluster_id, get_aid(DILATE_CV_3))){arov_wait_eu(&arov, cluster_id, get_aid(DILATE_CV_3));} break;
            case get_cid(DILATE_CV_4): while(!arov_is_finished(&arov, cluster_id, get_aid(DILATE_CV_4))){arov_wait_eu(&arov, cluster_id, get_aid(DILATE_CV_4));} break;
            case get_cid(ERODE_CV_5): while(!arov_is_finished(&arov, cluster_id, get_aid(ERODE_CV_5))){arov_wait_eu(&arov, cluster_id, get_aid(ERODE_CV_5));} break;
          }
          
          /* Transfer output tile to L2 */

          // If a processing stage[i] has not read data been processed by stage[i-1] and L2 buffers are full, 
          // then stage[i-1] must wait for feedback from the former before issuing new output transfers. 
          // This way, not yet processed data won't be overwritten.

          if((run_id > ((l2_n_buffers / 2) - 1)) && !(cluster_id == (n_clusters/2 - 1)))
            wait_cmd_eu_sw_evt(cluster_id, cluster_id + 1, CMD_TYPE_DMA_IN_TERMINATE);

          // Wake up a cluster that is only used to mimic the bi-directional DMA for ouputs
          send_cmd_eu_sw_evt(cluster_id, cluster_id + (n_clusters/2), CMD_TYPE_DMA_OUT_START); /* -- CMD_TYPE: DMA OUT START -- */
          
        }
      } // n_tiles  
    } // n_img

    /* ===================================================================== */

    /* MEASUREMENT - END */

    // Cluster synchronization barrier
    cluster_barrier_all_eu_soc_evt(cluster_id, 0); /* -- CMD_TYPE: STAGE INVOCATION -- */ 

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

    if(!cluster_id) cluster_slv_all_restart_eu_soc_evt(cluster_id, 0);

    /* ===================================================================== */

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

// #endif