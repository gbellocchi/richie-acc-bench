/* =====================================================================
 * Project:      Agile Design Methodology for Accelerator-Rich Cluster-based RISC-V SoC
 * Title:        main.c
 * Description:  Coarse-grained experiment with no optimizations.
 *
 * $Date:        8.8.2023
 * ===================================================================== */
/*
 * Copyright (C) 2022 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#include <hero-target.h>

#include <archi/eu/eu_v3.h>
#include <hal/eu/eu_v3.h>

int main(int argc, char *argv[])
{
  const int core_id = hero_rt_core_id();
  const int cluster_id = hero_rt_cluster_id();

  if(core_id>0){

    printf("Sleeping...\n");

    eu_evt_wait();

  } else {

    printf("Profiling of application costs\n");  

    return profiling(cluster_id);
  }

}