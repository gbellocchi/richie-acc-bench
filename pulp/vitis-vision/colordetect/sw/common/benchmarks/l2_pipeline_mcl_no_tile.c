/* =====================================================================
 * Project:      Color detect
 * Title:        l2_pipeline_mcl_no_tile.c
 * Description:  Implementation of an accelerator-rich vision pipeline 
 *               in a multi-cluster PULP architecture. Each accelerator 
 *               stage needs to wait for the previous to process an 
 *               entire image before to start.
 *
 * $Date:        21.3.2023
 * ===================================================================== */
/*
 * Copyright (C) 2023 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#include <configs.h>
#include <list_benchmarks.h>

#if (BENCHMARK_TYPE == L2_PIPELINE_MCL_NO_TILE)

#include <experiment.h>
#include <cluster_synch.h>

#include <stimuli.h>

// #define PRINT_PIPELINE_LOG
// #define DEBUG_DISABLE_PIPELINE_OPS

void run_benchmark(const int cluster_id, const int core_id) {

  /* Runtime IDs */

  int buffer_id; // Buffer selector
  int run_id; // Execution run

  /* Runtime indicators */
  
  // Processing status concerning current cluster
  int n_img_out_total;
  int n_img_in[n_acc_stages_cl], n_img_out[n_acc_stages_cl];
  int n_tiles_in[n_acc_stages_cl], n_tiles_in_on_flight[n_acc_stages_cl];
  int n_tiles_compute[n_acc_stages_cl], n_tiles_compute_on_flight[n_acc_stages_cl];
  int n_tiles_out[n_acc_stages_cl], n_tiles_out_on_flight[n_acc_stages_cl], n_tiles_out_not_checked[n_acc_stages_cl];

  // Number of images been processed from previous cluster and ready for use by current one
  int n_img_prev_cl_ready;

  // Number of images been used from next cluster, hence freeing the correspondant L2 buffer
  int n_img_next_cl_used, n_img_next_cl_used_on_flight;

  /* SoC events */

  uint32_t soc_evt_msg, soc_evt_mst, soc_evt_acc, soc_evt_cmd;

  // Asynchronous terminations from slave clusters
  int n_slv_cl_terminate[n_clusters];

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
    for(int i_buffer=0; i_buffer<l2_buffer_dim; i_buffer++) pulp_write32(l2_img_a+i_buffer*sizeof(int32_t), in_img_small[i_buffer]);
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

  pulp_dma_struct dma_in[n_acc_stages_cl * 2], dma_out[n_acc_stages_cl * 2], dma_wait[n_acc_stages_cl * 2];

  /* Accelerators */

  // Custom registers
  unsigned rows, cols;

  int offload_id[n_acc_stages_cl];

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
    cluster_barrier_all_eu_soc_evt(cluster_id, 0, 0xFFFFFFFF);

    // Cluster timer
    if(!cluster_id) t_experiment_sys_pov.cnt_0 = hero_get_clk_counter();

    // Activate clusters simultaneously
    // NB: Activation must be after the starting of time measurements
    if(!cluster_id) cluster_slv_all_restart_eu_soc_evt(cluster_id, 0);

    /* ===================================================================== */

    /*  Profiling - Color detection  */
    
    run_id = 0;

    n_img_prev_cl_ready = 0;
    n_img_next_cl_used = 0;

    n_img_next_cl_used_on_flight = 0;

    for(int acc_id=0; acc_id<n_acc_stages_cl; acc_id++){
      n_img_in[acc_id] = 0;
      n_img_out[acc_id] = 0;
      n_tiles_in[acc_id] = 0;
      n_tiles_in_on_flight[acc_id] = 0;
      n_tiles_compute[acc_id] = 0;
      n_tiles_compute_on_flight[acc_id] = 0;
      n_tiles_out[acc_id] = 0;
      n_tiles_out_on_flight[acc_id] = 0;
      n_tiles_out_not_checked[acc_id] = 0;
    }

    for(int i=0; i < n_clusters; i++){
      n_slv_cl_terminate[i] = 0;
    }

    n_img_out_total = 0;

    while(n_img_out_total < n_img){

                            /* Accelerator-rich cluster routine */

      if(cluster_id < (n_clusters/2)){

        if(run_id==0){

          /* Initialize selector */

          buffer_id = 0;

          /* Program accelerators */
          
          // -- RGB2HSV_CV_0 
          if(cluster_id == get_acc_cid(RGB2HSV_CV_0)){
            arov_init(&arov, get_acc_cid(RGB2HSV_CV_0), get_acc_aid(RGB2HSV_CV_0));
            arov_map_params_color_detect(&arov, get_acc_cid(RGB2HSV_CV_0), get_acc_aid(RGB2HSV_CV_0), l1_img[0], l1_img[2], l1_buffer_dim);
            offload_id[get_acc_aid(RGB2HSV_CV_0)] = arov_activate(&arov, get_acc_cid(RGB2HSV_CV_0), get_acc_aid(RGB2HSV_CV_0));
            arov_program(&arov, get_acc_cid(RGB2HSV_CV_0), get_acc_aid(RGB2HSV_CV_0));
          }

          // -- THRESHOLD_CV_1 
          if(cluster_id == get_acc_cid(THRESHOLD_CV_1)){
            arov_init(&arov, get_acc_cid(THRESHOLD_CV_1), get_acc_aid(THRESHOLD_CV_1));
            arov_map_params_color_detect(&arov, get_acc_cid(THRESHOLD_CV_1), get_acc_aid(THRESHOLD_CV_1), l1_img[0], l1_img[2], l1_buffer_dim);
            offload_id[get_acc_aid(THRESHOLD_CV_1)] = arov_activate(&arov, get_acc_cid(THRESHOLD_CV_1), get_acc_aid(THRESHOLD_CV_1));
            arov_program(&arov, get_acc_cid(THRESHOLD_CV_1), get_acc_aid(THRESHOLD_CV_1));
          }
            
          // -- ERODE_CV_2
          if(cluster_id == get_acc_cid(ERODE_CV_2)){
            arov_init(&arov, get_acc_cid(ERODE_CV_2),   get_acc_aid(ERODE_CV_2));
            arov_map_params_color_detect(&arov, get_acc_cid(ERODE_CV_2), get_acc_aid(ERODE_CV_2), l1_img[0], l1_img[2], l1_buffer_dim);
            offload_id[get_acc_aid(ERODE_CV_2)] = arov_activate(&arov, get_acc_cid(ERODE_CV_2), get_acc_aid(ERODE_CV_2));
            arov_program(&arov, get_acc_cid(ERODE_CV_2), get_acc_aid(ERODE_CV_2));
          }

          // -- DILATE_CV_3
          if(cluster_id == get_acc_cid(DILATE_CV_3)){
            arov_init(&arov, get_acc_cid(DILATE_CV_3),  get_acc_aid(DILATE_CV_3));
            arov_map_params_color_detect(&arov, get_acc_cid(DILATE_CV_3), get_acc_aid(DILATE_CV_3), l1_img[0], l1_img[2], l1_buffer_dim);
            offload_id[get_acc_aid(DILATE_CV_3)] = arov_activate(&arov, get_acc_cid(DILATE_CV_3), get_acc_aid(DILATE_CV_3));
            arov_program(&arov, get_acc_cid(DILATE_CV_3), get_acc_aid(DILATE_CV_3));
          }

          // -- DILATE_CV_4
          if(cluster_id == get_acc_cid(DILATE_CV_4)){
            arov_init(&arov, get_acc_cid(DILATE_CV_4),  get_acc_aid(DILATE_CV_4));
            arov_map_params_color_detect(&arov, get_acc_cid(DILATE_CV_4), get_acc_aid(DILATE_CV_4), l1_img[0], l1_img[2], l1_buffer_dim);
            offload_id[get_acc_aid(DILATE_CV_4)] = arov_activate(&arov, get_acc_cid(DILATE_CV_4), get_acc_aid(DILATE_CV_4));
            arov_program(&arov, get_acc_cid(DILATE_CV_4), get_acc_aid(DILATE_CV_4));
          }

          // -- ERODE_CV_5
          if(cluster_id == get_acc_cid(ERODE_CV_5)){
            arov_init(&arov, get_acc_cid(ERODE_CV_5),   get_acc_aid(ERODE_CV_5));
            arov_map_params_color_detect(&arov, get_acc_cid(ERODE_CV_5), get_acc_aid(ERODE_CV_5), l1_img[0], l1_img[2], l1_buffer_dim);  
            offload_id[get_acc_aid(ERODE_CV_5)] = arov_activate(&arov, get_acc_cid(ERODE_CV_5), get_acc_aid(ERODE_CV_5));
            arov_program(&arov, get_acc_cid(ERODE_CV_5), get_acc_aid(ERODE_CV_5));
          }

        } else {

          /* Swap buffer ID */
          
          buffer_id = !buffer_id;

        }

        /* ================================================ */
        /* Wait for invocation from previous pipeline stage */
        /* ================================================ */

        // All clusters but first have to wait for an invocation from their predecessor
        if(cluster_id > 0){
        
          // If this cluster has processed all the images the previous one had pre-processed, then wait for others to be ready
          // This means that all on-flight processes concerning the pipeline of current cluster are to be terminated
          // There is a possibility the event is received/read asynchronously, hence to wait won't be necessary
          if((n_img_prev_cl_ready * l2_n_tiles) == n_tiles_out[n_acc_stages_cl - 1]) {
          
            // Expect new invocations only if other images are expected to be processed
            if (n_img_prev_cl_ready < n_img) {
            
              while(1){

                soc_evt_msg = wait_cmd_eu_sw_evt(cluster_id, cluster_id - 1, CMD_TYPE_CL_START);  /* -- CMD_TYPE: CLUSTER START -- */ 
                
                #ifdef PRINT_PIPELINE_LOG
                  printf("<wait-for-invocation> - NEW: CL %d received message 0x%08x\n", hero_rt_cluster_id(), soc_evt_msg);
                #endif

                soc_evt_mst = get_soc_evt_cid(soc_evt_msg); // sender
                soc_evt_acc = get_soc_evt_aid(soc_evt_msg); // accelerator
                soc_evt_cmd = get_soc_evt_type(soc_evt_msg); // command

                /* Synchronous command */

                if(soc_evt_cmd == CMD_TYPE_CL_START){
                  n_img_prev_cl_ready++;
                  break;
                
                /* Asynchronous commands */

                } else if(soc_evt_cmd == CMD_TYPE_DMA_IN_TERMINATE) {
                  n_img_next_cl_used_on_flight--;
                  n_img_next_cl_used++;
                } else if(soc_evt_cmd == CMD_TYPE_DMA_OUT_TERMINATE) {
                  n_tiles_out_on_flight[soc_evt_acc]--;
                  n_tiles_out_not_checked[soc_evt_acc]++;
                } else if(soc_evt_cmd == CMD_TYPE_CL_TERMINATE) {
                  n_slv_cl_terminate[soc_evt_mst] = soc_evt_mst;

                /* Invalid commands */

                } else {
                  printf("<CMD_TYPE_CL_START> - ERROR: CL %d sent to CL %d an unexpected command %d\n", soc_evt_mst, hero_rt_cluster_id(), soc_evt_cmd);
                }
              
              }

            }

          }

        }

        /* Input tile */

        for(int acc_id = 0; acc_id < n_acc_stages_cl; acc_id++){

          int cluster_acc_id = n_acc_stages_cl * cluster_id + acc_id;

          // Check that each stage starts after the previous has processed an entire image
          if((!acc_id) || ((acc_id > 0) && (n_img_out[acc_id-1] > n_img_in[acc_id]))){

            /* ================== */
            /* Wait for input DMA */
            /* ================== */

            // Before waiting on the input DMA, let both transactions start on available buffers
            // This way after the wait period, a second transaction will already be running and its programming will hence be hidden
            if((run_id >= (acc_id * l2_n_tiles + dma_n_max_tx_on_flight)) && (n_tiles_in_on_flight[acc_id] > 0)){

              #ifdef PRINT_PIPELINE_LOG
                printf("dma_in_wait[%d]\n", acc_id);
              #endif

              // If double buffering, then wait for DMA transfer issued in previous round
              #ifdef PRINT_PIPELINE_LOG
                hero_dma_wait(dma_in[buffer_id + 2 * acc_id].job); 
              #endif
              
              // Count tiles been transferred in for each pipeline stage
              n_tiles_in_on_flight[acc_id]--;
              n_tiles_in[acc_id]++;

              // Count images been transferred in for each pipeline stage
              n_img_in[acc_id] = n_tiles_in[acc_id]/l2_n_tiles;

              /* ====================================================== */
              /* Notify previous cluster about image reading completion */
              /* ====================================================== */

              // If the application foresees a multi-cluster deployment
              if((n_clusters/2) > 1){

                // All clusters but first have to notify their predecessor
                if(cluster_id > 0){

                  // Check whether an entire image has been read
                  if(n_tiles_in[acc_id] == (n_img_in[acc_id] * l2_n_tiles)){

                    // Notify the stage[i-1] about the successful input DMA transfer, so as to free a buffer from already sunk data
                    send_cmd_eu_sw_evt(cluster_id, cluster_id - 1, acc_id, CMD_TYPE_DMA_IN_TERMINATE);  /* -- CMD_TYPE: DMA IN TERMINATE -- */ 
              
                  }

                }

              }

            }
            
            /* =========================== */
            /* Transfer input tile from L2 */
            /* =========================== */

            // Check that in the next round, accelerator will still need tiles to be read from L2 memory
            if((n_tiles_in[acc_id] + n_tiles_in_on_flight[acc_id]) < (l2_n_tiles * n_img)){

              // Check there is a free input buffer for the current accelerator
              if(n_tiles_in_on_flight[acc_id] < dma_n_max_tx_on_flight){

                // Check whether local buffer data have been used by the accelerator
                if((n_tiles_in[acc_id] + n_tiles_in_on_flight[acc_id] - n_tiles_compute[acc_id]) < dma_n_max_tx_on_flight){
              
                  #ifdef PRINT_PIPELINE_LOG
                    printf("dma_in_transfer[%d]\n", acc_id);
                  #endif

                  // NB: During the first round you cannot hide the DMA transfers

                  #ifndef DEBUG_DISABLE_PIPELINE_OPS
                    for (int i_dma = 0; i_dma < dma_n_tx; i_dma++) {
                      dma_in[buffer_id + 2 * acc_id].job = hero_memcpy_host2dev_async(l1_img[buffer_id], l2_img[buffer_id], dma_payload_dim * sizeof(uint32_t));
                    }
                  #endif

                  n_tiles_in_on_flight[acc_id]++;

                }

              }

            }

          }

          /* ======================== */
          /* Wait for pipeline stages */
          /* ======================== */

          // NB: This part of code is a WIP, as:
          // 1. The use a for loop around acc_id assumes the clusters to be containing the same number of accelerators, but this might not be true in general.
          // 2. As a temporaty approach, heterogeneous clusters (with different accelerators) are made by replicating the same acc-rich cluster multiple time 
          //    and targeting a different accelerator per cluster. Following this approach, "acc_id" is made to be the the accelerator ID in the cluster.

          // Check that each stage starts after a tile is ready in L1 memory
          if(n_tiles_compute_on_flight[acc_id] > 0){

            #ifdef PRINT_PIPELINE_LOG
              printf("compute_wait[%d]\n", acc_id);
            #endif

            // Wait accelerator execution

            #ifndef DEBUG_DISABLE_PIPELINE_OPS 
              if(cluster_id == get_acc_cid(RGB2HSV_CV_0))
                if(cluster_acc_id == get_acc_aid(RGB2HSV_CV_0))
                  while(!arov_is_finished(&arov, cluster_id, get_acc_aid(RGB2HSV_CV_0))){arov_wait_eu(&arov, get_acc_cid(RGB2HSV_CV_0), get_acc_aid(RGB2HSV_CV_0));}

              if(cluster_id == get_acc_cid(THRESHOLD_CV_1))
                if(cluster_acc_id == get_acc_aid(THRESHOLD_CV_1))
                  while(!arov_is_finished(&arov, cluster_id, get_acc_aid(THRESHOLD_CV_1))){arov_wait_eu(&arov, get_acc_cid(THRESHOLD_CV_1), get_acc_aid(THRESHOLD_CV_1));}

              if(cluster_id == get_acc_cid(ERODE_CV_2))
                if(cluster_acc_id == get_acc_aid(ERODE_CV_2))
                  while(!arov_is_finished(&arov, cluster_id, get_acc_aid(ERODE_CV_2))){arov_wait_eu(&arov, get_acc_cid(ERODE_CV_2), get_acc_aid(ERODE_CV_2));}

              if(cluster_id == get_acc_cid(DILATE_CV_3))
                if(cluster_acc_id == get_acc_aid(DILATE_CV_3))
                  while(!arov_is_finished(&arov, cluster_id, get_acc_aid(DILATE_CV_3))){arov_wait_eu(&arov, get_acc_cid(DILATE_CV_3), get_acc_aid(DILATE_CV_3));}

              if(cluster_id == get_acc_cid(DILATE_CV_4))
                if(cluster_acc_id == get_acc_aid(DILATE_CV_4))
                  while(!arov_is_finished(&arov, cluster_id, get_acc_aid(DILATE_CV_4))){arov_wait_eu(&arov, get_acc_cid(DILATE_CV_4), get_acc_aid(DILATE_CV_4));}

              if(cluster_id == get_acc_cid(ERODE_CV_5))
                if(cluster_acc_id == get_acc_aid(ERODE_CV_5))
                  while(!arov_is_finished(&arov, cluster_id, get_acc_aid(ERODE_CV_5))){arov_wait_eu(&arov, get_acc_cid(ERODE_CV_5), get_acc_aid(ERODE_CV_5));}
            #endif

            n_tiles_compute_on_flight[acc_id]--;
            n_tiles_compute[acc_id]++;

            // Swap accelerator buffers
            arov_swap_buffers_color_detect(&arov, cluster_id, !buffer_id, l1_img);

          }

          /* =================== */
          /* Run pipeline stages */
          /* =================== */

          // NB: This part of code is a WIP, as:
          // 1. The use a for loop around acc_id assumes the clusters to be containing the same number of accelerators, but this might not be true in general.
          // 2. As a temporaty approach, heterogeneous clusters (with different accelerators) are made by replicating the same acc-rich cluster multiple time 
          //    and targeting a different accelerator per cluster. Following this approach, "acc_id" is made to be the the accelerator ID in the cluster.

          // NB: This version of the code works only if each cluster holds a single pipeline stage

          // Check that each stage starts after a tile is ready in L1 memory
          if(n_tiles_in[acc_id] > n_tiles_compute[acc_id]){

            #ifdef PRINT_PIPELINE_LOG
              printf("compute_start[%d]\n", acc_id);
            #endif

            #ifndef DEBUG_DISABLE_PIPELINE_OPS 
              if(cluster_id == get_acc_cid(RGB2HSV_CV_0))
                if(cluster_acc_id == get_acc_aid(RGB2HSV_CV_0))
                  arov_compute(&arov, get_acc_cid(RGB2HSV_CV_0), get_acc_aid(RGB2HSV_CV_0)); 

              if(cluster_id == get_acc_cid(THRESHOLD_CV_1)) 
                if(cluster_acc_id == get_acc_aid(THRESHOLD_CV_1))
                  arov_compute(&arov, get_acc_cid(THRESHOLD_CV_1), get_acc_aid(THRESHOLD_CV_1));

              if(cluster_id == get_acc_cid(ERODE_CV_2)) 
                if(cluster_acc_id == get_acc_aid(ERODE_CV_2))
                  arov_compute(&arov, get_acc_cid(ERODE_CV_2), get_acc_aid(ERODE_CV_2));

              if(cluster_id == get_acc_cid(DILATE_CV_3)) 
                if(cluster_acc_id == get_acc_aid(DILATE_CV_3))
                  arov_compute(&arov, get_acc_cid(DILATE_CV_3), get_acc_aid(DILATE_CV_3));

              if(cluster_id == get_acc_cid(DILATE_CV_4)) 
                if(cluster_acc_id == get_acc_aid(DILATE_CV_4))
                  arov_compute(&arov, get_acc_cid(DILATE_CV_4), get_acc_aid(DILATE_CV_4));

              if(cluster_id == get_acc_cid(ERODE_CV_5)) 
                if(cluster_acc_id == get_acc_aid(ERODE_CV_5))
                  arov_compute(&arov, get_acc_cid(ERODE_CV_5), get_acc_aid(ERODE_CV_5));
            #endif

            n_tiles_compute_on_flight[acc_id]++;

          }

          /* ============================================== */
          /* Wait for output DMA to terminate data transfer */
          /* ============================================== */

          // Check if DMA out termination events have been received by other accelerator processes
          n_tiles_out[acc_id] += n_tiles_out_not_checked[acc_id];
          n_tiles_out_not_checked[acc_id] = 0;

          // Check if are still DMA output transaction to terminate
          if(n_tiles_out_on_flight[acc_id] > 0){

            while(1){

              soc_evt_msg = wait_cmd_eu_sw_evt(cluster_id, cluster_id + (n_clusters/2), CMD_TYPE_DMA_OUT_TERMINATE); /* -- CMD_TYPE: DMA OUT TERMINATE -- */
              
              #ifdef PRINT_PIPELINE_LOG
                printf("<wait-for-dma-out-end> - NEW: CL %d received message 0x%08x\n", hero_rt_cluster_id(), soc_evt_msg);
              #endif

              soc_evt_mst = get_soc_evt_cid(soc_evt_msg); // sender
              soc_evt_acc = get_soc_evt_aid(soc_evt_msg); // accelerator
              soc_evt_cmd = get_soc_evt_type(soc_evt_msg); // command

              /* Synchronous command */

              if(soc_evt_cmd == CMD_TYPE_DMA_OUT_TERMINATE){
                break;

              /* Asynchronous commands */

              } else if(soc_evt_cmd == CMD_TYPE_DMA_IN_TERMINATE) {
                n_img_next_cl_used_on_flight--;
                n_img_next_cl_used++;
              } else if(soc_evt_cmd == CMD_TYPE_CL_START){
                n_img_prev_cl_ready++;
              } else if(soc_evt_cmd == CMD_TYPE_CL_TERMINATE) {
                n_slv_cl_terminate[soc_evt_mst] = soc_evt_mst;

              /* Invalid commands */

              } else {
                printf("<CMD_TYPE_DMA_OUT_TERMINATE> - ERROR: CL %d sent to CL %d an unexpected command %d\n", soc_evt_mst, hero_rt_cluster_id(), soc_evt_cmd);
              }
            
            }

            // Count tiles been transferred out for each pipeline stage
            if(soc_evt_acc == acc_id){
              n_tiles_out_on_flight[acc_id]--;
              n_tiles_out[acc_id]++; 
            } else if(soc_evt_acc != acc_id){
              n_tiles_out_on_flight[soc_evt_acc]--;
              n_tiles_out_not_checked[soc_evt_acc]++;
            }

            // Count images been transferred out for each pipeline stage
            n_img_out[acc_id] = n_tiles_out[acc_id]/l2_n_tiles;

          }

          /* ========================== */
          /* Transfer output tile to L2 */
          /* ========================== */

          // Check that DMA output transaction starts after the correspondant stage has processed its tile
          if(n_tiles_compute[acc_id] > n_tiles_out[acc_id]){

            // Check there is a free output buffer for the current accelerator
            if(n_tiles_out_on_flight[acc_id] < dma_n_max_tx_on_flight){

              // Check whether there is at least one free L2 image buffer before to move data out
              // In case the DMA request comes from the last cluster, then the problem can be ignored 
              // The latter allows the adaptation also for a SCL case
              if(
                (cluster_id == (n_clusters / 2) - 1)
                || (((acc_id == (n_acc_stages_cl - 1)) && ((n_img_out[acc_id] - n_img_next_cl_used) < (l2_n_buffers / 2)))
                || ((acc_id < (n_acc_stages_cl - 1)) && ((n_img_out[acc_id] - n_img_in[acc_id + 1]) < (l2_n_buffers / 2))))
              ){

                #ifdef PRINT_PIPELINE_LOG
                  printf("dma_out_tx_initiate[%d]\n", acc_id);
                #endif
              
                // Wake up a cluster that is only used to mimic the bi-directional DMA for ouputs
                send_cmd_eu_sw_evt(cluster_id, cluster_id + (n_clusters/2), acc_id, CMD_TYPE_DMA_OUT_START); /* -- CMD_TYPE: DMA OUT START -- */
              
                n_tiles_out_on_flight[acc_id]++;

              }

            }

          }

          /* ========================================================== */
          /* Wait for next cluster to terminate on-flight image reading */
          /* ========================================================== */

          // If the application foresees a multi-cluster deployment
          if((n_clusters/2) > 1){

            // All clusters but last have to wait for their next to read all the buffered image
            if(cluster_id < ((n_clusters/2) - 1)){

              // If both L2 output image buffers are still to be read by next cluster, then current stalls
              // Also, if It's the last image and are still on-flight images, wait for events in order to sink them all
              if (((n_img_out[n_acc_stages_cl - 1] - n_img_next_cl_used) == (l2_n_buffers / 2)) 
                || ((n_img_next_cl_used_on_flight > 0) && (n_img_out[n_acc_stages_cl - 1] >= (n_img - 1)))
              ){

                while(1){

                  soc_evt_msg = wait_cmd_eu_sw_evt(cluster_id, cluster_id - 1, CMD_TYPE_DMA_IN_TERMINATE);  /* -- CMD_TYPE: CLUSTER START -- */ 
                  
                  #ifdef PRINT_PIPELINE_LOG
                    printf("<wait-for-next-cl-image-reading> - NEW: CL %d received message 0x%08x\n", hero_rt_cluster_id(), soc_evt_msg);
                  #endif

                  soc_evt_mst = get_soc_evt_cid(soc_evt_msg); // sender
                  soc_evt_acc = get_soc_evt_aid(soc_evt_msg); // accelerator
                  soc_evt_cmd = get_soc_evt_type(soc_evt_msg); // command

                  /* Synchronous command */

                  if(soc_evt_cmd == CMD_TYPE_DMA_IN_TERMINATE){
                    n_img_next_cl_used_on_flight--;
                    n_img_next_cl_used++;
                    break;

                  /* Asynchronous commands */

                  } else if(soc_evt_cmd == CMD_TYPE_DMA_OUT_TERMINATE) {
                    n_tiles_out_on_flight[soc_evt_acc]--;
                    n_tiles_out_not_checked[soc_evt_acc]++;
                  } else if(soc_evt_cmd == CMD_TYPE_CL_START){
                    n_img_prev_cl_ready++;
                  } else if(soc_evt_cmd == CMD_TYPE_CL_TERMINATE) {
                    n_slv_cl_terminate[soc_evt_mst] = soc_evt_mst;

                  /* Invalid commands */

                  } else {
                    printf("<CMD_TYPE_DMA_IN_TERMINATE> - ERROR: CL %d sent to CL %d an unexpected command %d\n", soc_evt_mst, hero_rt_cluster_id(), soc_evt_cmd);
                  }
                
                }

              }

            }

          }

          /* =========================== */
          /* Wake up next pipeline stage */
          /* =========================== */

          // Once this cluster terminates processing an image, It invokes the sucessive one.
          // The processing a cluster performs on an image is partial because each cluster integrates a different 
          // part of the whole accelerator-rich pipeline.

          // If the application foresees a multi-cluster deployment
          if((n_clusters/2) > 1){

            // All clusters but last can invoke their successor
            if(cluster_id < ((n_clusters/2) - 1)){

              // Check whether the next cluster has already been informed about the output image to be ready
              if (n_img_out[n_acc_stages_cl - 1] > (n_img_next_cl_used + n_img_next_cl_used_on_flight)){

                // Check whether the next cluster has a free L2 buffer for the new image to be processed
                if((n_img_next_cl_used_on_flight) < (l2_n_buffers / 2)){

                  #ifdef PRINT_PIPELINE_LOG
                    printf("wake up next cluster\n");
                  #endif

                  send_cmd_eu_sw_evt(cluster_id, cluster_id + 1, 0, CMD_TYPE_CL_START);  /* -- CMD_TYPE: CLUSTER START -- */ 

                  n_img_next_cl_used_on_flight++;
                
                }

              }

            }

          }

        }

      } else {

                            /* DMA out clusters */

        /* Sleep until next invocation */

        while(1){

          soc_evt_msg = wait_cmd_eu_sw_evt(cluster_id, cluster_id - (n_clusters/2), CMD_TYPE_DMA_OUT_START); /* -- CMD_TYPE: DMA OUT START -- */

          #ifdef PRINT_PIPELINE_LOG
            printf("<wait-for-dma-out-start> - NEW: CL %d received message 0x%08x\n", hero_rt_cluster_id(), soc_evt_msg);
          #endif

          soc_evt_mst = get_soc_evt_cid(soc_evt_msg); // sender
          soc_evt_acc = get_soc_evt_aid(soc_evt_msg); // accelerator
          soc_evt_cmd = get_soc_evt_type(soc_evt_msg); // command

          /* Synchronous command */

          if(soc_evt_cmd == CMD_TYPE_DMA_OUT_START){
            break;

          /* Invalid commands */

          } else {
            printf("<CMD_TYPE_DMA_OUT_START> - ERROR: CL %d sent to CL %d an unexpected command %d\n", soc_evt_mst, hero_rt_cluster_id(), soc_evt_cmd);
          }
        
        }

        /* Transfer output tile to L2 */

        // This implementation is like a tightly-coupled bounding between the output DMA 
        // and a proxy core inside an accelerator-rich cluster because the processor of the
        // additional cluster waits for the DMA, then notifies the former about transfer completion.

        #ifdef PRINT_PIPELINE_LOG
          printf("dma_out_tx_start[%d]\n", soc_evt_acc);
        #endif

        #ifndef DEBUG_DISABLE_PIPELINE_OPS 
          for (int i_dma = 0; i_dma < dma_n_tx; i_dma++) {
            hero_memcpy_dev2host(l2_img[2], l1_img[2], dma_payload_dim * sizeof(uint32_t));
          }
        #endif

        // Count tiles been transferred out for each pipeline stage
        n_tiles_out[soc_evt_acc]++;

        // Count images been transferred out for each pipeline stage
        n_img_out[soc_evt_acc] = n_tiles_out[soc_evt_acc]/l2_n_tiles;

        /* Send feedback about end of DMA transaction to MASTER CLUSTER */

        send_cmd_eu_sw_evt(cluster_id, cluster_id - (n_clusters/2), soc_evt_acc, CMD_TYPE_DMA_OUT_TERMINATE); /* -- CMD_TYPE: DMA OUT TERMINATE -- */
                  
      }

      // Count number of processed images (that passed all stages) 
      n_img_out_total = n_img_out[n_acc_stages_cl - 1];
      
      run_id++;

    } // n_img_out_total  

    /* Cleaning accelerators */

    // This routine is only executed by accelerator-rich clusters
    if(cluster_id < (n_clusters/2)){

      // -- RGB2HSV_CV_0 
      if(cluster_id == get_acc_cid(RGB2HSV_CV_0))
        arov_free(&arov, get_acc_cid(RGB2HSV_CV_0), get_acc_aid(RGB2HSV_CV_0));

      // -- THRESHOLD_CV_1 
      if(cluster_id == get_acc_cid(THRESHOLD_CV_1))
        arov_free(&arov, get_acc_cid(THRESHOLD_CV_1), get_acc_aid(THRESHOLD_CV_1));
        
      // -- ERODE_CV_2
      if(cluster_id == get_acc_cid(ERODE_CV_2))
        arov_free(&arov, get_acc_cid(ERODE_CV_2),   get_acc_aid(ERODE_CV_2));

      // -- DILATE_CV_3
      if(cluster_id == get_acc_cid(DILATE_CV_3))
        arov_free(&arov, get_acc_cid(DILATE_CV_3),  get_acc_aid(DILATE_CV_3));

      // -- DILATE_CV_4
      if(cluster_id == get_acc_cid(DILATE_CV_4))
        arov_free(&arov, get_acc_cid(DILATE_CV_4),  get_acc_aid(DILATE_CV_4));

      // -- ERODE_CV_5
      if(cluster_id == get_acc_cid(ERODE_CV_5))
        arov_free(&arov, get_acc_cid(ERODE_CV_5),   get_acc_aid(ERODE_CV_5));

    }

    /* ===================================================================== */

    #ifdef PRINT_PIPELINE_LOG
      printf("...THE END\n");
    #endif

    /* MEASUREMENT - END */

    // Cluster synchronization barrier
    cluster_barrier_all_eu_soc_evt(cluster_id, 0, n_slv_cl_terminate);

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

  } // job_id

  /* Cleaning L1 */
  
  #if defined(_pulp_rt_)
    hero_l1free(l1_base_address);
  #endif
}

#endif