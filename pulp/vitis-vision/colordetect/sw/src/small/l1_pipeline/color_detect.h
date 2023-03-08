/* =====================================================================
 * Project:      Color detection
 * Title:        color_detect.c
 * Description:  Application-level accelerator functions.
 *
 * $Date:        8.2.2023
 * ===================================================================== */
/*
 * Copyright (C) 2023 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#ifndef __COLOR_DETECT_API_H__
#define __COLOR_DETECT_API_H__

/* System libraries. */

#include <hero-target.h>
#include <arov-target.h>

#include <common_structs/def_struct_soc_perf_eval.h>

/* Struct */

typedef struct {
  uint32_t rows; 
  uint32_t cols; 
  hwpe_dataset_params_struct standard;
} hwpe_color_detect_workload_params;

/* Definitions - Parameters mapping */

void rgb2hsv_cv_pipeline_map_params(
  rgb2hsv_cv_wrapper_struct *wrapper,
  hwpe_l1_ptr_struct *l1_in_img,  
  hwpe_l1_ptr_struct *l1_out_img, 
  hwpe_color_detect_workload_params *params);

void threshold_cv_pipeline_map_params(
  threshold_cv_wrapper_struct *wrapper,
  hwpe_l1_ptr_struct *l1_in_img,  
  hwpe_l1_ptr_struct *l1_out_img, 
  hwpe_color_detect_workload_params *params);

void erode_cv_pipeline_map_params(
  erode_cv_wrapper_struct *wrapper,
  hwpe_l1_ptr_struct *l1_in_img,  
  hwpe_l1_ptr_struct *l1_out_img, 
  hwpe_color_detect_workload_params *params);

void dilate_cv_pipeline_map_params(
  dilate_cv_wrapper_struct *wrapper,
  hwpe_l1_ptr_struct *l1_in_img,  
  hwpe_l1_ptr_struct *l1_out_img, 
  hwpe_color_detect_workload_params *params);

static inline void arov_map_params_color_detect(
  // Accelerator-rich overlay
  AROV_DEVICE_PTR arov, 
  // System parameters
  const int cluster_id, 
  const int accelerator_id,
  // L1 accelerator image buffers
  DEVICE_PTR_CONST l1_in_img,
  DEVICE_PTR_CONST l1_out_img,
  uint32_t dim_img_buffer,
  // Workload paramaters
  uint32_t rows, 
  uint32_t cols
  ) { 

  /* Define parameters */

  hwpe_color_detect_workload_params params;

  params.rows = rows; 
  params.cols = cols; 

  /* L1 buffers */

  // Input image
  hwpe_l1_ptr_struct _l1_in_img;

  _l1_in_img.ptr = (DEVICE_PTR_CONST)l1_in_img;
  _l1_in_img.dim_buffer = dim_img_buffer;

  // Output image
  hwpe_l1_ptr_struct _l1_out_img;

  _l1_out_img.ptr = (DEVICE_PTR_CONST)l1_out_img;
  _l1_out_img.dim_buffer = dim_img_buffer;

  /* Decide which hardware accelerator to program */

  if(cluster_id == 0){
    switch (accelerator_id){

      case 0: rgb2hsv_cv_pipeline_map_params(&(arov->rgb2hsv_cv_0_0), &_l1_in_img, &_l1_out_img, &params); break;
      case 1: rgb2hsv_cv_pipeline_map_params(&(arov->threshold_cv_0_1), &_l1_in_img, &_l1_out_img, &params); break;
      case 2: rgb2hsv_cv_pipeline_map_params(&(arov->erode_cv_0_2), &_l1_in_img, &_l1_out_img, &params); break;
      case 3: rgb2hsv_cv_pipeline_map_params(&(arov->dilate_cv_0_3), &_l1_in_img, &_l1_out_img, &params); break;
      case 4: rgb2hsv_cv_pipeline_map_params(&(arov->dilate_cv_0_4), &_l1_in_img, &_l1_out_img, &params); break;
      case 5: rgb2hsv_cv_pipeline_map_params(&(arov->erode_cv_0_5), &_l1_in_img, &_l1_out_img, &params); break;
      default: printf("Error: No matching case for <arov_map_params_color_detect>\n"); break;
    }
  }
};

#endif