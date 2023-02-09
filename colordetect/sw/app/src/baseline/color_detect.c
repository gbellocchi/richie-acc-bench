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

/* ===================================================================== */

/*
 *
 *     FPGA overlay
 *
 */

/* Parameters mapping */

void color_detect_wrapper_map_params(
  color_detect_wrapper_struct *wrapper, 
  hwpe_l1_ptr_struct *l1_color_detect_buffer, 
  hwpe_color_detect_workload_params *params) 
{
  /* Streamer */

  // Input image
  
  wrapper->img_in.addr_gen.trans_size                   = 128 * 128;
  wrapper->img_in.addr_gen.line_stride                  = 0; 
  wrapper->img_in.addr_gen.line_length                  = 128 * 128;
  wrapper->img_in.addr_gen.feat_stride                  = 0; 
  wrapper->img_in.addr_gen.feat_length                  = 1; 
  wrapper->img_in.addr_gen.feat_roll                    = 0; 
  wrapper->img_in.addr_gen.loop_outer                   = 0; 
  wrapper->img_in.addr_gen.realign_type                 = 0; 
  wrapper->img_in.addr_gen.step                         = 4;

  // Output image

  wrapper->img_out.addr_gen.trans_size                  = 128 * 128; 
  wrapper->img_out.addr_gen.line_stride                 = 0; 
  wrapper->img_out.addr_gen.line_length                 = 128 * 128; 
  wrapper->img_out.addr_gen.feat_stride                 = 0; 
  wrapper->img_out.addr_gen.feat_length                 = 1; 
  wrapper->img_out.addr_gen.feat_roll                   = 0; 
  wrapper->img_out.addr_gen.loop_outer                  = 0; 
  wrapper->img_out.addr_gen.realign_type                = 0; 
  wrapper->img_out.addr_gen.step                        = 4;  

  // Assign buffer pointers
  wrapper->img_in.tcdm.ptr = (DEVICE_PTR)l1_color_detect_buffer->ptr; 
  wrapper->img_out.tcdm.ptr = (DEVICE_PTR)l1_color_detect_buffer->ptr + l1_color_detect_buffer->dim_buffer;

  /* Controller */

  // FSM
  wrapper->ctrl.fsm.n_engine_runs                         = 1;

  // Custom registers
  wrapper->ctrl.custom_regs.rows                          = 128;
  wrapper->ctrl.custom_regs.cols                          = 128; 

  wrapper->ctrl.custom_regs.low_thresh_0                  = 22;
  wrapper->ctrl.custom_regs.low_thresh_1                  = 150;
  wrapper->ctrl.custom_regs.low_thresh_2                  = 60;
  wrapper->ctrl.custom_regs.low_thresh_3                  = 38;
  wrapper->ctrl.custom_regs.low_thresh_4                  = 150;
  wrapper->ctrl.custom_regs.low_thresh_5                  = 60;
  wrapper->ctrl.custom_regs.low_thresh_6                  = 160;
  wrapper->ctrl.custom_regs.low_thresh_7                  = 150;
  wrapper->ctrl.custom_regs.low_thresh_8                  = 60;

  wrapper->ctrl.custom_regs.high_thresh_0                 = 38;
  wrapper->ctrl.custom_regs.high_thresh_1                 = 255;
  wrapper->ctrl.custom_regs.high_thresh_2                 = 255;
  wrapper->ctrl.custom_regs.high_thresh_3                 = 75;
  wrapper->ctrl.custom_regs.high_thresh_4                 = 255;
  wrapper->ctrl.custom_regs.high_thresh_5                 = 255;
  wrapper->ctrl.custom_regs.high_thresh_6                 = 179;
  wrapper->ctrl.custom_regs.high_thresh_7                 = 255;
  wrapper->ctrl.custom_regs.high_thresh_8                 = 255;

  wrapper->ctrl.custom_regs.process_shape_0               = 1;
  wrapper->ctrl.custom_regs.process_shape_1               = 1;
  wrapper->ctrl.custom_regs.process_shape_2               = 1;
  wrapper->ctrl.custom_regs.process_shape_3               = 1;
  wrapper->ctrl.custom_regs.process_shape_4               = 1;
  wrapper->ctrl.custom_regs.process_shape_5               = 1;
  wrapper->ctrl.custom_regs.process_shape_6               = 1;
  wrapper->ctrl.custom_regs.process_shape_7               = 1;
  wrapper->ctrl.custom_regs.process_shape_8               = 1;
}