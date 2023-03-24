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

#include <configs.h>

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
  hwpe_l1_ptr_struct *l1_out_img
);

void threshold_cv_pipeline_map_params(
  threshold_cv_wrapper_struct *wrapper,
  hwpe_l1_ptr_struct *l1_in_img,  
  hwpe_l1_ptr_struct *l1_out_img
);

void erode_cv_pipeline_map_params(
  erode_cv_wrapper_struct *wrapper,
  hwpe_l1_ptr_struct *l1_in_img,  
  hwpe_l1_ptr_struct *l1_out_img
);

void dilate_cv_pipeline_map_params(
  dilate_cv_wrapper_struct *wrapper,
  hwpe_l1_ptr_struct *l1_in_img,  
  hwpe_l1_ptr_struct *l1_out_img
);

static inline void arov_map_params_color_detect(
  // Accelerator-rich overlay
  AROV_DEVICE_PTR arov, 
  // System parameters
  const int cluster_id, 
  const int accelerator_id,
  // L1 accelerator image buffers
  DEVICE_PTR_CONST l1_in_img,
  DEVICE_PTR_CONST l1_out_img,
  uint32_t dim_img_buffer) 
{ 

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
      case 0: rgb2hsv_cv_pipeline_map_params(&(arov->rgb2hsv_cv_0_0), &_l1_in_img, &_l1_out_img); break;
      case 1: threshold_cv_pipeline_map_params(&(arov->threshold_cv_0_1), &_l1_in_img, &_l1_out_img); break;
      default: printf("Error: No matching case for <arov_map_params_color_detect>\n"); break;
    }
  }

  if(cluster_id == 1){
    switch (accelerator_id){
      case 2: erode_cv_pipeline_map_params(&(arov->erode_cv_1_2), &_l1_in_img, &_l1_out_img); break;
      case 3: dilate_cv_pipeline_map_params(&(arov->dilate_cv_1_3), &_l1_in_img, &_l1_out_img); break;
      default: printf("Error: No matching case for <arov_map_params_color_detect>\n"); break;
    }
  }

  if(cluster_id == 2){
    switch (accelerator_id){
      case 4: dilate_cv_pipeline_map_params(&(arov->dilate_cv_2_4), &_l1_in_img, &_l1_out_img); break;
      case 5: erode_cv_pipeline_map_params(&(arov->erode_cv_2_5), &_l1_in_img, &_l1_out_img); break;
      default: printf("Error: No matching case for <arov_map_params_color_detect>\n"); break;
    }
  }
};

static inline void arov_swap_buffers_color_detect(
  // Accelerator-rich overlay
  AROV_DEVICE_PTR arov, 
  // System parameters
  const int cluster_id, 
  const int buffer_id,
  // L1 accelerator image buffers
  DEVICE_PTR_CONST l1_img[l1_n_buffers]) 
{ 

  if(buffer_id){ // buffer_id == 1

    // Swap buffer pointers for RGB2HSV_CV
    if(cluster_id == get_acc_cid(RGB2HSV_CV_0)){
      arov->rgb2hsv_cv_0_0.img_in.tcdm.ptr = l1_img[1]; 
      arov->rgb2hsv_cv_0_0.img_out.tcdm.ptr = l1_img[3]; 
      arov_update_buffer_addr(arov, get_acc_cid(RGB2HSV_CV_0), get_acc_aid(RGB2HSV_CV_0));
    }

    // Swap buffer pointers for THRESHOLD_CV
    if(cluster_id == get_acc_cid(THRESHOLD_CV_1)){
      arov->threshold_cv_0_1.img_in.tcdm.ptr = l1_img[1]; 
      arov->threshold_cv_0_1.img_out.tcdm.ptr = l1_img[3]; 
      arov_update_buffer_addr(arov, get_acc_cid(THRESHOLD_CV_1), get_acc_aid(THRESHOLD_CV_1));
    }

    // Swap buffer pointers for ERODE_CV
    if(cluster_id == get_acc_cid(ERODE_CV_2)){
      arov->erode_cv_1_2.img_in.tcdm.ptr = l1_img[1]; 
      arov->erode_cv_1_2.img_out.tcdm.ptr = l1_img[3]; 
      arov_update_buffer_addr(arov, get_acc_cid(ERODE_CV_2), get_acc_aid(ERODE_CV_2));
    }

    // Swap buffer pointers for DILATE_CV
    if(cluster_id == get_acc_cid(DILATE_CV_3)){
      arov->dilate_cv_1_3.img_in.tcdm.ptr = l1_img[1]; 
      arov->dilate_cv_1_3.img_out.tcdm.ptr = l1_img[3]; 
      arov_update_buffer_addr(arov, get_acc_cid(DILATE_CV_3), get_acc_aid(DILATE_CV_3));
    }
    
    // Swap buffer pointers for DILATE_CV
    if(cluster_id == get_acc_cid(DILATE_CV_4)){
      arov->dilate_cv_2_4.img_in.tcdm.ptr = l1_img[1]; 
      arov->dilate_cv_2_4.img_out.tcdm.ptr = l1_img[3]; 
      arov_update_buffer_addr(arov, get_acc_cid(DILATE_CV_4), get_acc_aid(DILATE_CV_4));
    }

    // Swap buffer pointers for DILATE_CV
    if(cluster_id == get_acc_cid(ERODE_CV_5)){
      arov->erode_cv_2_5.img_in.tcdm.ptr = l1_img[1]; 
      arov->erode_cv_2_5.img_out.tcdm.ptr = l1_img[3]; 
      arov_update_buffer_addr(arov, get_acc_cid(ERODE_CV_5), get_acc_aid(ERODE_CV_5));
    }
  
  } else { // buffer_id == 0

    // Swap buffer pointers for RGB2HSV_CV
    if(cluster_id == get_acc_cid(RGB2HSV_CV_0)){
      arov->rgb2hsv_cv_0_0.img_in.tcdm.ptr = l1_img[0]; 
      arov->rgb2hsv_cv_0_0.img_out.tcdm.ptr = l1_img[2]; 
      arov_update_buffer_addr(arov, get_acc_cid(RGB2HSV_CV_0), get_acc_aid(RGB2HSV_CV_0));
    }

    // Swap buffer pointers for THRESHOLD_CV
    if(cluster_id == get_acc_cid(THRESHOLD_CV_1)){
      arov->threshold_cv_0_1.img_in.tcdm.ptr = l1_img[0]; 
      arov->threshold_cv_0_1.img_out.tcdm.ptr = l1_img[2]; 
      arov_update_buffer_addr(arov, get_acc_cid(THRESHOLD_CV_1), get_acc_aid(THRESHOLD_CV_1));
    }

    // Swap buffer pointers for ERODE_CV
    if(cluster_id == get_acc_cid(ERODE_CV_2)){
      arov->erode_cv_1_2.img_in.tcdm.ptr = l1_img[0]; 
      arov->erode_cv_1_2.img_out.tcdm.ptr = l1_img[2]; 
      arov_update_buffer_addr(arov, get_acc_cid(ERODE_CV_2), get_acc_aid(ERODE_CV_2));
    }

    // Swap buffer pointers for DILATE_CV
    if(cluster_id == get_acc_cid(DILATE_CV_3)){
      arov->dilate_cv_1_3.img_in.tcdm.ptr = l1_img[0]; 
      arov->dilate_cv_1_3.img_out.tcdm.ptr = l1_img[2]; 
      arov_update_buffer_addr(arov, get_acc_cid(DILATE_CV_3), get_acc_aid(DILATE_CV_3));
    }
    
    // Swap buffer pointers for DILATE_CV
    if(cluster_id == get_acc_cid(DILATE_CV_4)){
      arov->dilate_cv_2_4.img_in.tcdm.ptr = l1_img[0]; 
      arov->dilate_cv_2_4.img_out.tcdm.ptr = l1_img[2]; 
      arov_update_buffer_addr(arov, get_acc_cid(DILATE_CV_4), get_acc_aid(DILATE_CV_4));
    }

    // Swap buffer pointers for DILATE_CV
    if(cluster_id == get_acc_cid(ERODE_CV_5)){
      arov->erode_cv_2_5.img_in.tcdm.ptr = l1_img[0]; 
      arov->erode_cv_2_5.img_out.tcdm.ptr = l1_img[2]; 
      arov_update_buffer_addr(arov, get_acc_cid(ERODE_CV_5), get_acc_aid(ERODE_CV_5));
    }

  }
};

#endif