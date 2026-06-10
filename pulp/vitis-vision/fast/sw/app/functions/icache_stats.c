/* =====================================================================
 * Project:      System model
 * Title:        icache_stats.c
 * Description:  Application-level icache functions.
 *
 * $Date:        28.10.2022
 * ===================================================================== */
/*
 * Copyright (C) 2022 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#include "experiment.h"
#include "configs.h"

/* ======================== */
/*  I-CACHE INITIALIZATION  */
/* ======================== */

int icache_stats_init(const int cluster_id) {
  // Enable All Icache Banks api 
  *(volatile int*) (ARCHI_ICACHE_CTRL_ADDR) = 0xFFFFFFFF;
  pulp_write32(ARCHI_CLUSTER_PERIPHERALS_GLOBAL_ADDR(cluster_id) + ARCHI_ICACHE_CTRL_OFFSET, 0xFFFFFFFF);

  // Reset the statistic counters in each icache banks
  *(volatile int*) (ARCHI_ICACHE_CTRL_ADDR+0x10) = 0xFFFFFFFF;

  // Enable all the statistic counters in all icache banks availabe
  *(volatile int*) (ARCHI_ICACHE_CTRL_ADDR+0x14) = 0xFFFFFFFF;
}

/* =============== */
/*  I-CACHE RESET  */
/* =============== */

int icache_stats_reset(const int cluster_id) {
  // Reset the statistic counters in each icache banks
  *(volatile int*) (ARCHI_ICACHE_CTRL_ADDR+0x10) = 0xFFFFFFFF;
}

/* =================== */
/*  I-CACHE READ STAT  */
/* =================== */

int icache_stats_read(const int cluster_id, const int reg_offset)
{
  return *(volatile int*) (ARCHI_ICACHE_CTRL_ADDR+reg_offset);
}

/* =================== */
/*  I-CACHE FLUSH ALL  */
/* =================== */

void icache_flush_all ( unsigned int base )
{
  *(volatile int*) (ARCHI_ICACHE_CTRL_ADDR+0x4) = 1;
}