#include "floonoc_test.h"

#pragma omp declare target

/*
 * -------------------------------------------
 * PULP FlooNoC test - Calculate golden result
 * -------------------------------------------
 */

void get_golden(
  __device uint32_t* const golden,
  __device noc_id* const src_id_xy,
  __device noc_id* const dst_id_xy
) {
  // Destination address
  uint32_t dst_addr;

  // Initialization value
  uint32_t init = 0;

  // Initialize golden results with initial tile
  // memory value, before computation happens
  for (int x = 0; x < NOC_N_TILES_X; x++) {
    for (int y = 0; y < NOC_N_TILES_Y; y++) {

      uint32_t tid = y + NOC_N_TILES_X * x;

      // Destination address calculation
      dst_addr = RICHIE_FLOONOC_VERIF_BASE_ADDR + tid * RICHIE_FLOONOC_OFFS_ADDR;

      // Initialize content of the tile memory (dimensioned on buffer size)
      i_tile_mem: for (int i_bank = 0; i_bank < TRAFFIC_CFG_BF_SIZE; i_bank++) {

        // Initialize NoC memory
        NOC_WRITE(dst_addr, i_bank * sizeof(uint32_t), init);

        // Initialize golden based on NoC memory content
        golden[tid] = NOC_READ(dst_addr, i_bank * sizeof(uint32_t));
      }
    }
  }

  // Compute golden results
  for (int i = 0; i < N_TEST_TRANSACTIONS; i++) {
    // Retrieve destination memory
    uint32_t dst_id_int = xy_to_int(dst_id_xy[i]);

    // Iterate computation of ID token, as done by the HW traffic generator
    for (int j = 0; j < (COMPUTE_CFG_SIZE/TRAFFIC_CFG_BF_SIZE_MAX_HW); j++) {

      // Compute expected value that the source tile will write
      // in the destination tile memory at the i-th transaction
      golden[dst_id_int] += xy_to_int(src_id_xy[i]);
    }
  }
}

#pragma omp end declare target
