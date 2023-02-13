

/* =====================================================================
 * Project:      HWPE structures
 * Title:        def_struct_color_detect
 * Description:  Definition of C structures for HWPE wrappers.
 *
 * $Date:        15.7.2022
 * ===================================================================== */
/*
 * Copyright (C) 2022 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#ifndef __HWPE_STRUCTS_COLOR_DETECT_H__
#define __HWPE_STRUCTS_COLOR_DETECT_H__

#include <common_structs/def_struct_hwpe_common.h>

/* Definition - Types */

typedef struct hwpe_stream_struct color_detect_stream_struct;
typedef struct color_detect_custom_regs_struct color_detect_custom_regs_struct;
typedef struct color_detect_ctrl_struct color_detect_ctrl_struct;
typedef struct color_detect_wrapper_struct color_detect_wrapper_struct;

/* Definition - Functions */

typedef void (*Program_Color_Detect)(color_detect_wrapper_struct *_wrapper);
typedef void (*Command_Color_Detect)();

/* Definition - Structs */

struct color_detect_custom_regs_struct {
    unsigned rows;
    unsigned cols;
    unsigned low_thresh_0;
    unsigned low_thresh_1;
    unsigned low_thresh_2;
    unsigned low_thresh_3;
    unsigned low_thresh_4;
    unsigned low_thresh_5;
    unsigned low_thresh_6;
    unsigned low_thresh_7;
    unsigned low_thresh_8;
    unsigned high_thresh_0;
    unsigned high_thresh_1;
    unsigned high_thresh_2;
    unsigned high_thresh_3;
    unsigned high_thresh_4;
    unsigned high_thresh_5;
    unsigned high_thresh_6;
    unsigned high_thresh_7;
    unsigned high_thresh_8;
    unsigned process_shape_0;
    unsigned process_shape_1;
    unsigned process_shape_2;
    unsigned process_shape_3;
    unsigned process_shape_4;
    unsigned process_shape_5;
    unsigned process_shape_6;
    unsigned process_shape_7;
    unsigned process_shape_8;
};

struct color_detect_ctrl_struct {
    hwpe_fsm_struct fsm;
    color_detect_custom_regs_struct custom_regs;
};

struct color_detect_wrapper_struct {

    int id;
    int tag;

    color_detect_stream_struct img_in;
    color_detect_stream_struct img_out;

    color_detect_ctrl_struct           ctrl;

    Program_Color_Detect init;
    Program_Color_Detect program;
    Program_Color_Detect update_buffer_addr;
    Command_Color_Detect compute;
    Command_Color_Detect wait;
    Command_Color_Detect clear;
};

#endif