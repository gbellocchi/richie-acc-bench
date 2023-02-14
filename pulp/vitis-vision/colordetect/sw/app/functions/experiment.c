/* =====================================================================
 * Project:      System model
 * Title:        perf_refs.c
 * Description:  Application-level performance counter functions.
 *
 * $Date:        28.10.2022
 * ===================================================================== */
/*
 * Copyright (C) 2022 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#include <experiment.h>
#include <cluster_synch.h>
#include <configs.h>

#include <stimuli.h>

void run_l2_experiment(const int cluster_id, const int core_id) {

  /* ===================================================================== */

  int experiment_id = 0;

  /* Define cache stats */

  int hit[2], trns[2], miss[2];

  int reg_hit, reg_trns, reg_miss;

  /* Cycle counters */

  pulp_clk_struct t_experiment_sys_pov;

  /* ===================================================================== */

  /* Initialize L2 memory */

  // Variables
  unsigned l2_n_cl_per_port               = n_clusters/n_l2_ports_virt;
  unsigned l2_n_bytes_per_port            = l2_size_B/n_l2_ports_phy;
  unsigned l2_n_words_per_port            = l2_n_bytes_per_port/sizeof(uint32_t);
  
  // Calculate port ID (Optional: L2 cluster port offset)
  unsigned l2_cl_port_id                  = cluster_id/l2_n_cl_per_port + l2_cl_port_id_offset;

  // Declare L2 cluster base address  
  DEVICE_PTR_CONST l2_cl_base = (l2_cl_port_id==0) ? \
                                  arov_l2_heap() : \
                                    // bank 0 holds also the program, so buffers are allocated starting from the heap
                                  arov_l2_base() + l2_cl_port_id * l2_n_bytes_per_port;
                                    // same as the HW of the SoC bus 

  // Declare L2 cluster buffer address  
  DEVICE_PTR_CONST l2_cl_addr             = l2_cl_base + (cluster_id - (l2_cl_port_id - l2_cl_port_id_offset) * l2_n_cl_per_port) * dma_payload_dim_stop;

  // Declare read buffer address 
  DEVICE_PTR_CONST l2_r_reqs_data         = l2_cl_addr;
  // DEVICE_PTR_CONST l2_w_reqs_data         = l2_cl_addr + l1_buffer_dim_stop;

  #if defined(PRINT_LOG)
    printf(" # - [Params] ID_L2_port:             %d \n",           (int32_t)l2_cl_port_id);
    printf(" # - [Params] Base_address_L2:        %p \n",           (int32_t)l2_cl_base);
    printf(" # - [Params] Buffer_address_L2:      %p \n",           (int32_t)l2_cl_addr);
  #endif

  #ifdef INPUT_INIT

    for(int i=0; i<l1_buffer_dim_stop; i++){
      pulp_write32(l2_r_reqs_data+i*sizeof(int32_t), in_img_small[i]);
      // pulp_write32(l2_w_reqs_data+i*sizeof(int32_t), 0);

      // 128x128_golden_out_img
    }
  #endif

  /* Initialize L1 memory */

  #if defined(_pulp_rt_)
    __device uint32_t* l1_arov_buffer   = (__device uint32_t*)hero_l1malloc(sizeof(uint32_t) * l1_arov_buffer_dim);
  #elif defined(_pulp_bare_)
    DEVICE_PTR_CONST l1_arov_buffer    = arov_l1_heap(cluster_id) + n_l1_ports * sizeof(uint32_t);
  #endif

  /* System */

  pulp_dma_struct dma_in[n_hwpe_stop], dma_out[n_hwpe_stop], dma_wait[n_hwpe_stop];

  /* Accelerators */

  uint32_t n_total_reqs, t_ck_reqs, t_ck_idle;
  
  int offload_id[n_hwpe_stop];

  /* Allocate accelerator-rich overlay */

  arov_struct arov;

  /* Cluster steady state condition */
  
  cluster_barrier_eu_soc_evt(cluster_id, experiment_id);
  if(!cluster_id) cluster_slv_restart_eu_soc_evt(cluster_id, experiment_id);

  /* Design space exploration */

  int n_hwpe_active     = n_hwpe_total;
  int l1_buffer_dim     = l1_buffer_dim_stop;
  int dma_payload_dim   = dma_payload_dim_stop;

  for(int n_reps=n_reps_start; n_reps<=n_reps_stop; n_reps+=n_reps){

    /* Runtime experiment parameters */

    // - Request time (burst length)
    t_ck_reqs                      = 1; 

    // - Idle time
    t_ck_idle                      = 0;  

    // - Overall number of requests
    n_total_reqs                   = l1_buffer_dim * n_reps;

    /* Program accelerators */

    for(int acc_id=0; acc_id<n_hwpe_active; acc_id++){

      arov_init(&arov, cluster_id, acc_id);
      
      arov_map_params_color_detect(
        &arov, 
        cluster_id, 
        acc_id,
        l1_arov_buffer,
        l1_color_detect_buffer_dim,
        l1_buffer_dim, 
        1, 
        1,
        n_total_reqs, 
        t_ck_reqs, 
        t_ck_idle, 
        l1_buffer_dim,
        n_reps,
        n_l1_ports);

      offload_id[acc_id] = arov_activate(&arov, cluster_id, acc_id);

      arov_program(&arov, cluster_id, acc_id);
    }

    /* Flush cache (cold-cache condition) */

    icache_flush_all();

    /* Launch profiling jobs */

    for(int job_id=0; job_id<exp_len_job_queue; job_id++){

      /* ===================================================================== */

      /* Reset and start PULP counter */

      hero_reset_clk_counter();
      hero_start_clk_counter();

      /* Reset cache stats */

      if(cluster_id==0 && core_id==0){

        icache_stats_reset();

        reg_hit   = 0;
        reg_trns  = 0;
        reg_miss  = 0;

        /* Reset performance counters */

        hero_perf_reset_all();

      }

      /* ===================================================================== */

      /* MEASUREMENT - START */

      // cache statistics and performance counters
      start_measurement(cluster_id, core_id, hit, trns, miss);

      // Cluster synchronization barrier
      cluster_barrier_eu_soc_evt(cluster_id, experiment_id);

      // Cluster timer
      if(!cluster_id) t_experiment_sys_pov.cnt_0 = hero_get_clk_counter();

      // Activate clusters simultaneously
      // NB: Activation must be after the starting of time measurements
      if(!cluster_id) cluster_slv_restart_eu_soc_evt(cluster_id, experiment_id);

      /* ===================================================================== */

      /* ==================== */
      /*  Profiling - DMA in  */
      /* ==================== */

      /* Start by issuing data transfers to L1 for each accelerator */

      for(int acc_id=0; acc_id<n_hwpe_active; acc_id++){

        DEVICE_PTR l1_addr = l1_arov_buffer; // (DEVICE_PTR)(arov_get_dev_r_reqs_addr(&arov, cluster_id, acc_id));
        DEVICE_PTR l2_addr = l2_r_reqs_data;

        for (int i = 0; i < n_dma_tx; i++) {
          dma_in[acc_id].job = hero_memcpy_host2dev_async(l1_addr, l2_addr, dma_payload_dim * sizeof(uint32_t));
        }

      }

      /* ===================================================================== */

      /* ===================== */
      /*  Profiling - COMPUTE  */
      /* ===================== */

      /* Wait DMA to terminate data transfer, then launch computation */

      for(int acc_id=0; acc_id<n_hwpe_active; acc_id++){
        hero_dma_wait(dma_in[acc_id].job);
        // if (l1_buffer_dim>0) arov_compute(&arov, cluster_id, acc_id);
        if (l1_buffer_dim>0) pulp_write32(0x1b202000 + acc_id * 0x200, 0);
      }

      /* Wait for computation to terminate, then issue data transfers to L2 */

      for(int acc_id=0; acc_id<n_hwpe_active; acc_id++){
        
        if (l1_buffer_dim>0){ 
          while(!arov_is_finished(&arov, cluster_id, acc_id)){
            arov_wait_eu(&arov, cluster_id, acc_id);
          }
          // arov_wait_polling(&arov, cluster_id, acc_id);
        }

        DEVICE_PTR l1_addr = l1_arov_buffer; // (DEVICE_PTR)(arov_get_dev_w_reqs_addr(&arov, cluster_id, acc_id));
        DEVICE_PTR l2_addr = l2_r_reqs_data;

        for (int i = 0; i < n_dma_tx; i++) {
          // dma_out[acc_id].job = hero_memcpy_dev2host_async(l2_addr, l1_addr, dma_payload_dim * sizeof(uint32_t));
          dma_out[acc_id].job = hero_memcpy_dev2host_async_no_trigger(l2_addr, l1_addr, dma_payload_dim * sizeof(uint32_t));
        }

      }

      /* ===================================================================== */

      /* ===================== */
      /*  Profiling - DMA out  */
      /* ===================== */

      /* Wait DMA to terminate data transfer */

      // for(int acc_id=0; acc_id<n_hwpe_active; acc_id++){
      //   hero_dma_wait(dma_out[acc_id].job);
      // }

      /* ===================================================================== */

      /* MEASUREMENT - END */

      // Cluster synchronization barrier
      cluster_barrier_eu_soc_evt(cluster_id, experiment_id);

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
        experiment_id, 
        job_id, 
        "L2", 
        // Runtime parameters
        n_hwpe_active, 
        dma_payload_dim, 
        l1_buffer_dim, 
        n_reps,
        n_total_reqs,
        // Cache
        &reg_hit,
        &reg_trns, 
        &reg_miss, 
        // Clock counters
        0xFFFFFFFF,
        &t_experiment_sys_pov
      );

      /* Restart clusters */

      if(!cluster_id) cluster_slv_restart_eu_soc_evt(cluster_id, experiment_id);

    } // job_id

    /* Cleaning accelerators */

    for(int acc_id=0; acc_id<n_hwpe_active; acc_id++){
      arov_free(&arov, cluster_id, acc_id);
    }

    /* Cleaning L1 and L2 */
    
    #if defined(_pulp_rt_)
      hero_l3free(l2_r_reqs_data);
      hero_l3free(l2_w_reqs_data);
      hero_l1free(l1_arov_buffer);
    #endif
  
    experiment_id++;

  } // n_reps
}