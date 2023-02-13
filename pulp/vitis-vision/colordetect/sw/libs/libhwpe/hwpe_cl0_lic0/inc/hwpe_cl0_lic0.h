

/* =====================================================================
 * Project:      LibHWPE
 * Title:        hwpe_cl0_lic0
 * Description:  Definition of APIs for color_detect.
 *
 * $Date:        13.7.2022
 * ===================================================================== */
/*
 * Copyright (C) 2022 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#ifndef __HWPE_CL0_LIC0_API_H__
#define __HWPE_CL0_LIC0_API_H__

#include <stdint.h>

/* Include HERO library. */
#include <hero-target.h>

#include <common_structs/def_struct_hwpe_common.h>
#include <common_structs/def_struct_hwpe_color_detect.h>

/* Initialization */

void arov_cl0_lic0_init(color_detect_wrapper_struct *wrapper);

/* Activation */

int arov_cl0_lic0_activate();

/* Programming */

void arov_cl0_lic0_program(color_detect_wrapper_struct *wrapper);

/* Data memory interaction */

void arov_cl0_lic0_update_buffer_addr(color_detect_wrapper_struct *wrapper);

/* Processing */

void arov_cl0_lic0_compute();

/* Synchronization via Event Unit */

void arov_cl0_lic0_wait_eu();

/* Synchronization via Active Core Polling */

void arov_cl0_lic0_wait_polling();

/* Termination */

int arov_cl0_lic0_is_finished();

/* Deactivation */

void arov_cl0_lic0_free();

#endif