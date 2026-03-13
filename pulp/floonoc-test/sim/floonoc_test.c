#include "floonoc_test.h"

int main(int argc, char *argv[]) {

  printf("\n");

  const int core_id = hero_rt_core_id();
  const int cluster_id = hero_rt_cluster_id();

  if(core_id>0){

    printf("Sleeping...\n");
    while(1);

  } else {

    printf("I am the PROXY\n");

    // Source ID
    noc_id src_id_xy[N_TEST_TRANSACTIONS];

    // Destination ID
    noc_id dst_id_xy[N_TEST_TRANSACTIONS];

    // Addresses
    uint64_t dst_addr;

    // Operational intensity
    uint32_t traffic_dim = TRAFFIC_CFG_SIZE;
    uint32_t compute_dim = COMPUTE_CFG_SIZE;

    // DUT values (dimensioned on buffer size)
    uint64_t dut_vals[TRAFFIC_CFG_BF_SIZE];

    // Golden values
    uint64_t golden[NOC_N_TILES]; // golden result
    int dst_tile_include_mask[NOC_N_TILES]; // mask array to discriminate tested tile memories
    int dut_fail[NOC_N_TILES];

    //
    //  PREPARE NOC TILE PAIRS
    //

    printf("Prepare NoC tile pairs\n");

    // Initiale the array stating which tiles to test
    for (int x = 0; x < NOC_N_TILES_X; x++) {
      for (int y = 0; y < NOC_N_TILES_Y; y++) {
        int tid = y + NOC_N_TILES_X * x;
        dst_tile_include_mask[tid] = 0;
        dut_fail[tid] = 0;
      }
    }

    // Randomly aggregate pair of tiles to conduct the test
    create_tile_pairs: for (int i = 0; i < N_TEST_TRANSACTIONS; i++) {
      src_id_xy[i] = int_to_xy(3); //int_to_xy(rand() % NOC_N_TILES);
      dst_id_xy[i] = int_to_xy(2); //int_to_xy(rand() % NOC_N_TILES);

      // The pairs must correspond to different tiles
      while(xy_to_int(src_id_xy[i])==xy_to_int(dst_id_xy[i]))
        dst_id_xy[i] = int_to_xy(rand() % NOC_N_TILES);

#ifdef __DEBUG__
      printf(" # TEST-%d: x%d-y%d (%d) --> x%d-y%d (%d)\n", i, \
      src_id_xy[i].x, src_id_xy[i].y, xy_to_int(src_id_xy[i]), \
      dst_id_xy[i].x, dst_id_xy[i].y, xy_to_int(dst_id_xy[i]));
#endif

      // Update list of tiles to validate
      dst_tile_include_mask[xy_to_int(dst_id_xy[i])] = 1;
    }

#ifdef __VERBOSE__
    for (int x = 0; x < NOC_N_TILES_X; x++) {
      for (int y = 0; y < NOC_N_TILES_Y; y++) {

        int tid = y + NOC_N_TILES_X * x;

        if (dst_tile_include_mask[tid]) {
          printf(" # TILE-%d%d (%d) to test\n", x, y, tid);
        } else {
          printf(" # TILE-%d%d (%d) excluded\n", x, y, tid);
        }
      }
    }
#endif

    //
    //  CALCULATE GOLDEN RESULTS
    //

    printf("Calculate golden results\n");

    // Calculate golden results
    get_golden(golden, src_id_xy, dst_id_xy);

    for (int x = 0; x < NOC_N_TILES_X; x++) {
      for (int y = 0; y < NOC_N_TILES_Y; y++) {

        int tid = y + NOC_N_TILES_X * x;

#ifdef __DEBUG__
        if(dst_tile_include_mask[tid])
          printf(" # Golden-%d%d (%d): %d\n", x, y, tid, golden[tid]);
#endif
      }
    }

    //
    //  RUN THE DUT
    //

    printf("NoC DUT\n");

    noc_dut(src_id_xy, dst_id_xy, traffic_dim, compute_dim);

    //
    //  VALIDATE RESULTS
    //

    printf("NoC Validation\n");

    // Validate tile by tile
    for (int x = 0; x < NOC_N_TILES_X; x++) {
      for (int y = 0; y < NOC_N_TILES_Y; y++) {

        int tid = y + NOC_N_TILES_X * x;

        // Check if the tile has been tested or not
        if(dst_tile_include_mask[tid]){

          // Calculate destination address
          dst_addr = RICHIE_FLOONOC_VERIF_BASE_ADDR + tid * RICHIE_FLOONOC_OFFS_ADDR;

          // Read content of the tile memory (dimensioned on buffer size)
          i_tile_mem: for (int i_bank = 0; i_bank < TRAFFIC_CFG_BF_SIZE; i_bank++) {

            // Read value
            dut_vals[i_bank] = NOC_READ(dst_addr, i_bank * sizeof(uint64_t));

            // Check consistency with golden results
            if(golden[tid] != dut_vals[i_bank]) {
              dut_fail[tid] = 1;
              printf(" # Error:\n");
              printf(" # - Real-%d%d (%d): %d\n", x, y, tid, dut_vals[i_bank]);
              printf(" # - Golden-%d%d (%d): %d\n", x, y, tid, golden[tid]);
            }
          }
          // Validate results by comparing golden values with DUT values in the NoC memory subsystem
          if(!dut_fail[tid])
            printf(" # TILE-%d test - SUCCESS\n", tid);
          else
            printf(" # TILE-%d test - FAILED\n", tid);
        } else {
          printf(" # TILE-%d test - NOT TESTED\n", tid);
        }
      }
    }
  }

  return 0;
}
