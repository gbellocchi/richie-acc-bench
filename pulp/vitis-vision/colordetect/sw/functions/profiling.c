/* =====================================================================
 * Project:      System model
 * Title:        profiling.c
 * Description:  Multi cluster scaling
 *
 * $Date:        30.6.2022
 * ===================================================================== */
/*
 * Copyright (C) 2022 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#include "experiment.h"
#include "configs.h"

/* ===================================================================== */

/* ====================== */
/*  Print job statistics  */
/* ====================== */

int print_job_stats( 
  // System
  const int cluster_id, 
  const int core_id,
  // Experiment
  const int experiment_id, 
  const int job_id, 
  const char *component_name,
  // Runtime parameters
  int n_reps,
  int n_total_reqs,
  // Cache
  int *reg_hit, 
  int *reg_trns, 
  int *reg_miss,
  // Clock counters
  pulp_clk_struct *t_experiment_cl_pov,
  pulp_clk_struct *t_experiment_sys_pov
){

  if(cluster_id==0 && core_id==0){

    /* Deactivate performance registers */

    // Read performance counters

    #if defined(_allocate_use_perf_event_cycle_)
      const int64_t actual_perf_cycle = hero_perf_read(hero_perf_event_cycle);
    #endif

    #if defined(_allocate_perf_event_instr_retired_)
      const int64_t actual_perf_instr_retired = hero_perf_read(hero_perf_event_instr_retired);
    #endif

    #if defined(_allocate_perf_event_stall_load_)
      const int64_t actual_perf_stall_load = hero_perf_read(hero_perf_event_stall_load);
    #endif

    #if defined(_allocate_perf_event_load_)
      const int64_t actual_perf_load = hero_perf_read(hero_perf_event_load);
    #endif

    #if defined(_allocate_perf_event_store_)
      const int64_t actual_perf_store = hero_perf_read(hero_perf_event_store);
    #endif

    #if defined(_allocate_perf_event_load_external_)
      const int64_t actual_perf_load_external = hero_perf_read(hero_perf_event_load_external);
    #endif

    #if defined(_allocate_perf_event_store_external_)
      const int64_t actual_perf_store_external = hero_perf_read(hero_perf_event_store_external);
    #endif

    // Print profiling results

    printf(" # ============================================================================\n");
    printf(" # [EXP-%d] [JOB-%d] [CL-%d] [CORE-%d] Profiling %s\n\n", experiment_id, job_id, cluster_id, core_id, component_name);

    printf(" #\n");

    // Runtime parameters

      printf(" # - [Params] N_clusters:             %d \n",           (int32_t)n_clusters);
      printf(" # - [Params] N_acc:                  %d \n",           (int32_t)n_hwpe_active);
      printf(" # - [Params] N_reps:                 %d \n",           (int32_t)n_reps);
      printf(" # - [Params] DIM_dma_payload:        %d \n",           (int32_t)dma_payload_dim);
      printf(" # - [Params] DIM_buffer:             %d \n",           (int32_t)l1_buffer_dim);
      printf(" # - [Params] DIM_burst:              %d \n",           (int32_t)n_total_reqs);
      printf(" # - [Params] N_l1_banks:             %d \n",           (int32_t)n_l1_ports);
      printf(" # - [Params] N_L2_ports:             %d \n",           (int32_t)n_l2_ports_phy);

    printf(" #\n");

    // Performance counters

    #if defined(_allocate_use_perf_event_cycle_)
      printf(" # - [Perf-%d] CYCLES:                   %d\n",         core_id, (int32_t)actual_perf_cycle);
    #endif

    #if defined(_allocate_perf_event_instr_retired_)
      printf(" # - [Perf-%d] INSTR:                    %d\n",         core_id, (int32_t)actual_perf_instr_retired);
    #endif

    #if defined(_allocate_perf_event_stall_load_)
      printf(" # - [Perf-%d] STALL_LD:                 %d\n",         core_id, (int32_t)actual_perf_stall_load);
    #endif

    #if defined(_allocate_perf_event_load_)
      printf(" # - [Perf-%d] LD:                       %d\n",         core_id, (int32_t)actual_perf_load);
    #endif

    #if defined(_allocate_perf_event_store_)
      printf(" # - [Perf-%d] ST:                       %d\n",         core_id, (int32_t)actual_perf_store);
    #endif

    #if defined(_allocate_perf_event_load_external_)
      printf(" # - [Perf-%d] LD_EXT:                     %d\n",       core_id, (int32_t)actual_perf_load_external);
    #endif

    #if defined(_allocate_perf_event_store_external_)
      printf(" # - [Perf-%d] ST_EXT:                     %d\n",       core_id, (int32_t)actual_perf_store_external);
    #endif

    // Cache

    printf(" # - [Cache] Number of transactions:  %d \n", *reg_trns);
    printf(" # - [Cache] HIT count:               %d \n", *reg_hit);
    printf(" # - [Cache] MISS count:              %d \n", *reg_miss);

    // Cycle counters

    if(t_experiment_cl_pov != 0xFFFFFFFF){
      printf(" # - [CK-Cnt] T_meas_job (CL-pov):  %d \n", t_experiment_cl_pov->cnt_1 - t_experiment_cl_pov->cnt_0);
    }
    if(t_experiment_sys_pov != 0xFFFFFFFF){
      printf(" # - [CK-Cnt] T_meas_job (SYS-pov): %d \n", t_experiment_sys_pov->cnt_1 - t_experiment_sys_pov->cnt_0);
    }
    printf(" # ============================================================================\n");

  }
}

/* ===================================================================== */

/* =========== */
/*  Profiling  */
/* =========== */

int profiling(const int cluster_id, const int core_id)
{

  /* Initialize cache */ 

  icache_stats_init(cluster_id);

  if(cluster_id==0 && core_id==0){

    /* Initialize performance counters */

    #if defined(_pulp_bare_)
      const int init = hero_perf_init_bare(cluster_id, core_id, arov_l1_heap(cluster_id));
    #endif

    #if defined(_pulp_rt_)
      const int init = hero_perf_init();
    #endif

    if (init != 0) {
      printf("Error initializing performance measurement: %d!\n", init);
      return 1;
    }

    /* Allocate performance counters */

    // Bare Perf APIs definition is to be found in libhero-target
    // _pulp_bare_ macro can be enabled/disabled in configs.h (in case of disabling, use _pulp_rt_)

    #if defined(_pulp_bare_)

      #if defined(_allocate_use_perf_event_cycle_)
        if (hero_perf_alloc_bare(hero_perf_event_cycle) != 0) {
          printf("Error allocating counter for hero_perf_event_cycle!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_instr_retired_)
        if (hero_perf_alloc_bare(hero_perf_event_instr_retired) != 0) {
          printf("Error allocating counter for hero_perf_event_instr_retired!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_stall_load_)
        if (hero_perf_alloc_bare(hero_perf_event_stall_load) != 0) {
          printf("Error allocating counter for hero_perf_event_stall_load!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_load_)
        if (hero_perf_alloc_bare(hero_perf_event_load) != 0) {
          printf("Error allocating counter for hero_perf_event_load!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_store_)
        if (hero_perf_alloc_bare(hero_perf_event_store) != 0) {
          printf("Error allocating counter for hero_perf_event_store!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_load_external_)
        if (hero_perf_alloc_bare(hero_perf_event_load_external) != 0) {
          printf("Error allocating counter for hero_perf_event_load_external!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_store_external_)
        if (hero_perf_alloc_bare(hero_perf_event_store_external) != 0) {
          printf("Error allocating counter for hero_perf_event_store_external!\n");
          return 1;
        }
      #endif

    #endif

    // Perf APIs definition is to be found in libhero-target
    // _pulp_rt_ macro can be enabled/disabled in configs.h (in case of disabling, use _pulp_bare_)

    #if defined(_pulp_rt_)

      #if defined(_allocate_use_perf_event_cycle_)
        if (hero_perf_alloc(hero_perf_event_cycle) != 0) {
          printf("Error allocating counter for hero_perf_event_cycle!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_instr_retired_)
        if (hero_perf_alloc(hero_perf_event_instr_retired) != 0) {
          printf("Error allocating counter for hero_perf_event_instr_retired!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_stall_load_)
        if (hero_perf_alloc(hero_perf_event_stall_load) != 0) {
          printf("Error allocating counter for hero_perf_event_stall_load!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_load_)
        if (hero_perf_alloc(hero_perf_event_load) != 0) {
          printf("Error allocating counter for hero_perf_event_load!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_store_)
        if (hero_perf_alloc(hero_perf_event_store) != 0) {
          printf("Error allocating counter for hero_perf_event_store!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_load_external_)
        if (hero_perf_alloc(hero_perf_event_load_external) != 0) {
          printf("Error allocating counter for hero_perf_event_load_external!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_store_external_)
        if (hero_perf_alloc(hero_perf_event_store_external) != 0) {
          printf("Error allocating counter for hero_perf_event_store_external!\n");
          return 1;
        }
      #endif

    #endif
  
  }

  /* Launch profiling */
  
  #if defined(_profile_1cl_small_)
    run_1cl_small(cluster_id, core_id);
  #endif

  if(cluster_id==0 && core_id==0){

    /* Deallocate counters */

    // Bare Perf APIs definition is to be found in libhero-target
    // _pulp_bare_ macro can be enabled/disabled in configs.h (in case of disabling, use _pulp_rt_)

    #if defined(_pulp_bare_)
    
      #if defined(_allocate_use_perf_event_cycle_)
        if (hero_perf_dealloc_bare(hero_perf_event_cycle) != 0) {
          printf("Error deallocating counter for hero_perf_event_cycle\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_instr_retired_)
        if (hero_perf_dealloc_bare(hero_perf_event_instr_retired) != 0) {
          printf("Error deallocating counter for hero_perf_event_instr_retired\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_stall_load_)
        if (hero_perf_dealloc_bare(hero_perf_event_stall_load) != 0) {
          printf("Error deallocating counter for hero_perf_event_stall_load!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_load_)
        if (hero_perf_dealloc_bare(hero_perf_event_load) != 0) {
          printf("Error deallocating counter for hero_perf_event_load!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_store_)
        if (hero_perf_dealloc_bare(hero_perf_event_store) != 0) {
          printf("Error deallocating counter for hero_perf_event_store!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_load_external_)
        if (hero_perf_dealloc_bare(hero_perf_event_load_external) != 0) {
          printf("Error deallocating counter for hero_perf_event_load_external!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_store_external_)
        if (hero_perf_dealloc_bare(hero_perf_event_store_external) != 0) {
          printf("Error deallocating counter for hero_perf_event_store_external!\n");
          return 1;
        }
      #endif

    #endif

    // Perf APIs definition is to be found in libhero-target
    // _pulp_rt_ macro can be enabled/disabled in configs.h (in case of disabling, use _pulp_bare_)

    #if defined(_pulp_rt_)

      #if defined(_allocate_use_perf_event_cycle_)
        if (hero_perf_dealloc(hero_perf_event_cycle) != 0) {
          printf("Error deallocating counter for hero_perf_event_cycle\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_instr_retired_)
        if (hero_perf_dealloc(hero_perf_event_instr_retired) != 0) {
          printf("Error deallocating counter for hero_perf_event_instr_retired\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_stall_load_)
        if (hero_perf_dealloc(hero_perf_event_stall_load) != 0) {
          printf("Error deallocating counter for hero_perf_event_stall_load!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_load_)
        if (hero_perf_dealloc(hero_perf_event_load) != 0) {
          printf("Error deallocating counter for hero_perf_event_load!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_store_)
        if (hero_perf_dealloc(hero_perf_event_store) != 0) {
          printf("Error deallocating counter for hero_perf_event_store!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_load_external_)
        if (hero_perf_dealloc(hero_perf_event_load_external) != 0) {
          printf("Error deallocating counter for hero_perf_event_load_external!\n");
          return 1;
        }
      #endif

      #if defined(_allocate_perf_event_store_external_)
        if (hero_perf_dealloc(hero_perf_event_store_external) != 0) {
          printf("Error deallocating counter for hero_perf_event_store_external!\n");
          return 1;
        }
      #endif

    #endif

  }

  return 0;
}

/* ===================================================================== */