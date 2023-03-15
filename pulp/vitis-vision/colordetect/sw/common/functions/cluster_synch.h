/* =====================================================================
 * Project:      System model
 * Title:        cluster_synch.h
 * Description:  Multi cluster scaling
 *
 * $Date:        29.12.2022
 * ===================================================================== */
/*
 * Copyright (C) 2022 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#ifndef __CLUSTER_SYNCH_H__
#define __CLUSTER_SYNCH_H__

#include <configs.h>

// Define message types used in benchmarks with SW events (max 8)

#define CMD_TYPE_STAGE_INVOCATION 0 // A stage invokes the subsequent one
#define CMD_TYPE_DMA_IN_START 1 // Output DMA starts (happens when a cluster is used to mimic bi-directional data transfer)
#define CMD_TYPE_DMA_IN_TERMINATE 2 // A stage terminates transfering buffered data, hence frees the buffer of the previous stage
#define CMD_TYPE_DMA_OUT_START 3 // Output DMA starts (happens when a cluster is used to mimic bi-directional data transfer)
#define CMD_TYPE_DMA_OUT_TERMINATE 4 // Output DMA terminates (happens when a cluster is used to mimic bi-directional data transfer)

/* ===================================================================== */

/* Cluster synchronization based on SoC event FIFO */

/* Cluster checklist functions */

static inline int init_cl_list_eu_soc_evt(int *cluster_array){
  for(int i=0; i<n_clusters; i++){
    cluster_array[i] = 0;
  }
  return 0;
}

static inline int check_cl_list_eu_soc_evt(int *cluster_array){
  for(int i=1; i<n_clusters; i++){
    if (cluster_array[i]!=i) return 1;
  }
  return 0;
}

/* Cluster barrier */

static inline cluster_barrier_all_eu_soc_evt(const int cluster_id, const int experiment_id){

  // Set a barrier at the end of a profiling job. Master cluster sleeps waiting
  // for slave clusters to terminate. Once all slave clusters have terminated their operations,
  // they'll start sleeping till the master cluster won't wake up them for a new profiling job.

  int n_evt_received = 0;

  if(n_clusters>1){

    #if defined(DEBUG_SYNCH_EU) 
      // Synchronize clusters at the end of an experiment run
      if(!cluster_id)
        printf("[MST] [EXP-%d] Terminated run #%d!\n", hero_rt_cluster_id(), experiment_id);
      else
        printf("[SLV] [EXP-%d] Terminated run #%d!\n", hero_rt_cluster_id(), experiment_id);
      }
    #endif

                              /* MASTER CLUSTER */

    if(!cluster_id){

      #if defined(DEBUG_SYNCH_EU)
        printf("[MST] [EXP-%d] Waiting for SLV clusters to start experiment together...\n", experiment_id);
      #endif

      // Cluster checklist

      int cluster_array[n_clusters];

      __compiler_barrier();

      // Set event unit mask (SoC fifo event)

      eu_evt_maskSet(1 << PULP_SOC_EVENTS_EVENT);

      // Init event cluster list
      init_cl_list_eu_soc_evt(cluster_array);

      // Wait for synchronization
      while(check_cl_list_eu_soc_evt(cluster_array)){
        eu_evt_wait();
        int evt_cid = eu_soc_events_pop() & 0xFF; // Data width = Event width = 8
        if(evt_cid < n_clusters) 
          cluster_array[evt_cid] = evt_cid;
      }

      // Clear event unit

      eu_evt_clr(1 << PULP_SOC_EVENTS_EVENT);

      __compiler_barrier();
        
                              /* SLAVE CLUSTER */
      
    } else {

      #if defined(DEBUG_SYNCH_EU)
        printf("\n\n[SLV] [EXP-%d] Hello MST!\n", experiment_id);
      #endif

      // Send a SW event to MASTER cluster to notify the cluster is ready to go...

      __compiler_barrier();

      // A memory-mapped write to the EU SoC event FIFO will be mapped to a streaming interface
      // by the interconnect demux circuitry. This behavior is entirely implemented in HW and 
      // hence transparent to software. The address location is the same to which event pop operations
      // are directed to (see event unit hal).
      pulp_write32(ARCHI_CLUSTER_PERIPHERALS_GLOBAL_ADDR(0) + ARCHI_EU_OFFSET + EU_SOC_EVENTS_AREA_OFFSET + EU_SOC_EVENTS_CURRENT_EVENT, cluster_id);

      __compiler_barrier();

      #if defined(DEBUG_SYNCH_EU)
        printf("\n\n[SLV] [EXP-%d] And now back to sleep...\n", experiment_id);
      #endif

      // ...then sleep waiting for new events from MASTER cluster

      __compiler_barrier();

      eu_evt_maskWaitAndClr(1 << CMD_TYPE_STAGE_INVOCATION);
      
      __compiler_barrier();

      #if defined(DEBUG_SYNCH_EU)
        printf("\n\n[SLV] [EXP-%d] Time to work!\n", experiment_id);
      #endif

    }
  }

  return n_evt_received;
}

/* Restart slave clusters */

static inline void cluster_slv_all_restart_eu_soc_evt(const int cluster_id, const int experiment_id){

  // Wake up sleeping slave clusters to start a new profiling job.

  if(n_clusters>1){

                              /* MASTER CLUSTER */

    if(!cluster_id){

      // Now send events to all SLAVE clusters and wake them up for executing the experiment

      __compiler_barrier();

      for(int cid=1; cid<n_clusters; cid++){

        #if defined(DEBUG_SYNCH_EU)
          printf("[MST] [EXP-%d] Waking up SLV cluster %d\n", experiment_id, cid);
        #endif

        eu_evt_trig(eu_evt_trig_cluster_addr(cid, CMD_TYPE_STAGE_INVOCATION), 0);

      } 

      __compiler_barrier();

      #if defined(DEBUG_SYNCH_EU)
        printf("\n\n[MST] [EXP-%d] Time to work!\n", experiment_id);
      #endif

    }
  }
}

/* ===================================================================== */

/* Send command from MST (sender) to SLV (receiver) */

static inline void send_cmd_eu_sw_evt(const int mst_cluster_id, const int slv_cluster_id, const int cmd_type){

  if(mst_cluster_id == hero_rt_cluster_id()){

    __compiler_barrier();

    eu_evt_trig(eu_evt_trig_cluster_addr(slv_cluster_id, cmd_type), 0);

    __compiler_barrier();

  } else {

    printf("<send_cmd_eu_sw_evt> - ERROR: MST %d is acting as cluster %d\n", hero_rt_cluster_id(), mst_cluster_id);

  }

}

/* Wait for command sent from MST (sender) to SLV (receiver) */

static inline void wait_cmd_eu_sw_evt(const int slv_cluster_id, const int mst_cluster_id, const int cmd_type){

  if(slv_cluster_id == hero_rt_cluster_id()){

    __compiler_barrier();

   eu_evt_maskWaitAndClr(1 << cmd_type);

    __compiler_barrier();

  } else {

    printf("<wait_cmd_eu_sw_evt> - ERROR: SLV %d is acting as cluster %d\n", hero_rt_cluster_id(), slv_cluster_id);

  }

}

#endif