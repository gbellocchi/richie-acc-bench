/* =====================================================================
 * Project:      System model
 * Title:        cluster_synch.c
 * Description:  Application-level cluster synchronization functions.
 *
 * $Date:        3.11.2022
 * ===================================================================== */
/*
 * Copyright (C) 2022 University of Modena and Reggio Emilia.
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#include <experiment.h>
#include <cluster_synch.h>
#include <configs.h>

/* ===================================================================== */

/* Check observance of cluster barrier */

static inline int cluster_barrier_check(const int n_evt_expected){

  // Check barrier and throw an exception if MST passes without waiting for SLV

  __compiler_barrier();

  int n_masked_events_received = __builtin_popcount(eu_evt_statusMasked());

                                // Count received (masked) events, hence count 
                                // number of set bits ('1) in the event status register 
                                // and AND them with the mask that has been previously setup

  __compiler_barrier();

  // If there's something wrong an error message is thrown

  if(n_masked_events_received != n_evt_expected) {
    const char *error = "ERROR: MST has passed barrier without waiting for SLV";
    printf("%s\n", error);
    printf("Number of clusters at barrier:            %d out of %d\n", n_masked_events_received, n_evt_expected);
  }

  // Additional information about synchronization status

  #if defined(DEBUG_SYNCH_EU)
    printf("Number of received masked events:       %d\n", n_masked_events_received);
    printf("[MST] [EXP-%d] EU status:           0x%08x\n", experiment_id, eu_evt_status());
    printf("[MST] [EXP-%d] EU status (masked):  0x%08x\n", experiment_id, eu_evt_statusMasked());
  #endif

  return n_masked_events_received;

}

/* ===================================================================== */

/* Cluster synchronization based on SW events */

/* Cluster barrier */

int cluster_barrier_eu_sw_evt(const int cluster_id, const int experiment_id){

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

      __compiler_barrier();

      // Set event unit mask

      for(int i=1; i<n_clusters; i++){
        eu_evt_maskSet(1<<i);
      }

      // Wait for synchronization

      for(int i=1; i<n_clusters; i++){
        eu_evt_wait();
      }

      // Clear event unit

      for(int i=1; i<n_clusters; i++){
        eu_evt_clr(1<<i);
      }

      __compiler_barrier();
        
                              /* SLAVE CLUSTER */
      
    } else {

      #if defined(DEBUG_SYNCH_EU)
        printf("\n\n[SLV] [EXP-%d] Hello MST!\n", experiment_id);
      #endif

      // Send a SW event to MASTER cluster to notify the cluster is ready to go...

      __compiler_barrier();

      eu_evt_trig(eu_evt_trig_cluster_addr(0, hero_rt_cluster_id()), 0);

      __compiler_barrier();

      #if defined(DEBUG_SYNCH_EU)
        printf("\n\n[SLV] [EXP-%d] And now back to sleep...\n", experiment_id);
      #endif

      // ...then sleep waiting for new events from MASTER cluster

      __compiler_barrier();

      eu_evt_maskSet(1 << 0);
      eu_evt_wait();
      eu_evt_clr(1 << 0);
      
      __compiler_barrier();

      #if defined(DEBUG_SYNCH_EU)
        printf("\n\n[SLV] [EXP-%d] Time to work!\n", experiment_id);
      #endif

    }
  }

  return n_evt_received;
}

/* Restart slave clusters */

void cluster_slv_restart_eu_sw_evt(const int cluster_id, const int experiment_id){

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

        eu_evt_trig(eu_evt_trig_cluster_addr(cid, 0), 0);

      } 

      __compiler_barrier();

      #if defined(DEBUG_SYNCH_EU)
        printf("\n\n[MST] [EXP-%d] Time to work!\n", experiment_id);
      #endif

    }
  }
}

/* ===================================================================== */

/* Cluster synchronization based on SoC event FIFO */

// inlined in cluster_synch.h 

/* ===================================================================== */