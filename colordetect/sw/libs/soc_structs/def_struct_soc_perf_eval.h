

/* =====================================================================
 * Project:      SoC structures
 * Title:        def_struct_soc_perf_eval.h
 * Description:   Definition of default structures for performance profiling.
 *
 * $Date:        1.12.2022
 * ===================================================================== */
/*
 * Copyright (C) 2022 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#ifndef __SOC_STRUCTS_PERF_EVAL_H__
#define __SOC_STRUCTS_PERF_EVAL_H__

/* Libraries */

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/* System. */

#include <hero-target.h>

// Structs - Performance evaluation

typedef struct host_timer_struct        host_timer_struct;
typedef struct pulp_clk_struct          pulp_clk_struct;

struct host_timer_struct {
    struct timespec t0;
    struct timespec t1;
    float t_meas;
};

struct pulp_clk_struct {
    uint32_t cnt_0;
    uint32_t cnt_1;
    uint32_t cnt_2;
};

// Structs - PULP

typedef struct pulp_dma_struct        pulp_dma_struct;

struct pulp_dma_struct {
    hero_dma_job_t job;
    pulp_clk_struct clk;
};

#endif