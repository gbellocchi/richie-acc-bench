

/* =====================================================================
 * Project:      LibAROV
 * Title:        arov-target.h
 * Description:  Definition of APIs for the accelerator-rich system.
 *
 * $Date:        13.7.2022
 * ===================================================================== */
/*
 * Copyright (C) 2022 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#ifndef __AROV_API_H__
#define __AROV_API_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* Hardware accelerators */

#include <common_structs/def_struct_hwpe_fast_corner_detect.h>

#include <libhwpe/hwpe_cl0_lic0.h>

// If LLVM, use our address space support, otherwise fall back to bit-compatible
// data types.
#if defined(__llvm__)
#   define AROV_DEVICE_PTR __device arov_struct*
#else
#   define AROV_DEVICE_PTR arov_struct*
#endif

/* Definition - Types */

typedef struct arov_struct arov_struct;

/* Definition - Structs */

struct arov_struct {

    /* Accelerators - Cluster 0 */

    fast_corner_detect_wrapper_struct fast_corner_detect_0_0;

};

/* AROV functions definition  */

static inline void arov_init(AROV_DEVICE_PTR arov, const int cluster_id, const int accelerator_id){

    /* Decide which hardware accelerator to initialize */

    if(cluster_id == 0){
        switch (accelerator_id){

	        case 0: arov_cl0_lic0_init(&(arov->fast_corner_detect_0_0)); break;
            default: printf("Error: No matching case for <arov_init>\n");
        }
    }

};

static inline int arov_activate(AROV_DEVICE_PTR arov, const int cluster_id, const int accelerator_id){

    /* Decide which hardware accelerator to activate */

    int offload_id;

    if(cluster_id == 0){
        switch (accelerator_id){

	        case 0: offload_id = arov_cl0_lic0_activate(); break;
            default: printf("Error: No matching case for <arov_activate>\n");
        }
    }

    return offload_id;
};

static inline void arov_program(AROV_DEVICE_PTR arov, const int cluster_id, const int accelerator_id){

    /* Decide which hardware accelerator to program */

    if(cluster_id == 0){
        switch (accelerator_id){

	        case 0: arov_cl0_lic0_program(&(arov->fast_corner_detect_0_0)); break;
            default: printf("Error: No matching case for <arov_program>\n");
        }
    }

};

static inline void arov_update_buffer_addr(AROV_DEVICE_PTR arov, const int cluster_id, const int accelerator_id){

    /* Decide which hardware accelerator to update with new buffer addresses */

    if(cluster_id == 0){
        switch (accelerator_id){

	        case 0: arov_cl0_lic0_update_buffer_addr(&(arov->fast_corner_detect_0_0)); break;
            default: printf("Error: No matching case for <arov_update_buffer_addr>\n");
        }
    }

};

static inline void arov_compute(AROV_DEVICE_PTR arov, const int cluster_id, const int accelerator_id){

    /* Decide which hardware accelerator to execute */

    if(cluster_id == 0){
        switch (accelerator_id){

	        case 0: arov_cl0_lic0_compute(); break;
            default: printf("Error: No matching case for <arov_compute>\n");
        }
    }

    // Use the following low-level implementation only if:
	// - HWPE peripheral ports start at 0x1b202000
	// - HWPE peripheral address range equals 0x200
	// Check the cluster interconnect design for more information.

	// uint32_t add = 0x1b202000 + accelerator_id * 0x200;
	// pulp_write32(add, 0);

};

static inline void arov_wait_eu(AROV_DEVICE_PTR arov, const int cluster_id, const int accelerator_id){

    /* Decide which hardware accelerator to wait for */

    if(cluster_id == 0){
        switch (accelerator_id){

	        case 0: arov_cl0_lic0_wait_eu(); break;
            default: printf("Error: No matching case for <arov_wait_eu>\n");
        }
    }

};

static inline void arov_wait_polling(AROV_DEVICE_PTR arov, const int cluster_id, const int accelerator_id){

    /* Decide which hardware accelerator to wait for */

    if(cluster_id == 0){
        switch (accelerator_id){

	        case 0: arov_cl0_lic0_wait_polling(); break;
            default: printf("Error: No matching case for <arov_wait_polling>\n");
        }
    }

};

static inline int arov_is_finished(AROV_DEVICE_PTR arov, const int cluster_id, const int accelerator_id){

    /* Decide which hardware accelerator to check for termination */

    int is_finished;

    if(cluster_id == 0){
        switch (accelerator_id){

	        case 0: is_finished = arov_cl0_lic0_is_finished(); break;
            default: printf("Error: No matching case for <arov_is_finished>\n");
        }
    }

    return is_finished;
};

static inline void arov_free(AROV_DEVICE_PTR arov, const int cluster_id, const int accelerator_id){

    /* Decide which hardware accelerator to disable */

    if(cluster_id == 0){
        switch (accelerator_id){

	        case 0: arov_cl0_lic0_free(); break;
            default: printf("Error: No matching case for <arov_free>\n");
        }
    }

};

#endif