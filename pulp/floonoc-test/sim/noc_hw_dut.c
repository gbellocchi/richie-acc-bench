#include "floonoc_test.h"

/* NoC DUT */

void noc_dut(
  noc_id* src_id_xy,
  noc_id* dst_id_xy,
  uint32_t traffic_dim,
  uint32_t compute_dim
) {
  // Source/Destination addresses
  uint64_t src_addr, dst_addr;

  i_tests: for (int i = 0; i < N_TEST_TRANSACTIONS; i++) {

    // Source address calculation
    src_addr = RICHIE_FLOONOC_BASE_ADDR + xy_to_int(src_id_xy[i]) * RICHIE_FLOONOC_OFFS_ADDR;

    // Destination address calculation
    dst_addr = RICHIE_FLOONOC_BASE_ADDR + xy_to_int(dst_id_xy[i]) * RICHIE_FLOONOC_OFFS_ADDR;

    // Launch test
    noc_dut_program(traffic_dim, compute_dim, src_id_xy[i], dst_id_xy[i], src_addr, dst_addr);
    noc_dut_execute(src_id_xy[i], src_addr);
    noc_dut_wait_termination(src_id_xy[i], src_addr);
  }
}

/* NoC DUT (with tile permutation) */

void noc_dut_permutation() {
  // Source ID
  noc_id src_id_xy, dst_id_xy;
  int src_id_int, dst_id_int;
  uint64_t src_addr, dst_addr;

  // Operational intensity
  uint32_t traffic_dim = TRAFFIC_CFG_SIZE;
  uint32_t compute_dim = COMPUTE_CFG_SIZE;

  // Loop over source X coordinate
  hw_test_src_x: for (src_id_xy.x = 0; src_id_xy.x < NOC_N_TILES_X; src_id_xy.x++) {
    // Loop over source Y coordinate
    hw_test_src_y: for (src_id_xy.y = 0; src_id_xy.y < NOC_N_TILES_Y; src_id_xy.y++) {
      // Check all paths toward other tiles
      hw_test_check_tile_paths: for (int dst_id_int = 0; dst_id_int < NOC_N_TILES; dst_id_int++) {

        int src_id_int = xy_to_int(src_id_xy);

        // Destination cannot correspond to source
        if(src_id_int == dst_id_int){
          ;
        } else {

          // Calculate destination as integer
          dst_id_xy = int_to_xy(dst_id_int);

          // Source address calculation
          src_addr = RICHIE_FLOONOC_BASE_ADDR + src_id_int * RICHIE_FLOONOC_OFFS_ADDR;

          // Destination address calculation
          dst_addr = RICHIE_FLOONOC_BASE_ADDR + dst_id_int * RICHIE_FLOONOC_OFFS_ADDR;

          // Launch test
          noc_dut_program(traffic_dim, compute_dim, src_id_xy, dst_id_xy, src_addr, dst_addr);
          noc_dut_execute(src_id_xy, src_addr);
          noc_dut_wait_termination(src_id_xy, src_addr);
        }
      }
    }
  }
}
