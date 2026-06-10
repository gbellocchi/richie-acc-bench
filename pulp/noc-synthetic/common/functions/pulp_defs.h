/* =====================================================================
 * Project:      System model
 * Title:        pulp_defs.h
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

#ifndef __PULP_DEFS_H__
#define __PULP_DEFS_H__

#include <hal/pulp_io.h>

/* PULP architecture */

#include <archi/chips/bigpulp/memory_map.h>

/* Defintitions */

inline __device static void* arov_l1_base(cluster_id)
{
  extern void __pulp_l1_base;
  return &__pulp_l1_base + cluster_id * ARCHI_CLUSTER_SIZE;
}

inline __device static void* arov_l1_heap(cluster_id)
{
  extern void __l1_heap_start;
  return &__l1_heap_start + cluster_id * ARCHI_CLUSTER_SIZE;
}

inline __device static void* arov_l2_base()
{
  extern void __pulp_l2_base;
  return &__pulp_l2_base;
}

inline __device static void* arov_l2_heap()
{
  extern void __l2_heap_start;
  return &__l2_heap_start;
}

#endif