

/* =====================================================================
 * Project:      LibHWPE
 * Title:        hwpe_cl0_lic0
 * Description:  Software APIs for color_detect.
 *
 * $Date:        11.7.2022
 * ===================================================================== */
/*
 * Copyright (C) 2022 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

/* HWPE */

// Library

#include <libhwpe/hwpe_cl0_lic0.h>

// Hardware abstraction layer

#include <libhwpe/archi/hwpe_cl0_lic0.h>
#include <libhwpe/hal/hwpe_cl0_lic0.h>

/* PULP peripherals */

#include <archi/eu/eu_v3.h>
#include <hal/eu/eu_v3.h>

/* ==================================================================================== */

/* Initialization */

void arov_cl0_lic0_init(color_detect_wrapper_struct *wrapper) {

    // Set wrapper index
    wrapper->id   = 0;

    // Set wrapper type
    wrapper->tag   = COLOR_DETECT;
};

/* Activation */

int arov_cl0_lic0_activate() {

    // Activate wrapper
    int offload_id;
    hwpe_cg_enable();
    while((offload_id = hwpe_acquire_job()) < 0);
    return offload_id;
};

/* Programming */

void arov_cl0_lic0_program(color_detect_wrapper_struct *wrapper) {

    // Extract streams

    // color_detect_stream_struct stream_img_in   = wrapper->img_in;
    // color_detect_stream_struct stream_img_out  = wrapper->img_out;

    // Extract controller

    // color_detect_ctrl_struct ctrl          = wrapper->ctrl;

    /*
     * STANDARD REGISTERS
     */

    // Iteration length

    // Number of engine computations before an event is generated

    hwpe_len_iter_set_img_out(wrapper->ctrl.fsm.n_engine_runs - 1);

    // Address generator programming

    hwpe_addr_gen_img_in(
        wrapper->img_in.addr_gen.trans_size,
        wrapper->img_in.addr_gen.line_stride,
        wrapper->img_in.addr_gen.line_length,
        wrapper->img_in.addr_gen.feat_stride,
        wrapper->img_in.addr_gen.feat_length,
        wrapper->img_in.addr_gen.feat_roll,
        wrapper->img_in.addr_gen.loop_outer,
        wrapper->img_in.addr_gen.realign_type,
        wrapper->img_in.addr_gen.step
    );

    hwpe_addr_gen_img_out(
        wrapper->img_out.addr_gen.trans_size,
        wrapper->img_out.addr_gen.line_stride,
        wrapper->img_out.addr_gen.line_length,
        wrapper->img_out.addr_gen.feat_stride,
        wrapper->img_out.addr_gen.feat_length,
        wrapper->img_out.addr_gen.feat_roll,
        wrapper->img_out.addr_gen.loop_outer,
        wrapper->img_out.addr_gen.realign_type,
        wrapper->img_out.addr_gen.step
    );

    /*
     * CUSTOM REGISTERS
     */

    // Set user custom registers

    hwpe_rows_set( wrapper->ctrl.custom_regs.rows );
    hwpe_cols_set( wrapper->ctrl.custom_regs.cols );
    hwpe_low_thresh_0_set( wrapper->ctrl.custom_regs.low_thresh_0 );
    hwpe_low_thresh_1_set( wrapper->ctrl.custom_regs.low_thresh_1 );
    hwpe_low_thresh_2_set( wrapper->ctrl.custom_regs.low_thresh_2 );
    hwpe_low_thresh_3_set( wrapper->ctrl.custom_regs.low_thresh_3 );
    hwpe_low_thresh_4_set( wrapper->ctrl.custom_regs.low_thresh_4 );
    hwpe_low_thresh_5_set( wrapper->ctrl.custom_regs.low_thresh_5 );
    hwpe_low_thresh_6_set( wrapper->ctrl.custom_regs.low_thresh_6 );
    hwpe_low_thresh_7_set( wrapper->ctrl.custom_regs.low_thresh_7 );
    hwpe_low_thresh_8_set( wrapper->ctrl.custom_regs.low_thresh_8 );
    hwpe_high_thresh_0_set( wrapper->ctrl.custom_regs.high_thresh_0 );
    hwpe_high_thresh_1_set( wrapper->ctrl.custom_regs.high_thresh_1 );
    hwpe_high_thresh_2_set( wrapper->ctrl.custom_regs.high_thresh_2 );
    hwpe_high_thresh_3_set( wrapper->ctrl.custom_regs.high_thresh_3 );
    hwpe_high_thresh_4_set( wrapper->ctrl.custom_regs.high_thresh_4 );
    hwpe_high_thresh_5_set( wrapper->ctrl.custom_regs.high_thresh_5 );
    hwpe_high_thresh_6_set( wrapper->ctrl.custom_regs.high_thresh_6 );
    hwpe_high_thresh_7_set( wrapper->ctrl.custom_regs.high_thresh_7 );
    hwpe_high_thresh_8_set( wrapper->ctrl.custom_regs.high_thresh_8 );
    hwpe_process_shape_0_set( wrapper->ctrl.custom_regs.process_shape_0 );
    hwpe_process_shape_1_set( wrapper->ctrl.custom_regs.process_shape_1 );
    hwpe_process_shape_2_set( wrapper->ctrl.custom_regs.process_shape_2 );
    hwpe_process_shape_3_set( wrapper->ctrl.custom_regs.process_shape_3 );
    hwpe_process_shape_4_set( wrapper->ctrl.custom_regs.process_shape_4 );
    hwpe_process_shape_5_set( wrapper->ctrl.custom_regs.process_shape_5 );
    hwpe_process_shape_6_set( wrapper->ctrl.custom_regs.process_shape_6 );
    hwpe_process_shape_7_set( wrapper->ctrl.custom_regs.process_shape_7 );
    hwpe_process_shape_8_set( wrapper->ctrl.custom_regs.process_shape_8 );

    /*
     * TCDM REGISTERS
     */

    // Program controller with L1 buffer address

    hwpe_img_in_addr_set((int32_t)wrapper->img_in.tcdm.ptr);

    hwpe_img_out_addr_set((int32_t)wrapper->img_out.tcdm.ptr);
};

/* Data memory interaction */

void arov_cl0_lic0_update_buffer_addr(color_detect_wrapper_struct *wrapper) {

    // Extract streams

    // color_detect_stream_struct stream_img_in   = wrapper->img_in;
    // color_detect_stream_struct stream_img_out  = wrapper->img_out;

    /*
     * TCDM REGISTERS
     */

    // Program controller with L1 buffer address

    hwpe_img_in_addr_set((int32_t)wrapper->img_in.tcdm.ptr);

    hwpe_img_out_addr_set((int32_t)wrapper->img_out.tcdm.ptr);
};

/* Processing */

void arov_cl0_lic0_compute() {
    hwpe_trigger_job();
};

/* Synchronization via Event Unit */

void arov_cl0_lic0_wait_eu() {

    // TODO: Accelerators share the same event line inside a cluster,
    // hence the processor needs to proactively check which accelerator
    // has terminated after an event is received.

    // Cores go to sleep and EU is programmed to wait for
    // hardware event coming from the accelerator region.

    __asm__ __volatile__ ("" : : : "memory");
    eu_evt_maskWaitAndClr(1 << ARCHI_CL_EVT_ACC0);
    __asm__ __volatile__ ("" : : : "memory");
};

/* Synchronization via Active Core Polling */

void arov_cl0_lic0_wait_polling() {

    // The core starts polling on the accelerator to verify
    // whether It has terminated its operations.

    while(!hwpe_get_finished()){
      asm volatile ("nop");
    }
};

/* Termination */

int arov_cl0_lic0_is_finished() {

    // Check if accelerator has terminated its operation
    int is_finished = hwpe_get_finished();
    return is_finished;
};

/* Deactivation */

void arov_cl0_lic0_free() {
    hwpe_soft_clear();
    hwpe_cg_disable();
};