/* =====================================================================
 * Project:      FAST corner detection
 * Title:        fast_corner_detect.c
 * Description:  Application-level accelerator functions.
 *
 * $Date:        2.2.2023
 * ===================================================================== */
/*
 * Copyright (C) 2023 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#ifndef __FAST_CORNER_DETECT_API_H__
#define __FAST_CORNER_DETECT_API_H__

/* System libraries. */

#include <hero-target.h>
#include <arov-target.h>

#include <common_structs/def_struct_soc_perf_eval.h>

/* Struct */

typedef struct {
  uint32_t n_total_reqs; 
  uint32_t t_ck_reqs; 
  uint32_t t_ck_idle;
  uint32_t max_buffer_dim;
  uint32_t n_reps;
  uint32_t n_tcdm_banks;
  hwpe_dataset_params_struct standard;
} hwpe_fast_corner_detect_workload_params;

/* Definitions - Parameters mapping */

void fast_corner_detect_wrapper_map_params(
  fast_corner_detect_wrapper_struct *wrapper, 
  hwpe_l1_ptr_struct *l1_fast_corner_detect_buffer, 
  hwpe_fast_corner_detect_workload_params *params);

static inline void arov_map_params_fast_corner_detect(
  // Accelerator-rich overlay
  AROV_DEVICE_PTR arov, 
  // System parameters
  const int cluster_id, 
  const int accelerator_id,
  // L1 accelerator buffer
  DEVICE_PTR_CONST l1_arov_acc_buffer,
  uint32_t dim_fast_corner_detect_buffers,
  // Workload paramaters
  uint32_t width, 
  uint32_t height, 
  uint32_t stripe_height,
  uint32_t n_total_reqs, 
  uint32_t t_ck_reqs, 
  uint32_t t_ck_idle, 
  uint32_t max_buffer_dim,
  uint32_t n_reps,
  uint32_t n_tcdm_banks) { 

  /* Define parameters */

  hwpe_fast_corner_detect_workload_params params;

  params.n_total_reqs     = n_total_reqs; 
  params.t_ck_reqs        = t_ck_reqs; 
  params.t_ck_idle        = t_ck_idle;
  params.max_buffer_dim   = max_buffer_dim;
  params.n_reps           = n_reps;
  params.n_tcdm_banks     = n_tcdm_banks;

  /* L1 buffer pointer */

  hwpe_l1_ptr_struct l1_fast_corner_detect_buffer;

  l1_fast_corner_detect_buffer.ptr = (DEVICE_PTR_CONST)l1_arov_acc_buffer + dim_fast_corner_detect_buffers * accelerator_id;
  l1_fast_corner_detect_buffer.dim_buffer = dim_fast_corner_detect_buffers;

  /* Decide which hardware accelerator to program */

  if(cluster_id == 0){
    switch (accelerator_id){

      case 0: fast_corner_detect_wrapper_map_params(&(arov->fast_corner_detect_0_0), &l1_fast_corner_detect_buffer, &params); break;
      default: printf("Error: No matching case for <arov_map_params_fast_corner_detect>\n"); break;
    }
  }
};

#endif