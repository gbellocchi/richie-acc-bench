#include "floonoc_test.h"

/* NoC SW reference */

void noc_sw_ref(
  noc_id src_id_xy,
  noc_id dst_id_xy,
  uint32_t traffic_dim
) {
  uint32_t local_buffer[traffic_dim];
  uint32_t base_addr = RICHIE_FLOONOC_VERIF_BASE_ADDR;

  // Read
  for (int i = 0; i < traffic_dim; i++) {
    local_buffer[i] = NOC_READ(base_addr, xy_to_int(dst_id_xy) * RICHIE_FLOONOC_OFFS_ADDR + i * sizeof(uint32_t));
  }

  // Compute
  for (int i = 0; i < traffic_dim; i++) {
    local_buffer[i] = xy_to_int(src_id_xy);
  }

  // Write
  for (int i = 0; i < traffic_dim; i++) {
    NOC_WRITE(base_addr, xy_to_int(dst_id_xy) * RICHIE_FLOONOC_OFFS_ADDR + i * sizeof(uint32_t), local_buffer[i]);
  }
}

/* Calculate golden result */

void get_golden(
  uint64_t* golden,
  noc_id* src_id_xy,
  noc_id* dst_id_xy
) {
  // Destination address
  uint64_t dst_addr;

  // Initialization value
  uint64_t init = 0;

  // Initialize golden results with initial tile
  // memory value, before computation happens
  for (int x = 0; x < NOC_N_TILES_X; x++) {
    for (int y = 0; y < NOC_N_TILES_Y; y++) {

      int tid = y + NOC_N_TILES_X * x;

      // Destination address calculation
      dst_addr = RICHIE_FLOONOC_VERIF_BASE_ADDR + tid * RICHIE_FLOONOC_OFFS_ADDR;

      // Initialize content of the tile memory (dimensioned on buffer size)
      i_tile_mem: for (int i_bank = 0; i_bank < TRAFFIC_CFG_BF_SIZE; i_bank++) {

        // Initialize NoC memory
        NOC_WRITE(dst_addr, i_bank * sizeof(uint64_t), init);

        // Initialize golden based on NoC memory content
        golden[tid] = NOC_READ(dst_addr, i_bank * sizeof(uint64_t));
      }
    }
  }

  // Compute golden results
  for (int i = 0; i < N_TEST_TRANSACTIONS; i++) {
    // Retrieve destination memory
    int dst_id_int = xy_to_int(dst_id_xy[i]);

    // Iterate computation of ID token, as done by the HW traffic generator
    for (int j = 0; j < (COMPUTE_CFG_SIZE/TRAFFIC_CFG_BF_SIZE_MAX_HW); j++) {

      // Compute expected value that the source tile will write
      // in the destination tile memory at the i-th transaction
      golden[dst_id_int] += xy_to_int(src_id_xy[i]);
    }
  }
}
