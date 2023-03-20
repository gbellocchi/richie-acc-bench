/* =====================================================================
 * Project:      Color detect
 * Title:        l2_pipeline_scl_no_tile.c
 * Description:  Implementation of an accelerator-rich vision pipeline 
 *               in a PULP cluster. Each accelerator stage needs to process
 *               an entire image before
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

#if (BENCHMARK_TYPE == L2_PIPELINE_SCL_NO_TILE)

#include <experiment.h>
#include <cluster_synch.h>

#include <stimuli.h>

// #define PRINT_PIPELINE_LOG

void run_benchmark(const int cluster_id, const int core_id) {

  /* Runtime IDs */

  int buffer_id; // Buffer selector
  int run_id; // Execution run

  /* Runtime indicators */
  
  int n_img_out_total;

  int n_img_in[n_acc_active], n_img_out[n_acc_active];
  int n_tiles_in[n_acc_active], n_tiles_in_on_flight[n_acc_active];
  int n_tiles_out[n_acc_active], n_tiles_out_on_flight[n_acc_active], n_tiles_out_not_checked[n_acc_active];
  int n_tiles_processed[n_acc_active];

  /* SoC events */

  uint32_t soc_evt_msg, soc_evt_mst, soc_evt_acc, soc_evt_cmd;

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

  pulp_dma_struct dma_in[n_acc_active * 2], dma_out[n_acc_active * 2], dma_wait[n_acc_active * 2];

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
    
    run_id = 0;

    for(int acc_id=0; acc_id<n_acc_active; acc_id++){
      n_img_in[acc_id] = 0;
      n_img_out[acc_id] = 0;
      n_tiles_in[acc_id] = 0;
      n_tiles_in_on_flight[acc_id] = 0;
      n_tiles_out[acc_id] = 0;
      n_tiles_out_not_checked[acc_id] = 0;
      n_tiles_out_on_flight[acc_id] = 0;
      n_tiles_processed[acc_id] = 0;
    }

    n_img_out_total = 0;

    while(n_img_out_total < n_img){

                            /* Accelerator-rich cluster routine */

      if(cluster_id < (n_clusters/2)){

        if(run_id==0){

          // Initialize selector          
          buffer_id = 0;

          // Program accelerators
          for(int acc_id=0; acc_id<n_acc_active; acc_id++){
            arov_init(&arov, cluster_id, acc_id);
            arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img[0], l1_img[2], l1_buffer_dim);
            offload_id[acc_id] = arov_activate(&arov, cluster_id, acc_id);
            arov_program(&arov, cluster_id, acc_id);
          }

        } else {
          
          // Swap buffer ID
          buffer_id = !buffer_id;

          for(int acc_id=0; acc_id<n_acc_active; acc_id++){
            if( (!acc_id) || ((acc_id > 0) && (n_img_out[acc_id-1] > n_img_in[acc_id])) ){
              // Swap buffer pointers
              arov_swap_buffers_color_detect(&arov, cluster_id, buffer_id, l1_img);
            }
          }

        }

        /* Input tile */

        for(int acc_id=0; acc_id<n_acc_active; acc_id++){

          // Check that each stage starts after the previous has processed an entire image
          if((!acc_id) || ((acc_id > 0) && (n_img_out[acc_id-1] > n_img_in[acc_id]))){
            
            /* =========================== */
            /* Transfer input tile from L2 */
            /* =========================== */

            // Check that in the next round, accelerator will still need tiles to be read from L2 memory
            if((n_tiles_in[acc_id] + n_tiles_in_on_flight[acc_id]) < (l2_n_tiles * n_img)){
              
              #ifdef PRINT_PIPELINE_LOG
                printf("dma_in_transfer[%d]\n", acc_id);
              #endif

              // NB: During the first round you cannot hide the DMA transfers
              for (int i_dma = 0; i_dma < dma_n_tx; i_dma++) {
                dma_in[buffer_id + 2 * acc_id].job = hero_memcpy_host2dev_async(l1_img[buffer_id], l2_img[buffer_id], dma_payload_dim * sizeof(uint32_t));
              }

              n_tiles_in_on_flight[acc_id]++;

            }

            /* ================== */
            /* Wait for input DMA */
            /* ================== */

            #ifdef PRINT_PIPELINE_LOG
              printf("dma_in_wait[%d]\n", acc_id);
            #endif

            if((run_id > (acc_id * l2_n_tiles)) && (n_tiles_in_on_flight[acc_id] > 0)){

              // If double buffering, then wait for DMA transfer issued in previous round
              hero_dma_wait(dma_in[!buffer_id + 2 * acc_id].job); 
              
              n_tiles_in_on_flight[acc_id]--;
              n_tiles_in[acc_id]++;

            }

          }
        }

        /* ============================================== */
        /* Wait for output DMA to terminate data transfer */
        /* ============================================== */

        for(int acc_id=0; acc_id<n_acc_active; acc_id++){

          // Check if DMA out termination events have been received by other accelerator processes
          if(n_tiles_out_not_checked[acc_id] > 0){
            n_tiles_out[acc_id] += n_tiles_out_not_checked[acc_id];
            n_tiles_out_not_checked[acc_id] = 0;
          }

          // Check if are still DMA output transaction to terminate
          if(n_tiles_processed[acc_id] > n_tiles_out[acc_id]){

            #ifdef PRINT_PIPELINE_LOG
              printf("dma_out_wait[%d]\n", acc_id);
            #endif

            while(1){

              soc_evt_msg = wait_cmd_eu_sw_evt(cluster_id, cluster_id + (n_clusters/2), CMD_TYPE_DMA_OUT_TERMINATE); /* -- CMD_TYPE: DMA OUT TERMINATE -- */
              
              #ifdef PRINT_PIPELINE_LOG
                printf("<wait_cmd_eu_sw_evt> - NEW: CL %d received message 0x%08x\n", hero_rt_cluster_id(), soc_evt_msg);
              #endif

              soc_evt_mst = get_soc_evt_cid(soc_evt_msg); // sender
              soc_evt_acc = get_soc_evt_aid(soc_evt_msg); // accelerator
              soc_evt_cmd = get_soc_evt_type(soc_evt_msg); // command

              if(soc_evt_cmd == CMD_TYPE_DMA_OUT_TERMINATE){
                break;
              } else {
                printf("<wait_cmd_eu_sw_evt> - ERROR: CL %d received unexpected command %d\n", hero_rt_cluster_id(), soc_evt_cmd);
              }
            
            }

            // Count tiles been transferred out for each pipeline stage
            if(soc_evt_acc == acc_id){
              n_tiles_out[acc_id]++; 
            } else if(soc_evt_acc != acc_id){
              n_tiles_out_not_checked[soc_evt_acc]++;
            }

            // Count images been transferred out for each pipeline stage
            n_img_out[acc_id] = n_tiles_out[acc_id]/l2_n_tiles;

          }
        }

        /* =================== */
        /* Run pipeline stages */
        /* =================== */

        for(int acc_id=0; acc_id<n_acc_active; acc_id++){

          // Check that each stage starts after a tile is ready in L1 memory
          if(n_tiles_in[acc_id] > n_tiles_processed[acc_id]){

            #ifdef PRINT_PIPELINE_LOG
              printf("compute_start[%d]\n", acc_id);
            #endif

            arov_compute(&arov, cluster_id, acc_id);

          }
        }

        for(int acc_id=0; acc_id<n_acc_active; acc_id++){

          /* ======================== */
          /* Wait for pipeline stages */
          /* ======================== */

          // Check that each stage starts after a tile is ready in L1 memory
          if(n_tiles_in[acc_id] > n_tiles_processed[acc_id]){

            #ifdef PRINT_PIPELINE_LOG
              printf("compute_wait[%d]\n", acc_id);
            #endif

            // Wait accelerator execution
            while(!arov_is_finished(&arov, cluster_id, acc_id)){

              arov_wait_eu(&arov, cluster_id, acc_id);

            }

            n_tiles_processed[acc_id]++;

          }

          /* ========================== */
          /* Transfer output tile to L2 */
          /* ========================== */

          // Check that DMA output transaction starts after the correspondant stage has processed its tile
          if(n_tiles_processed[acc_id] > n_tiles_out[acc_id]){

            #ifdef PRINT_PIPELINE_LOG
              printf("dma_out_tx[%d]\n", acc_id);
            #endif
          
            // Wake up a cluster that is only used to mimic the bi-directional DMA for ouputs
            send_cmd_eu_sw_evt(cluster_id, cluster_id + (n_clusters/2), acc_id, CMD_TYPE_DMA_OUT_START); /* -- CMD_TYPE: DMA OUT START -- */
            
          }

        }

      } else {

                            /* DMA out clusters */

        /* Sleep until next invocation */

        while(1){

          soc_evt_msg = wait_cmd_eu_sw_evt(cluster_id, cluster_id - (n_clusters/2), CMD_TYPE_DMA_OUT_START); /* -- CMD_TYPE: DMA OUT START -- */

          #ifdef PRINT_PIPELINE_LOG
            printf("<wait_cmd_eu_sw_evt> - NEW: CL %d received message 0x%08x\n", hero_rt_cluster_id(), soc_evt_msg);
          #endif

          soc_evt_mst = get_soc_evt_cid(soc_evt_msg); // sender
          soc_evt_acc = get_soc_evt_aid(soc_evt_msg); // accelerator
          soc_evt_cmd = get_soc_evt_type(soc_evt_msg); // command

          if(soc_evt_cmd == CMD_TYPE_DMA_OUT_START){
            break;
          } else {
            printf("<wait_cmd_eu_sw_evt> - ERROR: CL %d received unexpected command %d\n", hero_rt_cluster_id(), soc_evt_cmd);
          }
        
        }

        /* Transfer output tile to L2 */

        // This implementation is like a tightly-coupled bounding between the output DMA 
        // and a proxy core inside an accelerator-rich cluster because the processor of the
        // additional cluster waits for the DMA, then notifies the former about transfer completion.

        #ifdef PRINT_PIPELINE_LOG
          printf("dma_out_tx[%d]\n", soc_evt_acc);
        #endif

        for (int i_dma = 0; i_dma < dma_n_tx; i_dma++) {
          hero_memcpy_dev2host(l2_img[2], l1_img[2], dma_payload_dim * sizeof(uint32_t));
        }

        // Count tiles been transferred out for each pipeline stage
        n_tiles_out[soc_evt_acc]++;

        // Count images been transferred out for each pipeline stage
        n_img_out[soc_evt_acc] = n_tiles_out[soc_evt_acc]/l2_n_tiles;

        /* Send feedback about end of DMA transaction to MASTER CLUSTER */

        send_cmd_eu_sw_evt(cluster_id, cluster_id - (n_clusters/2), soc_evt_acc, CMD_TYPE_DMA_OUT_TERMINATE); /* -- CMD_TYPE: DMA OUT TERMINATE -- */
                  
      }

      // Count number of processed images (that passed all stages) 
      n_img_out_total = n_img_out[(n_acc_stages/(n_clusters/2)) - 1];
      
      run_id++;

    } // n_img_out_total  

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