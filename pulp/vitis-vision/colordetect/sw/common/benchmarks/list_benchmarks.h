/* =====================================================================
 * Project:      System model
 * Title:        list_benchmarks.h
 * Description:  Color detect DSE configuration.
 *
 * $Date:        16.2.2023
 * ===================================================================== */
/*
 * Copyright (C) 2023 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#ifndef __LIST_BENCHMARKS_H__
#define __LIST_BENCHMARKS_H__

#define L1_BASELINE                 0
#define L1_PIPELINE                 1
#define L2_BASELINE                 2
#define L2_PIPELINE_SCL_NO_TILE     3
#define L2_PIPELINE_MCL_NO_TILE     4
#define L2_PIPELINE_SCL_CONST_TILE  5
#define L2_PIPELINE_MCL_CONST_TILE  6
#define L2_PIPELINE_SCL_VAR_TILE    7
#define L2_PIPELINE_MCL_VAR_TILE    8

#endif