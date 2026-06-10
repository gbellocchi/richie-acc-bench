

/* =====================================================================
 * Project:      HWPE structures
 * Title:        def_struct_hwpe_common.h
 * Description:  Default structures for HWPE wrappers.
 *
 * $Date:        15.7.2022
 * ===================================================================== */
/*
 * Copyright (C) 2022 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#ifndef __HWPE_STRUCTS_COMMON_H__
#define __HWPE_STRUCTS_COMMON_H__

#include <hero-target.h>

/* Definition - Tags */

enum accelerator_tag {
    FAST_CORNER_DETECT
};

/* Definition - Types */

typedef struct hwpe_l1_ptr_struct           hwpe_l1_ptr_struct;
typedef struct hwpe_dma_struct              hwpe_dma_struct;
typedef struct hwpe_addr_gen_struct         hwpe_addr_gen_struct;
typedef struct hwpe_dataset_params_struct   hwpe_dataset_params_struct;
typedef struct hwpe_stream_struct           hwpe_stream_struct;
typedef struct hwpe_sw_states               hwpe_sw_states;
typedef struct hwpe_fsm_struct              hwpe_fsm_struct;

/* Definition - Structs */

struct hwpe_l1_ptr_struct {
    DEVICE_PTR ptr;
    uint32_t dim_buffer;
};

struct hwpe_dma_struct {
    uint32_t cnt_tx;
    uint32_t num_tx;
};

struct hwpe_addr_gen_struct {
    unsigned port_offset;
    unsigned trans_size;
    unsigned line_stride;
    unsigned line_length;
    unsigned feat_stride;
    unsigned feat_length;
    unsigned feat_roll;
    unsigned loop_outer;
    unsigned realign_type;
    unsigned step;
};

struct hwpe_dataset_params_struct {
    uint32_t width;
    uint32_t height;
    uint32_t stim_dim;
    uint32_t stripe_height;
    uint32_t stripe_length;
    uint32_t buffer_dim;
    uint32_t buffer_stride;     // buffer allocation
    uint32_t data_stride;       // data memory access
};

struct hwpe_stream_struct {
    hwpe_l1_ptr_struct tcdm;
    // hwpe_dma_struct dma;
    hwpe_addr_gen_struct addr_gen;
    hwpe_dataset_params_struct params;
};

struct hwpe_fsm_struct {
    uint32_t n_engine_runs;
};

#endif