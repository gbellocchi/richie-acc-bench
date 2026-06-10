/* =====================================================================
 * Project:      System model
 * Title:        profiling.h
 * Description:  Multi cluster scaling
 *
 * $Date:        21.11.2022
 * ===================================================================== */
/*
 * Copyright (C) 2022 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#ifndef __PROFILING_H__
#define __PROFILING_H__

/* =================== */
/*  Start measurement  */
/* =================== */

inline int start_measurement(const int cluster_id, const int core_id, int hit[2], trns[2], miss[2]){

  if(cluster_id==0 && core_id==0){

    /* Read cache stats */

    hit[0]  = icache_stats_read(cluster_id, 0x40);
    trns[0] = icache_stats_read(cluster_id, 0x44);
    miss[0] = icache_stats_read(cluster_id, 0x48);

    /* Activate performance registers */

    hero_perf_continue_all();

  }

}

/* ================== */
/*  Stop measurement  */
/* ================== */

inline int stop_measurement(const int cluster_id, const int core_id, int hit[2], trns[2], miss[2]){

  if(cluster_id==0 && core_id==0){

    /* Deactivate performance registers */

    hero_perf_pause_all();

    /* Read cache stats */

    hit[1]  = icache_stats_read(cluster_id, 0x40);
    trns[1] = icache_stats_read(cluster_id, 0x44);
    miss[1] = icache_stats_read(cluster_id, 0x48);

  }
  
}

#endif