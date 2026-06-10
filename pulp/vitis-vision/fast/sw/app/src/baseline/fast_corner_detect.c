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

#include <fast_corner_detect.h>
#include <configs.h>

/* ===================================================================== */

/*
 *
 *     FPGA overlay
 *
 */

/* Parameters mapping */

void fast_corner_detect_wrapper_map_params(
  fast_corner_detect_wrapper_struct *wrapper, 
  hwpe_l1_ptr_struct *l1_fast_corner_detect_buffer, 
  hwpe_fast_corner_detect_workload_params *params) 
{
  /* Streamer */

  // Input image
  
  wrapper->img_in.addr_gen.trans_size                  = 128 * 128;
  wrapper->img_in.addr_gen.line_stride                 = 0; 
  wrapper->img_in.addr_gen.line_length                 = 128 * 128;
  wrapper->img_in.addr_gen.feat_stride                 = 0; 
  wrapper->img_in.addr_gen.feat_length                 = 1; 
  wrapper->img_in.addr_gen.feat_roll                   = 0; 
  wrapper->img_in.addr_gen.loop_outer                  = 0; 
  wrapper->img_in.addr_gen.realign_type                = 0; 
  wrapper->img_in.addr_gen.step                        = 4;

  // Output image

  wrapper->img_out.addr_gen.trans_size                 = 128 * 128; 
  wrapper->img_out.addr_gen.line_stride                = 0; 
  wrapper->img_out.addr_gen.line_length                = 128 * 128; 
  wrapper->img_out.addr_gen.feat_stride                = 0; 
  wrapper->img_out.addr_gen.feat_length                = 1; 
  wrapper->img_out.addr_gen.feat_roll                  = 0; 
  wrapper->img_out.addr_gen.loop_outer                 = 0; 
  wrapper->img_out.addr_gen.realign_type               = 0; 
  wrapper->img_out.addr_gen.step                       = 4;  

  // Assign buffer pointers
  wrapper->img_in.tcdm.ptr = (DEVICE_PTR)l1_fast_corner_detect_buffer->ptr; 
  wrapper->img_out.tcdm.ptr = (DEVICE_PTR)l1_fast_corner_detect_buffer->ptr + l1_fast_corner_detect_buffer->dim_buffer;

  /* Controller */

  // FSM
  wrapper->ctrl.fsm.n_engine_runs                         = 1;

  // Custom registers
  wrapper->ctrl.custom_regs.threshold                     = 20;
  wrapper->ctrl.custom_regs.rows                          = 128;
  wrapper->ctrl.custom_regs.cols                          = 128; 
}