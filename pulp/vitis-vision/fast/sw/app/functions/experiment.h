/* =====================================================================
 * Project:      System model
 * Title:        experiment.h
 * Description:  Multi cluster scaling
 *
 * $Date:        17.7.2022
 * ===================================================================== */
/*
 * Copyright (C) 2022 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#ifndef __EXPERIMENT_H__
#define __EXPERIMENT_H__

/* Libraries */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>

/* System. */

#include <common_structs/def_struct_soc_perf_eval.h>
#include <hero-target.h>
#include <arov-target.h>

#include <fast_corner_detect.h>
#include <profiling.h>

/* PULP runtime */

#include "pulp_defs.h"

// Event Unit
#include <archi/eu/eu_v3.h>
#include <hal/eu/eu_v3.h>

#endif