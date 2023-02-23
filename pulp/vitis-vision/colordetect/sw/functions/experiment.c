/* =====================================================================
 * Project:      System model
 * Title:        perf_refs.c
 * Description:  Application-level performance counter functions.
 *
 * $Date:        28.10.2022
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

#include <stimuli.h>

/* =====================================================================
 * Project:      Color detect
 * Title:        run_baseline
 * 
 * Description:  Baseline application for color detect accelerator-rich 
 *               pipeline with no pipelining.
 * ===================================================================== */

void run_baseline(const int cluster_id, const int core_id) {

  #if defined(PRINT_LOG)
    printf(" # - [Params] n_img:                  %d \n",           (int32_t)n_img);
    printf(" # - [Params] img_tile:               %d \n",           (int32_t)img_tile);
    printf(" # - [Params] l1_buffer_dim:          %d \n",           (int32_t)l1_buffer_dim);
    printf(" # - [Params] L1_n_reps:              %d \n",           (int32_t)l1_n_buffer_reps);
    printf(" # - [Params] l2_buffer_dim:          %d \n",           (int32_t)l2_buffer_dim);
    printf(" # - [Params] l2_n_tiles:             %d \n",           (int32_t)l2_n_tiles);
    printf(" # - [Params] dma_payload_dim:        %d \n",           (int32_t)dma_payload_dim);
    printf(" # - [Params] dma_n_tx:               %d \n",           (int32_t)dma_n_tx);
  #endif

  /* ===================================================================== */

  /* Runtime IDs */

  int run_id; // Pipeline execution run

  /* Define cache stats */

  int hit[2], trns[2], miss[2];
  int reg_hit, reg_trns, reg_miss;

  /* Cycle counters */

  pulp_clk_struct t_experiment_sys_pov;

  /* ===================================================================== */

  /* Initialize L2 memory */

  // Declare L2 cluster base address  
  DEVICE_PTR_CONST l2_cl_base = (l2_cl_port_id==0) ? \
                                  arov_l2_heap() : \
                                    // bank 0 holds also the program, so buffers are allocated starting from the heap
                                  arov_l2_base() + l2_cl_port_id * l2_n_bytes_per_port;
                                    // same as the HW of the SoC bus 

  // Declare L2 cluster buffer address  
  DEVICE_PTR_CONST l2_cl_addr             = l2_cl_base + (cluster_id - (l2_cl_port_id - l2_cl_port_id_offset) * l2_n_cl_per_port) * dma_payload_dim;

  #ifndef l2_double_buffering
  // Declare L2 image buffers
  DEVICE_PTR_CONST l2_img_a               = l2_cl_addr; // raw image (input image)
  DEVICE_PTR_CONST l2_img_b               = l2_img_a + l2_buffer_dim; // rgb2hsv
  DEVICE_PTR_CONST l2_img_c               = l2_img_b + l2_buffer_dim; // threshold
  DEVICE_PTR_CONST l2_img_d               = l2_img_c + l2_buffer_dim; // erode 1
  DEVICE_PTR_CONST l2_img_e               = l2_img_d + l2_buffer_dim; // dilate 1
  DEVICE_PTR_CONST l2_img_f               = l2_img_e + l2_buffer_dim; // dilate 2
  DEVICE_PTR_CONST l2_img_g               = l2_img_f + l2_buffer_dim; // erode 2 (output image)
  # else
  // Declare L2 image buffers
  DEVICE_PTR_CONST l2_img_a_0             = l2_cl_addr; // raw image (input image)
  DEVICE_PTR_CONST l2_img_b_0             = l2_img_a_0 + l2_buffer_dim; // rgb2hsv
  DEVICE_PTR_CONST l2_img_c_0             = l2_img_b_0 + l2_buffer_dim; // threshold
  DEVICE_PTR_CONST l2_img_d_0             = l2_img_c_0 + l2_buffer_dim; // erode 1
  DEVICE_PTR_CONST l2_img_e_0             = l2_img_d_0 + l2_buffer_dim; // dilate 1
  DEVICE_PTR_CONST l2_img_f_0             = l2_img_e_0 + l2_buffer_dim; // dilate 2
  DEVICE_PTR_CONST l2_img_g_0             = l2_img_f_0 + l2_buffer_dim; // erode 2 (output image)

  DEVICE_PTR_CONST l2_img_a_1             = l2_img_g_0 + l2_buffer_dim; // raw image (input image)
  DEVICE_PTR_CONST l2_img_b_1             = l2_img_a_1 + l2_buffer_dim; // rgb2hsv
  DEVICE_PTR_CONST l2_img_c_1             = l2_img_b_1 + l2_buffer_dim; // threshold
  DEVICE_PTR_CONST l2_img_d_1             = l2_img_c_1 + l2_buffer_dim; // erode 1
  DEVICE_PTR_CONST l2_img_e_1             = l2_img_d_1 + l2_buffer_dim; // dilate 1
  DEVICE_PTR_CONST l2_img_f_1             = l2_img_e_1 + l2_buffer_dim; // dilate 2
  DEVICE_PTR_CONST l2_img_g_1             = l2_img_f_1 + l2_buffer_dim; // erode 2 (output image)
  #endif

  #if defined(PRINT_LOG)
    printf(" # - [Params] ID_L2_port:             %d \n",           (int32_t)l2_cl_port_id);
    printf(" # - [Params] Base_address_L2:        %p \n",           (int32_t)l2_cl_base);
    printf(" # - [Params] Buffer_address_L2:      %p \n",           (int32_t)l2_cl_addr);
  #endif

  #ifdef INPUT_INIT
    // Initialize input buffer with input image
    for(int i=0; i<l2_buffer_dim; i++) pulp_write32(l2_img_a+i*sizeof(int32_t), in_img_small[i]);
  #endif

  /* ===================================================================== */

  /* Initialize L1 memory */

  #if defined(_pulp_rt_)
    __device uint32_t* l1_arov_buffer   = (__device uint32_t*)hero_l1malloc(sizeof(uint32_t) * l1_arov_buffer_dim);
  #elif defined(_pulp_bare_)
    DEVICE_PTR_CONST l1_arov_buffer    = arov_l1_heap(cluster_id) + n_l1_ports * sizeof(uint32_t);
  #endif

  #ifndef l2_double_buffering
  // Declare L1 image buffers
  DEVICE_PTR_CONST l1_img_a               = l1_arov_buffer; // raw image (input image)
  DEVICE_PTR_CONST l1_img_b               = l1_img_a + l1_buffer_dim; // rgb2hsv
  DEVICE_PTR_CONST l1_img_c               = l1_img_b + l1_buffer_dim; // threshold
  DEVICE_PTR_CONST l1_img_d               = l1_img_c + l1_buffer_dim; // erode 1
  DEVICE_PTR_CONST l1_img_e               = l1_img_d + l1_buffer_dim; // dilate 1
  DEVICE_PTR_CONST l1_img_f               = l1_img_e + l1_buffer_dim; // dilate 2
  DEVICE_PTR_CONST l1_img_g               = l1_img_f + l1_buffer_dim; // erode 2 (output image)
  # else
  // Declare L1 image buffers
  DEVICE_PTR_CONST l1_img_a_0             = l1_arov_buffer; // raw image (input image)
  DEVICE_PTR_CONST l1_img_b_0             = l1_img_a_0 + l1_buffer_dim; // rgb2hsv
  DEVICE_PTR_CONST l1_img_c_0             = l1_img_b_0 + l1_buffer_dim; // threshold
  DEVICE_PTR_CONST l1_img_d_0             = l1_img_c_0 + l1_buffer_dim; // erode 1
  DEVICE_PTR_CONST l1_img_e_0             = l1_img_d_0 + l1_buffer_dim; // dilate 1
  DEVICE_PTR_CONST l1_img_f_0             = l1_img_e_0 + l1_buffer_dim; // dilate 2
  DEVICE_PTR_CONST l1_img_g_0             = l1_img_f_0 + l1_buffer_dim; // erode 2 (output image)

  DEVICE_PTR_CONST l1_img_a_1             = l1_img_g_0 + l1_buffer_dim; // raw image (input image)
  DEVICE_PTR_CONST l1_img_b_1             = l1_img_a_1 + l1_buffer_dim; // rgb2hsv
  DEVICE_PTR_CONST l1_img_c_1             = l1_img_b_1 + l1_buffer_dim; // threshold
  DEVICE_PTR_CONST l1_img_d_1             = l1_img_c_1 + l1_buffer_dim; // erode 1
  DEVICE_PTR_CONST l1_img_e_1             = l1_img_d_1 + l1_buffer_dim; // dilate 1
  DEVICE_PTR_CONST l1_img_f_1             = l1_img_e_1 + l1_buffer_dim; // dilate 2
  DEVICE_PTR_CONST l1_img_g_1             = l1_img_f_1 + l1_buffer_dim; // erode 2 (output image)
  #endif

  // [TO-DO] ...Then also intermediate result buffers are to be declared 

  // ...

  /* ===================================================================== */

  /* System */

  pulp_dma_struct dma_in[n_hwpe_active], dma_out[n_hwpe_active], dma_wait[n_hwpe_active];

  /* Accelerators */

  // Custom registers
  unsigned rows, cols;

  int offload_id[n_hwpe_active];

  /* Allocate accelerator-rich overlay */

  arov_struct arov;

  /* ===================================================================== */

  /* Cluster steady state condition */
  
  cluster_barrier_eu_soc_evt(cluster_id, 0);
  if(!cluster_id) cluster_slv_restart_eu_soc_evt(cluster_id, 0);

  /* ===================================================================== */

  /* Flush cache (cold-cache condition) */

  icache_flush_all();

  /* Launch profiling jobs */

  for(int job_id=0; job_id<exp_len_job_queue; job_id++){

    /* ===================================================================== */

    /* Initialize counters */

    // Reset and start PULP counter
    hero_reset_clk_counter();
    hero_start_clk_counter();

    if(cluster_id==0 && core_id==0){

      // Reset cache stats
      icache_stats_reset();

      reg_hit   = 0;
      reg_trns  = 0;
      reg_miss  = 0;

      // Reset performance counters
      hero_perf_reset_all();

    }

    /* ===================================================================== */

    /* MEASUREMENT - START */

    // cache statistics and performance counters
    start_measurement(cluster_id, core_id, hit, trns, miss);

    // Cluster synchronization barrier
    cluster_barrier_eu_soc_evt(cluster_id, experiment_id);

    // Cluster timer
    if(!cluster_id) t_experiment_sys_pov.cnt_0 = hero_get_clk_counter();

    // Activate clusters simultaneously
    // NB: Activation must be after the starting of time measurements
    if(!cluster_id) cluster_slv_restart_eu_soc_evt(cluster_id, experiment_id);

    /* ===================================================================== */

    /* ============================= */
    /*  Profiling - Color detection  */
    /* ============================= */

    /* Transfer input image from L2 */

    for (int i = 0; i < n_dma_tx; i++) {
      dma_in[RGB2HSV_CV].job = hero_memcpy_host2dev_async(l1_img_a, l2_img_a, dma_payload_dim * sizeof(uint32_t));
    }

    /* Wait DMA to terminate data transfer */

    hero_dma_wait(dma_in[RGB2HSV_CV].job);

    /* Pipeline */

    // RGB2HSV_CV

    arov_compute(&arov, cluster_id, RGB2HSV_CV);

    while(!arov_is_finished(&arov, cluster_id, RGB2HSV_CV)){
      arov_wait_eu(&arov, cluster_id, RGB2HSV_CV);
    }

    // THRESHOLD_CV
    
    arov_compute(&arov, cluster_id, THRESHOLD_CV);
    
    while(!arov_is_finished(&arov, cluster_id, THRESHOLD_CV)){
      arov_wait_eu(&arov, cluster_id, THRESHOLD_CV);
    }

    // ERODE_CV

    arov_compute(&arov, cluster_id, ERODE_CV);
    
    while(!arov_is_finished(&arov, cluster_id, ERODE_CV)){
      arov_wait_eu(&arov, cluster_id, ERODE_CV);
    }

    // DILATE_CV

    arov_compute(&arov, cluster_id, DILATE_CV);
    
    while(!arov_is_finished(&arov, cluster_id, DILATE_CV)){
      arov_wait_eu(&arov, cluster_id, DILATE_CV);
    }

    // Update buffer pointers for DILATE_CV
    arov.dilate_cv_0_3.img_in.tcdm.ptr = l1_img_e; 
    arov.dilate_cv_0_3.img_out.tcdm.ptr = l1_img_f; 
    arov_update_buffer_addr(&arov, cluster_id, DILATE_CV);

    // Update buffer pointers for ERODE_CV
    arov.erode_cv_0_2.img_in.tcdm.ptr = l1_img_f; 
    arov.erode_cv_0_2.img_out.tcdm.ptr = l1_img_g; 
    arov_update_buffer_addr(&arov, cluster_id, ERODE_CV);

    // DILATE_CV

    arov_compute(&arov, cluster_id, DILATE_CV);
    
    while(!arov_is_finished(&arov, cluster_id, DILATE_CV)){
      arov_wait_eu(&arov, cluster_id, DILATE_CV);
    }

    // ERODE_CV

    arov_compute(&arov, cluster_id, ERODE_CV);
    
    while(!arov_is_finished(&arov, cluster_id, ERODE_CV)){
      arov_wait_eu(&arov, cluster_id, ERODE_CV);
    }

    /* Transfer input image from L2 */

    for (int i = 0; i < n_dma_tx; i++) {
      dma_out[ERODE_CV].job = hero_memcpy_dev2host_async(l2_img_g, l1_img_g, dma_payload_dim * sizeof(uint32_t));
    }

    /* Wait DMA to terminate data transfer */

    hero_dma_wait(dma_in[ERODE_CV].job);

    /* ===================================================================== */

    /* MEASUREMENT - END */

    // Cluster synchronization barrier
    cluster_barrier_eu_soc_evt(cluster_id, experiment_id);

    // Cluster timer
    if(!cluster_id) t_experiment_sys_pov.cnt_1 = hero_get_clk_counter();

    // Cache statistics and performance counters
    stop_measurement(cluster_id, core_id, hit, trns, miss);

    // Update measured cache stats
    reg_hit  += hit[1] - hit[0];
    reg_trns += trns[1] - trns[0];
    reg_miss += miss[1] - miss[0];

    /* ===================================================================== */

    /* Print statistics */

    print_job_stats(
      // System
      cluster_id, 
      core_id, 
      // Experiment
      0, 
      job_id, 
      "color_detect_1cl_small", 
      // Cache
      &reg_hit,
      &reg_trns, 
      &reg_miss, 
      // Clock counters
      0xFFFFFFFF,
      &t_experiment_sys_pov
    );

    /* Restart clusters */

    if(!cluster_id) cluster_slv_restart_eu_soc_evt(cluster_id, 0);

  } // job_id

  /* Cleaning accelerators */

  for(int acc_id=0; acc_id<n_acc_active; acc_id++){
    arov_free(&arov, cluster_id, acc_id);
  }

  /* Cleaning L1 and L2 */
  
  #if defined(_pulp_rt_)
    hero_l3free(l2_img_a);
    
    // TO-DO: Add all buffers

    hero_l1free(l1_arov_buffer);
  #endif
}



/* =====================================================================
 * Project:      Color detect
 * Title:        run_baseline
 * 
 * Description:  Baseline application for color detect accelerator-rich 
 *               pipeline with no pipelining.
 * ===================================================================== */

// void run_baseline(const int cluster_id, const int core_id) {

//   /* ===================================================================== */

//   int experiment_id = 0;

//   /* Define cache stats */

//   int hit[2], trns[2], miss[2];
//   int reg_hit, reg_trns, reg_miss;

//   /* Cycle counters */

//   pulp_clk_struct t_experiment_sys_pov;

//   /* ===================================================================== */

//   /* Initialize L2 memory */

//   // Variables
//   unsigned l2_n_cl_per_port               = n_clusters/n_l2_ports_virt;
//   unsigned l2_n_bytes_per_port            = l2_size_B/n_l2_ports_phy;
//   unsigned l2_n_words_per_port            = l2_n_bytes_per_port/sizeof(uint32_t);
  
//   // Calculate port ID (Optional: L2 cluster port offset)
//   unsigned l2_cl_port_id                  = cluster_id/l2_n_cl_per_port + l2_cl_port_id_offset;

//   // Declare L2 cluster base address  
//   DEVICE_PTR_CONST l2_cl_base = (l2_cl_port_id==0) ? \
//                                   arov_l2_heap() : \
//                                     // bank 0 holds also the program, so buffers are allocated starting from the heap
//                                   arov_l2_base() + l2_cl_port_id * l2_n_bytes_per_port;
//                                     // same as the HW of the SoC bus 

//   // Declare L2 cluster buffer address  
//   DEVICE_PTR_CONST l2_cl_addr             = l2_cl_base + (cluster_id - (l2_cl_port_id - l2_cl_port_id_offset) * l2_n_cl_per_port) * dma_payload_dim;

//   // Declare L2 image buffers
//   DEVICE_PTR_CONST l2_img_a               = l2_cl_addr; // raw image (input image)
//   DEVICE_PTR_CONST l2_img_b               = l2_img_a + l2_buffer_dim; // rgb2hsv
//   DEVICE_PTR_CONST l2_img_c               = l2_img_b + l2_buffer_dim; // threshold
//   DEVICE_PTR_CONST l2_img_d               = l2_img_c + l2_buffer_dim; // erode 1
//   DEVICE_PTR_CONST l2_img_e               = l2_img_d + l2_buffer_dim; // dilate 1
//   DEVICE_PTR_CONST l2_img_f               = l2_img_e + l2_buffer_dim; // dilate 2
//   DEVICE_PTR_CONST l2_img_g               = l2_img_f + l2_buffer_dim; // erode 2 (output image)

//   #if defined(PRINT_LOG)
//     printf(" # - [Params] ID_L2_port:             %d \n",           (int32_t)l2_cl_port_id);
//     printf(" # - [Params] Base_address_L2:        %p \n",           (int32_t)l2_cl_base);
//     printf(" # - [Params] Buffer_address_L2:      %p \n",           (int32_t)l2_cl_addr);
//   #endif

//   #ifdef INPUT_INIT
//     // Initialize input buffer with input image
//     for(int i=0; i<l2_buffer_dim; i++) pulp_write32(l2_img_a+i*sizeof(int32_t), in_img_small[i]);
//   #endif

//   /* ===================================================================== */

//   /* Initialize L1 memory */

//   #if defined(_pulp_rt_)
//     __device uint32_t* l1_arov_buffer   = (__device uint32_t*)hero_l1malloc(sizeof(uint32_t) * l1_arov_buffer_dim);
//   #elif defined(_pulp_bare_)
//     DEVICE_PTR_CONST l1_arov_buffer    = arov_l1_heap(cluster_id);
//   #endif

//   // Declare L1 image buffers
//   DEVICE_PTR_CONST l1_img_a               = l1_arov_buffer; // raw image (input image)
//   DEVICE_PTR_CONST l1_img_b               = l1_img_a + l1_buffer_dim; // rgb2hsv
//   DEVICE_PTR_CONST l1_img_c               = l1_img_b + l1_buffer_dim; // threshold
//   DEVICE_PTR_CONST l1_img_d               = l1_img_c + l1_buffer_dim; // erode 1
//   DEVICE_PTR_CONST l1_img_e               = l1_img_d + l1_buffer_dim; // dilate 1
//   DEVICE_PTR_CONST l1_img_f               = l1_img_e + l1_buffer_dim; // dilate 2
//   DEVICE_PTR_CONST l1_img_g               = l1_img_f + l1_buffer_dim; // erode 2 (output image)

//   /* ===================================================================== */

//   /* System */

//   pulp_dma_struct dma_in[n_acc_active], dma_out[n_acc_active], dma_wait[n_acc_active];

//   /* Accelerators */

//   // Custom registers
//   unsigned rows, cols;

//   int offload_id[n_acc_active];

//   /* Allocate accelerator-rich overlay */

//   arov_struct arov;

//   /* ===================================================================== */

//   /* Cluster steady state condition */
  
//   cluster_barrier_eu_soc_evt(cluster_id, experiment_id);
//   if(!cluster_id) cluster_slv_restart_eu_soc_evt(cluster_id, experiment_id);

//   /* ===================================================================== */

//   /* Runtime experiment parameters */

//   // - Number of image rows
//   rows = img_rows; 

//   // - Number of image columns
//   cols = img_cols;  

//   /* Program accelerators */

//   for(int acc_id=0; acc_id<n_acc_active; acc_id++){

//     // Initialization
//     arov_init(&arov, cluster_id, acc_id);
    
//     // -- rgb2hsv - Map parameters
//     if (cluster_id==0 && acc_id==0) arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img_a, l1_img_b, l1_buffer_dim, rows, cols);
//     // -- threshold - Map parameters
//     if (cluster_id==0 && acc_id==1) arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img_b, l1_img_c, l1_buffer_dim, rows, cols);
//     // -- erode - Map parameters
//     if (cluster_id==0 && acc_id==2) arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img_c, l1_img_d, l1_buffer_dim, rows, cols);
//     // -- dilate - Map parameters
//     if (cluster_id==0 && acc_id==3) arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img_d, l1_img_e, l1_buffer_dim, rows, cols);

//     // Activation and programming
//     offload_id[acc_id] = arov_activate(&arov, cluster_id, acc_id);
//     arov_program(&arov, cluster_id, acc_id);

//   }

//   /* Flush cache (cold-cache condition) */

//   icache_flush_all();

//   /* Launch profiling jobs */

//   for(int job_id=0; job_id<exp_len_job_queue; job_id++){

//     /* ===================================================================== */

//     /* Reset and start PULP counter */

//     hero_reset_clk_counter();
//     hero_start_clk_counter();

//     /* Reset cache stats */

//     if(cluster_id==0 && core_id==0){

//       icache_stats_reset();

//       reg_hit   = 0;
//       reg_trns  = 0;
//       reg_miss  = 0;

//       /* Reset performance counters */

//       hero_perf_reset_all();

//     }

//     /* ===================================================================== */

//     /* MEASUREMENT - START */

//     // cache statistics and performance counters
//     start_measurement(cluster_id, core_id, hit, trns, miss);

//     // Cluster synchronization barrier
//     cluster_barrier_eu_soc_evt(cluster_id, experiment_id);

//     // Cluster timer
//     if(!cluster_id) t_experiment_sys_pov.cnt_0 = hero_get_clk_counter();

//     // Activate clusters simultaneously
//     // NB: Activation must be after the starting of time measurements
//     if(!cluster_id) cluster_slv_restart_eu_soc_evt(cluster_id, experiment_id);

//     /* ===================================================================== */

//     /* ============================= */
//     /*  Profiling - Color detection  */
//     /* ============================= */

//     /* Transfer input image from L2 */

//     for (int i = 0; i < dma_n_tx; i++) {
//       dma_in[RGB2HSV_CV].job = hero_memcpy_host2dev_async(l1_img_a, l2_img_a, dma_payload_dim * sizeof(uint32_t));
//     }

//     /* Wait DMA to terminate data transfer */

//     hero_dma_wait(dma_in[RGB2HSV_CV].job);

//     /* Pipeline */

//     // RGB2HSV_CV

//     arov_compute(&arov, cluster_id, RGB2HSV_CV);

//     while(!arov_is_finished(&arov, cluster_id, RGB2HSV_CV)){
//       arov_wait_eu(&arov, cluster_id, RGB2HSV_CV);
//     }

//     // THRESHOLD_CV
    
//     arov_compute(&arov, cluster_id, THRESHOLD_CV);
    
//     while(!arov_is_finished(&arov, cluster_id, THRESHOLD_CV)){
//       arov_wait_eu(&arov, cluster_id, THRESHOLD_CV);
//     }

//     // ERODE_CV

//     arov_compute(&arov, cluster_id, ERODE_CV);
    
//     while(!arov_is_finished(&arov, cluster_id, ERODE_CV)){
//       arov_wait_eu(&arov, cluster_id, ERODE_CV);
//     }

//     // DILATE_CV

//     arov_compute(&arov, cluster_id, DILATE_CV);
    
//     while(!arov_is_finished(&arov, cluster_id, DILATE_CV)){
//       arov_wait_eu(&arov, cluster_id, DILATE_CV);
//     }

//     // Update buffer pointers for DILATE_CV
//     arov.dilate_cv_0_3.img_in.tcdm.ptr = l1_img_e; 
//     arov.dilate_cv_0_3.img_out.tcdm.ptr = l1_img_f; 
//     arov_update_buffer_addr(&arov, cluster_id, DILATE_CV);

//     // Update buffer pointers for ERODE_CV
//     arov.erode_cv_0_2.img_in.tcdm.ptr = l1_img_f; 
//     arov.erode_cv_0_2.img_out.tcdm.ptr = l1_img_g; 
//     arov_update_buffer_addr(&arov, cluster_id, ERODE_CV);

//     // DILATE_CV

//     arov_compute(&arov, cluster_id, DILATE_CV);
    
//     while(!arov_is_finished(&arov, cluster_id, DILATE_CV)){
//       arov_wait_eu(&arov, cluster_id, DILATE_CV);
//     }

//     // ERODE_CV

//     arov_compute(&arov, cluster_id, ERODE_CV);
    
//     while(!arov_is_finished(&arov, cluster_id, ERODE_CV)){
//       arov_wait_eu(&arov, cluster_id, ERODE_CV);
//     }

//     /* Transfer input image from L2 */

//     for (int i = 0; i < dma_n_tx; i++) {
//       dma_out[ERODE_CV].job = hero_memcpy_dev2host_async(l2_img_g, l1_img_g, dma_payload_dim * sizeof(uint32_t));
//     }

//     /* Wait DMA to terminate data transfer */

//     hero_dma_wait(dma_in[ERODE_CV].job);

//     /* ===================================================================== */

//     /* MEASUREMENT - END */

//     // Cluster synchronization barrier
//     cluster_barrier_eu_soc_evt(cluster_id, experiment_id);

//     // Cluster timer
//     if(!cluster_id) t_experiment_sys_pov.cnt_1 = hero_get_clk_counter();

//     // Cache statistics and performance counters
//     stop_measurement(cluster_id, core_id, hit, trns, miss);

//     // Update measured cache stats
//     reg_hit  += hit[1] - hit[0];
//     reg_trns += trns[1] - trns[0];
//     reg_miss += miss[1] - miss[0];

//     /* ===================================================================== */

//     /* Print statistics */

//     print_job_stats(
//       // System
//       cluster_id, 
//       core_id, 
//       // Experiment
//       experiment_id, 
//       job_id, 
//       "color_detect_1cl_small", 
//       // Cache
//       &reg_hit,
//       &reg_trns, 
//       &reg_miss, 
//       // Clock counters
//       0xFFFFFFFF,
//       &t_experiment_sys_pov
//     );

//     /* Restart clusters */

//     if(!cluster_id) cluster_slv_restart_eu_soc_evt(cluster_id, experiment_id);

//   } // job_id

//   /* Cleaning accelerators */

//   for(int acc_id=0; acc_id<n_acc_active; acc_id++){
//     arov_free(&arov, cluster_id, acc_id);
//   }

//   /* Cleaning L1 and L2 */
  
//   #if defined(_pulp_rt_)
//     hero_l3free(l2_img_a);
    
//     // TO-DO: Add all buffers

//     hero_l1free(l1_arov_buffer);
//   #endif

//   experiment_id++;
// }




/* =====================================================================
 * Project:      HWPE kernel adapter
 * Title:        hwpe_kernel_adapter.sv
 * Description:  Interface between hardware wrapper and accelerated kernel.
 *
 * $Date:        15.09.2021
 *
 * Target Processor: PULP cores
 * ===================================================================== */

// void run_1cl_small(const int cluster_id, const int core_id) {

//   /* ===================================================================== */

//   int experiment_id = 0;

//   /* Define cache stats */

//   int hit[2], trns[2], miss[2];
//   int reg_hit, reg_trns, reg_miss;

//   /* Cycle counters */

//   pulp_clk_struct t_experiment_sys_pov;

//   /* ===================================================================== */

//   /* Initialize L2 memory */

//   // Variables
//   unsigned l2_n_cl_per_port               = n_clusters/n_l2_ports_virt;
//   unsigned l2_n_bytes_per_port            = l2_size_B/n_l2_ports_phy;
//   unsigned l2_n_words_per_port            = l2_n_bytes_per_port/sizeof(uint32_t);
  
//   // Calculate port ID (Optional: L2 cluster port offset)
//   unsigned l2_cl_port_id                  = cluster_id/l2_n_cl_per_port + l2_cl_port_id_offset;

//   // Declare L2 cluster base address  
//   DEVICE_PTR_CONST l2_cl_base = (l2_cl_port_id==0) ? \
//                                   arov_l2_heap() : \
//                                     // bank 0 holds also the program, so buffers are allocated starting from the heap
//                                   arov_l2_base() + l2_cl_port_id * l2_n_bytes_per_port;
//                                     // same as the HW of the SoC bus 

//   // Declare L2 cluster buffer address  
//   DEVICE_PTR_CONST l2_cl_addr             = l2_cl_base + (cluster_id - (l2_cl_port_id - l2_cl_port_id_offset) * l2_n_cl_per_port) * dma_payload_dim;

//   // Declare L2 image buffers
//   DEVICE_PTR_CONST l2_img_a_0             = l2_cl_addr; // raw image (input image)
//   DEVICE_PTR_CONST l2_img_b_0             = l2_img_a_0 + l2_buffer_dim; // rgb2hsv
//   DEVICE_PTR_CONST l2_img_c_0             = l2_img_b_0 + l2_buffer_dim; // threshold
//   DEVICE_PTR_CONST l2_img_d_0             = l2_img_c_0 + l2_buffer_dim; // erode 1
//   DEVICE_PTR_CONST l2_img_e_0             = l2_img_d_0 + l2_buffer_dim; // dilate 1
//   DEVICE_PTR_CONST l2_img_f_0             = l2_img_e_0 + l2_buffer_dim; // dilate 2
//   DEVICE_PTR_CONST l2_img_g_0             = l2_img_f_0 + l2_buffer_dim; // erode 2 (output image)

//   DEVICE_PTR_CONST l2_img_a_1             = l2_img_g_0 + l2_buffer_dim; // raw image (input image)
//   DEVICE_PTR_CONST l2_img_b_1             = l2_img_a_1 + l2_buffer_dim; // rgb2hsv
//   DEVICE_PTR_CONST l2_img_c_1             = l2_img_b_1 + l2_buffer_dim; // threshold
//   DEVICE_PTR_CONST l2_img_d_1             = l2_img_c_1 + l2_buffer_dim; // erode 1
//   DEVICE_PTR_CONST l2_img_e_1             = l2_img_d_1 + l2_buffer_dim; // dilate 1
//   DEVICE_PTR_CONST l2_img_f_1             = l2_img_e_1 + l2_buffer_dim; // dilate 2
//   DEVICE_PTR_CONST l2_img_g_1             = l2_img_f_1 + l2_buffer_dim; // erode 2 (output image)

//   #if defined(PRINT_LOG)
//     printf(" # - [Params] ID_L2_port:             %d \n",           (int32_t)l2_cl_port_id);
//     printf(" # - [Params] Base_address_L2:        %p \n",           (int32_t)l2_cl_base);
//     printf(" # - [Params] Buffer_address_L2:      %p \n",           (int32_t)l2_cl_addr);
//   #endif

//   #ifdef INPUT_INIT
//     // Initialize input buffer with input image
//     for(int i=0; i<l2_buffer_dim; i++) pulp_write32(l2_img_a_0+i*sizeof(int32_t), in_img_small[i]);
//     for(int i=0; i<l2_buffer_dim; i++) pulp_write32(l2_img_a_1+i*sizeof(int32_t), in_img_small[i]);
//   #endif

//   /* ===================================================================== */

//   /* Initialize L1 memory */

//   #if defined(_pulp_rt_)
//     __device uint32_t* l1_arov_buffer   = (__device uint32_t*)hero_l1malloc(sizeof(uint32_t) * l1_arov_buffer_dim);
//   #elif defined(_pulp_bare_)
//     DEVICE_PTR_CONST l1_arov_buffer    = arov_l1_heap(cluster_id);
//   #endif

//   // Declare L1 image buffers
//   DEVICE_PTR_CONST l1_img_a_0             = l1_arov_buffer; // raw image (input image)
//   DEVICE_PTR_CONST l1_img_b_0             = l1_img_a_0 + l1_buffer_dim; // rgb2hsv
//   DEVICE_PTR_CONST l1_img_c_0             = l1_img_b_0 + l1_buffer_dim; // threshold
//   DEVICE_PTR_CONST l1_img_d_0             = l1_img_c_0 + l1_buffer_dim; // erode 1
//   DEVICE_PTR_CONST l1_img_e_0             = l1_img_d_0 + l1_buffer_dim; // dilate 1
//   DEVICE_PTR_CONST l1_img_f_0             = l1_img_e_0 + l1_buffer_dim; // dilate 2
//   DEVICE_PTR_CONST l1_img_g_0             = l1_img_f_0 + l1_buffer_dim; // erode 2 (output image)

//   DEVICE_PTR_CONST l1_img_a_1             = l1_img_g_0 + l1_buffer_dim; // raw image (input image)
//   DEVICE_PTR_CONST l1_img_b_1             = l1_img_a_1 + l1_buffer_dim; // rgb2hsv
//   DEVICE_PTR_CONST l1_img_c_1             = l1_img_b_1 + l1_buffer_dim; // threshold
//   DEVICE_PTR_CONST l1_img_d_1             = l1_img_c_1 + l1_buffer_dim; // erode 1
//   DEVICE_PTR_CONST l1_img_e_1             = l1_img_d_1 + l1_buffer_dim; // dilate 1
//   DEVICE_PTR_CONST l1_img_f_1             = l1_img_e_1 + l1_buffer_dim; // dilate 2
//   DEVICE_PTR_CONST l1_img_g_1             = l1_img_f_1 + l1_buffer_dim; // erode 2 (output image)

//   /* ===================================================================== */

//   /* System */

//   pulp_dma_struct dma_in[n_acc_active], dma_out[n_acc_active], dma_wait[n_acc_active];

//   /* Accelerators */

//   // Custom registers
//   unsigned rows, cols;

//   int offload_id[n_acc_active];

//   /* Allocate accelerator-rich overlay */

//   arov_struct arov;

//   /* ===================================================================== */

//   /* Cluster steady state condition */
  
//   cluster_barrier_eu_soc_evt(cluster_id, experiment_id);
//   if(!cluster_id) cluster_slv_restart_eu_soc_evt(cluster_id, experiment_id);

//   /* ===================================================================== */

//   /* Runtime experiment parameters */

//   // - Number of image rows
//   rows = img_rows; 

//   // - Number of image columns
//   cols = img_cols;  

//   /* Program accelerators */

//   for(int acc_id=0; acc_id<n_acc_active; acc_id++){

//     // Initialization
//     arov_init(&arov, cluster_id, acc_id);
    
//     // -- rgb2hsv - Map parameters
//     if (cluster_id==0 && acc_id==0) arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img_a, l1_img_b, l1_buffer_dim, rows, cols);
//     // -- threshold - Map parameters
//     if (cluster_id==0 && acc_id==1) arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img_b, l1_img_c, l1_buffer_dim, rows, cols);
//     // -- erode - Map parameters
//     if (cluster_id==0 && acc_id==2) arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img_c, l1_img_d, l1_buffer_dim, rows, cols);
//     // -- dilate - Map parameters
//     if (cluster_id==0 && acc_id==3) arov_map_params_color_detect(&arov, cluster_id, acc_id, l1_img_d, l1_img_e, l1_buffer_dim, rows, cols);

//     // Activation and programming
//     offload_id[acc_id] = arov_activate(&arov, cluster_id, acc_id);
//     arov_program(&arov, cluster_id, acc_id);

//   }

//   /* Flush cache (cold-cache condition) */

//   icache_flush_all();

//   /* Launch profiling jobs */

//   for(int job_id=0; job_id<exp_len_job_queue; job_id++){

//     /* ===================================================================== */

//     /* Reset and start PULP counter */

//     hero_reset_clk_counter();
//     hero_start_clk_counter();

//     /* Reset cache stats */

//     if(cluster_id==0 && core_id==0){

//       icache_stats_reset();

//       reg_hit   = 0;
//       reg_trns  = 0;
//       reg_miss  = 0;

//       /* Reset performance counters */

//       hero_perf_reset_all();

//     }

//     /* ===================================================================== */

//     /* MEASUREMENT - START */

//     // cache statistics and performance counters
//     start_measurement(cluster_id, core_id, hit, trns, miss);

//     // Cluster synchronization barrier
//     cluster_barrier_eu_soc_evt(cluster_id, experiment_id);

//     // Cluster timer
//     if(!cluster_id) t_experiment_sys_pov.cnt_0 = hero_get_clk_counter();

//     // Activate clusters simultaneously
//     // NB: Activation must be after the starting of time measurements
//     if(!cluster_id) cluster_slv_restart_eu_soc_evt(cluster_id, experiment_id);

//     /* ===================================================================== */

//     /* ============================= */
//     /*  Profiling - Color detection  */
//     /* ============================= */

//     /* Transfer input image from L2 */

//     for (int i = 0; i < dma_n_tx; i++) {
//       dma_in[RGB2HSV_CV].job = hero_memcpy_host2dev_async(l1_img_a, l2_img_a, dma_payload_dim * sizeof(uint32_t));
//     }

//     /* Wait DMA to terminate data transfer */

//     hero_dma_wait(dma_in[RGB2HSV_CV].job);

//     /* Pipeline */

//     // RGB2HSV_CV

//     arov_compute(&arov, cluster_id, RGB2HSV_CV);

//     while(!arov_is_finished(&arov, cluster_id, RGB2HSV_CV)){
//       arov_wait_eu(&arov, cluster_id, RGB2HSV_CV);
//     }

//     // THRESHOLD_CV
    
//     arov_compute(&arov, cluster_id, THRESHOLD_CV);
    
//     while(!arov_is_finished(&arov, cluster_id, THRESHOLD_CV)){
//       arov_wait_eu(&arov, cluster_id, THRESHOLD_CV);
//     }

//     // ERODE_CV

//     arov_compute(&arov, cluster_id, ERODE_CV);
    
//     while(!arov_is_finished(&arov, cluster_id, ERODE_CV)){
//       arov_wait_eu(&arov, cluster_id, ERODE_CV);
//     }

//     // DILATE_CV

//     arov_compute(&arov, cluster_id, DILATE_CV);
    
//     while(!arov_is_finished(&arov, cluster_id, DILATE_CV)){
//       arov_wait_eu(&arov, cluster_id, DILATE_CV);
//     }

//     // Update buffer pointers for DILATE_CV
//     arov.dilate_cv_0_3.img_in.tcdm.ptr = l1_img_e; 
//     arov.dilate_cv_0_3.img_out.tcdm.ptr = l1_img_f; 
//     arov_update_buffer_addr(&arov, cluster_id, DILATE_CV);

//     // Update buffer pointers for ERODE_CV
//     arov.erode_cv_0_2.img_in.tcdm.ptr = l1_img_f; 
//     arov.erode_cv_0_2.img_out.tcdm.ptr = l1_img_g; 
//     arov_update_buffer_addr(&arov, cluster_id, ERODE_CV);

//     // DILATE_CV

//     arov_compute(&arov, cluster_id, DILATE_CV);
    
//     while(!arov_is_finished(&arov, cluster_id, DILATE_CV)){
//       arov_wait_eu(&arov, cluster_id, DILATE_CV);
//     }

//     // ERODE_CV

//     arov_compute(&arov, cluster_id, ERODE_CV);
    
//     while(!arov_is_finished(&arov, cluster_id, ERODE_CV)){
//       arov_wait_eu(&arov, cluster_id, ERODE_CV);
//     }

//     /* Transfer input image from L2 */

//     for (int i = 0; i < dma_n_tx; i++) {
//       dma_out[ERODE_CV].job = hero_memcpy_dev2host_async(l2_img_g, l1_img_g, dma_payload_dim * sizeof(uint32_t));
//     }

//     /* Wait DMA to terminate data transfer */

//     hero_dma_wait(dma_in[ERODE_CV].job);

//     /* ===================================================================== */

//     /* MEASUREMENT - END */

//     // Cluster synchronization barrier
//     cluster_barrier_eu_soc_evt(cluster_id, experiment_id);

//     // Cluster timer
//     if(!cluster_id) t_experiment_sys_pov.cnt_1 = hero_get_clk_counter();

//     // Cache statistics and performance counters
//     stop_measurement(cluster_id, core_id, hit, trns, miss);

//     // Update measured cache stats
//     reg_hit  += hit[1] - hit[0];
//     reg_trns += trns[1] - trns[0];
//     reg_miss += miss[1] - miss[0];

//     /* ===================================================================== */

//     /* Print statistics */

//     print_job_stats(
//       // System
//       cluster_id, 
//       core_id, 
//       // Experiment
//       experiment_id, 
//       job_id, 
//       "color_detect_1cl_small", 
//       // Cache
//       &reg_hit,
//       &reg_trns, 
//       &reg_miss, 
//       // Clock counters
//       0xFFFFFFFF,
//       &t_experiment_sys_pov
//     );

//     /* Restart clusters */

//     if(!cluster_id) cluster_slv_restart_eu_soc_evt(cluster_id, experiment_id);

//   } // job_id

//   /* Cleaning accelerators */

//   for(int acc_id=0; acc_id<n_acc_active; acc_id++){
//     arov_free(&arov, cluster_id, acc_id);
//   }

//   /* Cleaning L1 and L2 */
  
//   #if defined(_pulp_rt_)
//     hero_l3free(l2_img_a_0);
//     hero_l3free(l2_img_a_1);

//     // TO-DO: Add all buffers

//     hero_l1free(l1_arov_buffer);
//   #endif

//   experiment_id++;
// }