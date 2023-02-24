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

#include <color_detect.h>
#include <configs.h>

/* =====================================================================
 * Parameters mapping --> RGB2HSV_CV
 * ===================================================================== */

void rgb2hsv_cv_pipeline_map_params(
  rgb2hsv_cv_wrapper_struct *wrapper,
  hwpe_l1_ptr_struct *l1_in_img,  
  hwpe_l1_ptr_struct *l1_out_img, 
  hwpe_color_detect_workload_params *params) 
{
  /* Streamer */

  // Input image
  
  wrapper->img_in.addr_gen.trans_size                   = params->rows * params->cols;
  wrapper->img_in.addr_gen.line_stride                  = 0; 
  wrapper->img_in.addr_gen.line_length                  = (params->rows * params->cols)/l1_bank_stride;
  wrapper->img_in.addr_gen.feat_stride                  = 0; 
  wrapper->img_in.addr_gen.feat_length                  = l1_n_buffer_reps * l1_bank_stride; 
  wrapper->img_in.addr_gen.feat_roll                    = 0; 
  wrapper->img_in.addr_gen.loop_outer                   = 0; 
  wrapper->img_in.addr_gen.realign_type                 = 0; 
  wrapper->img_in.addr_gen.step                         = 4 * l1_bank_stride;

  // Output image

  wrapper->img_out.addr_gen.trans_size                  = params->rows * params->cols;
  wrapper->img_out.addr_gen.line_stride                 = 0; 
  wrapper->img_out.addr_gen.line_length                 = (params->rows * params->cols)/l1_bank_stride; 
  wrapper->img_out.addr_gen.feat_stride                 = 0; 
  wrapper->img_out.addr_gen.feat_length                 = l1_n_buffer_reps * l1_bank_stride; 
  wrapper->img_out.addr_gen.feat_roll                   = 0; 
  wrapper->img_out.addr_gen.loop_outer                  = 0; 
  wrapper->img_out.addr_gen.realign_type                = 0; 
  wrapper->img_out.addr_gen.step                        = 4 * l1_bank_stride;  

  // Assign buffer pointers
  wrapper->img_in.tcdm.ptr = (DEVICE_PTR)l1_in_img->ptr; 
  wrapper->img_out.tcdm.ptr = (DEVICE_PTR)l1_out_img->ptr;

  /* Controller */

  // Engine
  wrapper->ctrl.engine.packet_size_img_in                 = params->cols;

  // FSM
  wrapper->ctrl.fsm.n_engine_runs                         = 1;

  // Custom registers
  wrapper->ctrl.custom_regs.rows                          = params->rows;
  wrapper->ctrl.custom_regs.cols                          = params->cols; 
}

/* =====================================================================
 * Parameters mapping --> THRESHOLD_CV
 * ===================================================================== */

void threshold_cv_pipeline_map_params(
  threshold_cv_wrapper_struct *wrapper,
  hwpe_l1_ptr_struct *l1_in_img,  
  hwpe_l1_ptr_struct *l1_out_img, 
  hwpe_color_detect_workload_params *params) 
{
  /* Streamer */

  // Input image
  
  wrapper->img_in.addr_gen.trans_size                   = params->rows * params->cols;
  wrapper->img_in.addr_gen.line_stride                  = 0; 
  wrapper->img_in.addr_gen.line_length                  = (params->rows * params->cols)/l1_bank_stride;
  wrapper->img_in.addr_gen.feat_stride                  = 0; 
  wrapper->img_in.addr_gen.feat_length                  = l1_n_buffer_reps * l1_bank_stride; 
  wrapper->img_in.addr_gen.feat_roll                    = 0; 
  wrapper->img_in.addr_gen.loop_outer                   = 0; 
  wrapper->img_in.addr_gen.realign_type                 = 0; 
  wrapper->img_in.addr_gen.step                         = 4 * l1_bank_stride;

  // Output image

  wrapper->img_out.addr_gen.trans_size                  = params->rows * params->cols;
  wrapper->img_out.addr_gen.line_stride                 = 0; 
  wrapper->img_out.addr_gen.line_length                 = (params->rows * params->cols)/l1_bank_stride; 
  wrapper->img_out.addr_gen.feat_stride                 = 0; 
  wrapper->img_out.addr_gen.feat_length                 = l1_n_buffer_reps * l1_bank_stride; 
  wrapper->img_out.addr_gen.feat_roll                   = 0; 
  wrapper->img_out.addr_gen.loop_outer                  = 0; 
  wrapper->img_out.addr_gen.realign_type                = 0; 
  wrapper->img_out.addr_gen.step                        = 4 * l1_bank_stride;  

  // Assign buffer pointers
  wrapper->img_in.tcdm.ptr = (DEVICE_PTR)l1_in_img->ptr; 
  wrapper->img_out.tcdm.ptr = (DEVICE_PTR)l1_out_img->ptr;

  /* Controller */

  // Engine
  wrapper->ctrl.engine.packet_size_img_in                 = params->cols;

  // FSM
  wrapper->ctrl.fsm.n_engine_runs                         = 1;

  // Custom registers
  wrapper->ctrl.custom_regs.rows                          = params->rows;
  wrapper->ctrl.custom_regs.cols                          = params->cols; 
}

/* =====================================================================
 * Parameters mapping --> ERODE_CV
 * ===================================================================== */

void erode_cv_pipeline_map_params(
  erode_cv_wrapper_struct *wrapper,
  hwpe_l1_ptr_struct *l1_in_img,  
  hwpe_l1_ptr_struct *l1_out_img, 
  hwpe_color_detect_workload_params *params) 
{
  /* Streamer */

  // Input image
  
  wrapper->img_in.addr_gen.trans_size                   = params->rows * params->cols;
  wrapper->img_in.addr_gen.line_stride                  = 0; 
  wrapper->img_in.addr_gen.line_length                  = (params->rows * params->cols)/l1_bank_stride;
  wrapper->img_in.addr_gen.feat_stride                  = 0; 
  wrapper->img_in.addr_gen.feat_length                  = l1_n_buffer_reps * l1_bank_stride; 
  wrapper->img_in.addr_gen.feat_roll                    = 0; 
  wrapper->img_in.addr_gen.loop_outer                   = 0; 
  wrapper->img_in.addr_gen.realign_type                 = 0; 
  wrapper->img_in.addr_gen.step                         = 4 * l1_bank_stride;

  // Output image

  wrapper->img_out.addr_gen.trans_size                  = params->rows * params->cols;
  wrapper->img_out.addr_gen.line_stride                 = 0; 
  wrapper->img_out.addr_gen.line_length                 = (params->rows * params->cols)/l1_bank_stride;
  wrapper->img_out.addr_gen.feat_stride                 = 0; 
  wrapper->img_out.addr_gen.feat_length                 = l1_n_buffer_reps * l1_bank_stride; 
  wrapper->img_out.addr_gen.feat_roll                   = 0; 
  wrapper->img_out.addr_gen.loop_outer                  = 0; 
  wrapper->img_out.addr_gen.realign_type                = 0; 
  wrapper->img_out.addr_gen.step                        = 4 * l1_bank_stride;  

  // Assign buffer pointers
  wrapper->img_in.tcdm.ptr = (DEVICE_PTR)l1_in_img->ptr; 
  wrapper->img_out.tcdm.ptr = (DEVICE_PTR)l1_out_img->ptr;

  /* Controller */

  // Engine
  wrapper->ctrl.engine.packet_size_img_in                 = params->cols;

  // FSM
  wrapper->ctrl.fsm.n_engine_runs                         = 1;

  // Custom registers
  wrapper->ctrl.custom_regs.rows                          = params->rows;
  wrapper->ctrl.custom_regs.cols                          = params->cols; 
}

/* =====================================================================
 * Parameters mapping --> DILATE_CV
 * ===================================================================== */

void dilate_cv_pipeline_map_params(
  dilate_cv_wrapper_struct *wrapper,
  hwpe_l1_ptr_struct *l1_in_img,  
  hwpe_l1_ptr_struct *l1_out_img, 
  hwpe_color_detect_workload_params *params) 
{
  /* Streamer */

  // Input image
  
  wrapper->img_in.addr_gen.trans_size                   = params->rows * params->cols;
  wrapper->img_in.addr_gen.line_stride                  = 0; 
  wrapper->img_in.addr_gen.line_length                  = (params->rows * params->cols)/l1_bank_stride;
  wrapper->img_in.addr_gen.feat_stride                  = 0; 
  wrapper->img_in.addr_gen.feat_length                  = l1_n_buffer_reps * l1_bank_stride; 
  wrapper->img_in.addr_gen.feat_roll                    = 0; 
  wrapper->img_in.addr_gen.loop_outer                   = 0; 
  wrapper->img_in.addr_gen.realign_type                 = 0; 
  wrapper->img_in.addr_gen.step                         = 4 * l1_bank_stride;

  // Output image

  wrapper->img_out.addr_gen.trans_size                  = params->rows * params->cols;
  wrapper->img_out.addr_gen.line_stride                 = 0; 
  wrapper->img_out.addr_gen.line_length                 = (params->rows * params->cols)/l1_bank_stride;
  wrapper->img_out.addr_gen.feat_stride                 = 0; 
  wrapper->img_out.addr_gen.feat_length                 = l1_n_buffer_reps * l1_bank_stride; 
  wrapper->img_out.addr_gen.feat_roll                   = 0; 
  wrapper->img_out.addr_gen.loop_outer                  = 0; 
  wrapper->img_out.addr_gen.realign_type                = 0; 
  wrapper->img_out.addr_gen.step                        = 4 * l1_bank_stride;  

  // Assign buffer pointers
  wrapper->img_in.tcdm.ptr = (DEVICE_PTR)l1_in_img->ptr; 
  wrapper->img_out.tcdm.ptr = (DEVICE_PTR)l1_out_img->ptr;

  /* Controller */

  // Engine
  wrapper->ctrl.engine.packet_size_img_in                 = params->cols;

  // FSM
  wrapper->ctrl.fsm.n_engine_runs                         = 1;

  // Custom registers
  wrapper->ctrl.custom_regs.rows                          = params->rows;
  wrapper->ctrl.custom_regs.cols                          = params->cols; 
}