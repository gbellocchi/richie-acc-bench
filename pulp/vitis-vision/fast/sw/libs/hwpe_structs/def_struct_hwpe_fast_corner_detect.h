

/* =====================================================================
 * Project:      HWPE structures
 * Title:        def_struct_fast_corner_detect
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

#ifndef __HWPE_STRUCTS_FAST_CORNER_DETECT_H__
#define __HWPE_STRUCTS_FAST_CORNER_DETECT_H__

#include <common_structs/def_struct_hwpe_common.h>

/* Definition - Types */

typedef struct hwpe_stream_struct fast_corner_detect_stream_struct;
typedef struct fast_corner_detect_custom_regs_struct fast_corner_detect_custom_regs_struct;
typedef struct fast_corner_detect_ctrl_struct fast_corner_detect_ctrl_struct;
typedef struct fast_corner_detect_wrapper_struct fast_corner_detect_wrapper_struct;

/* Definition - Functions */

typedef void (*Program_Fast_Corner_Detect)(fast_corner_detect_wrapper_struct *_wrapper);
typedef void (*Command_Fast_Corner_Detect)();

/* Definition - Structs */

struct fast_corner_detect_custom_regs_struct {
    unsigned threshold;
    unsigned rows;
    unsigned cols;
};

struct fast_corner_detect_ctrl_struct {
    hwpe_fsm_struct fsm;
    fast_corner_detect_custom_regs_struct custom_regs;
};

struct fast_corner_detect_wrapper_struct {

    int id;
    int tag;

    fast_corner_detect_stream_struct img_in;
    fast_corner_detect_stream_struct img_out;

    fast_corner_detect_ctrl_struct           ctrl;

    Program_Fast_Corner_Detect init;
    Program_Fast_Corner_Detect program;
    Program_Fast_Corner_Detect update_buffer_addr;
    Command_Fast_Corner_Detect compute;
    Command_Fast_Corner_Detect wait;
    Command_Fast_Corner_Detect clear;
};

#endif