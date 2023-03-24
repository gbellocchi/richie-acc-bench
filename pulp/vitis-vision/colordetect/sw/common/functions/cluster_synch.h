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

#define CMD_TYPE_CL_START 0 // A stage invokes the subsequent one
#define CMD_TYPE_CL_TERMINATE 1 // A stage invokes the subsequent one
#define CMD_TYPE_DMA_IN_START 2 // Input DMA starts 
#define CMD_TYPE_DMA_IN_TERMINATE 3 // A stage terminates transfering buffered data, hence frees the buffer of the previous stage
#define CMD_TYPE_DMA_OUT_START 4 // Output DMA starts (happens when a cluster is used to mimic bi-directional data transfer)
#define CMD_TYPE_DMA_OUT_TERMINATE 5 // Output DMA terminates (happens when a cluster is used to mimic bi-directional data transfer)

// Macros for retrieving SoC event information
#define get_soc_evt_cid(val)     ((0xFF) & (val >> 16))
#define get_soc_evt_aid(val)     ((0xFF) & (val >> 8))
#define get_soc_evt_type(val)    ((0xFF) & (val >> 0))

#define codify_soc_evt_cid(val)     ((0xFF & val) << 16)
#define codify_soc_evt_aid(val)     ((0xFF & val) << 8)
#define codify_soc_evt_type(val)    ((0xFF & val) << 0)

#define my_soc_evt(cid, aid, type) (codify_soc_evt_cid(cid) + codify_soc_evt_aid(aid) + codify_soc_evt_type(type))

/* ==================== Cluster synchronization based on SoC event FIFO ==================== */

/* Send command from MST (sender) to SLV (receiver) */

static inline void send_cmd_eu_sw_evt(const int mst_cluster_id, const int slv_cluster_id, const int acc_id, const int cmd_type){

  if(mst_cluster_id == hero_rt_cluster_id()){

    const uint32_t soc_evt_msg = my_soc_evt(mst_cluster_id, acc_id, cmd_type);

    __compiler_barrier();

    // eu_evt_trig(eu_evt_trig_cluster_addr(slv_cluster_id, cmd_type), 0);
    pulp_write32(ARCHI_CLUSTER_PERIPHERALS_GLOBAL_ADDR(slv_cluster_id) + ARCHI_EU_OFFSET + EU_SOC_EVENTS_AREA_OFFSET + EU_SOC_EVENTS_CURRENT_EVENT, soc_evt_msg);

    __compiler_barrier();

  } else if(mst_cluster_id != hero_rt_cluster_id()){

    printf("<send_cmd_eu_sw_evt> - ERROR: MST %d is acting as cluster %d\n", hero_rt_cluster_id(), mst_cluster_id);

  } else if(slv_cluster_id == hero_rt_cluster_id()){

    printf("<send_cmd_eu_sw_evt> - ERROR: MST %d is sending a command to itself %d\n", hero_rt_cluster_id(), slv_cluster_id);

  }

}

/* Wait for command sent from MST (sender) to SLV (receiver) */

static inline uint32_t wait_cmd_eu_sw_evt(const int slv_cluster_id, const int mst_cluster_id, const int cmd_type){

  if(slv_cluster_id == hero_rt_cluster_id()){

    __compiler_barrier();

    // eu_evt_maskWaitAndClr(1 << cmd_type);

    // Set event mask
    eu_evt_maskSet(1 << PULP_SOC_EVENTS_EVENT);

    // Wait for synchronization
    eu_evt_wait();

    // Decode associated event message
    const uint32_t soc_evt_msg = eu_soc_events_pop();

    // Clear event unit
    eu_evt_clr(1 << PULP_SOC_EVENTS_EVENT);

    __compiler_barrier();

    return soc_evt_msg;

  } else if(slv_cluster_id != hero_rt_cluster_id()){

    printf("<wait_cmd_eu_sw_evt> - ERROR: SLV %d is acting as cluster %d\n", hero_rt_cluster_id(), slv_cluster_id);

    return -1;

  } else if(mst_cluster_id == hero_rt_cluster_id()){

    printf("<wait_cmd_eu_sw_evt> - ERROR: SLV %d is waiting for a command from itself %d\n", hero_rt_cluster_id(), mst_cluster_id);
    
    return -1;

  }

}

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

static inline cluster_barrier_all_eu_soc_evt(const int cluster_id, const int experiment_id, const int n_slv_cl_terminate_old[n_clusters]){

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
    #endif

                              /* MASTER CLUSTER */

    if(!cluster_id){

      #if defined(DEBUG_SYNCH_EU)
        printf("[MST] [EXP-%d] Waiting for SLV clusters to start experiment together...\n", experiment_id);
      #endif

      // Cluster checklist
      int cluster_array[n_clusters];

      // SoC event message
      uint32_t soc_evt_msg;

      // SoC event information
      uint32_t soc_evt_mst, soc_evt_acc, soc_evt_cmd;

      __compiler_barrier();

      // Init event cluster list
      init_cl_list_eu_soc_evt(cluster_array);

      // Fill previously received termination events in the list
      if(n_slv_cl_terminate_old != 0xFFFFFFFF){
        for(int i=0; i < n_clusters; i++){
          cluster_array[i] = n_slv_cl_terminate_old[i];
        }
      }

      // Wait for synchronization
      while(check_cl_list_eu_soc_evt(cluster_array)){

        soc_evt_msg = wait_cmd_eu_sw_evt(cluster_id, cluster_id + 1, CMD_TYPE_CL_TERMINATE);

        soc_evt_mst = get_soc_evt_cid(soc_evt_msg); // sender
        soc_evt_acc = get_soc_evt_aid(soc_evt_msg); // accelerator
        soc_evt_cmd = get_soc_evt_type(soc_evt_msg); // command

        if((soc_evt_cmd == CMD_TYPE_CL_TERMINATE) && (soc_evt_mst != cluster_id) && (soc_evt_mst < n_clusters)){
          cluster_array[soc_evt_mst] = soc_evt_mst;
        } else {
          printf("<CMD_TYPE_CL_START> - ERROR: CL %d sent to CL %d an unexpected command %d\n", soc_evt_mst, hero_rt_cluster_id(), soc_evt_cmd);
        }

      }

      __compiler_barrier();
        
                              /* SLAVE CLUSTER */
      
    } else {

      #if defined(DEBUG_SYNCH_EU)
        printf("\n\n[SLV] [EXP-%d] Hello MST!\n", experiment_id);
      #endif

      // Send a SW event to MASTER cluster to notify the cluster has terminated its operations...

      const uint32_t mst_cluster_id = 0;
      const uint32_t soc_evt_msg = my_soc_evt(cluster_id, 0, CMD_TYPE_CL_TERMINATE);

      __compiler_barrier();

      // A memory-mapped write to the EU SoC event FIFO will be mapped to a streaming interface
      // by the interconnect demux circuitry. This behavior is entirely implemented in HW and 
      // hence transparent to software. The address location is the same to which event pop operations
      // are directed to (see event unit hal).
      pulp_write32(ARCHI_CLUSTER_PERIPHERALS_GLOBAL_ADDR(mst_cluster_id) + ARCHI_EU_OFFSET + EU_SOC_EVENTS_AREA_OFFSET + EU_SOC_EVENTS_CURRENT_EVENT, soc_evt_msg);

      __compiler_barrier();

      #if defined(DEBUG_SYNCH_EU)
        printf("\n\n[SLV] [EXP-%d] And now back to sleep...\n", experiment_id);
      #endif

      // ...then sleep waiting for new events from MASTER cluster

      __compiler_barrier();

      eu_evt_maskWaitAndClr(1 << CMD_TYPE_CL_START);
      
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

        eu_evt_trig(eu_evt_trig_cluster_addr(cid, CMD_TYPE_CL_START), 0);

      } 

      __compiler_barrier();

      #if defined(DEBUG_SYNCH_EU)
        printf("\n\n[MST] [EXP-%d] Time to work!\n", experiment_id);
      #endif

    }
  }
}

/* ===================================================================== */

#endif