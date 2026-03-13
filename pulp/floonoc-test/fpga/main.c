#include "floonoc_test.h"

/*
 * ------------------------
 * PULP FlooNoC test - Main
 * ------------------------
 */

#pragma omp declare target

void floonoc_test(
  uint32_t clk_counter[N_TEST_TRANSACTIONS]
) {

#ifdef __VERBOSE__
  printf("%d # FlooNoC test - Begin\n");
#endif

  /* Source and destination */

  // Source ID (XY)
  noc_id src_id_xy[N_TEST_TRANSACTIONS];

  // Destination ID (XY)
  noc_id dst_id_xy[N_TEST_TRANSACTIONS];

  // Destination address
  __device uint32_t* dst_addr;

  /* Traffic parameters */

  // Operational intensity
  uint32_t traffic_dim = TRAFFIC_CFG_SIZE;
  uint32_t compute_dim = COMPUTE_CFG_SIZE;

  // DUT values (dimensioned on buffer size)
  __device uint32_t* dut_vals = (__device uint32_t *)hero_l1malloc(TRAFFIC_CFG_BF_SIZE * sizeof(uint32_t));

  // Golden results
  __device uint32_t* golden = (__device uint32_t *)hero_l1malloc(NOC_N_TILES * sizeof(uint32_t));

  // Mask array to discriminate tested tile memories
  __device uint32_t* dst_tile_include_mask = (__device uint32_t *)hero_l1malloc(NOC_N_TILES * sizeof(uint32_t));

  // Success/Fail flag
  __device uint32_t* dut_fail = (__device uint32_t *)hero_l1malloc(NOC_N_TILES * sizeof(uint32_t));

  /* Prepare NoC tile pairs */

#ifdef __VERBOSE__
  printf("%d # Prepare NoC tile pairs:\n");
#endif

  // Initiale the array stating which tiles to test
  for (int x = 0; x < NOC_N_TILES_X; x++) {
    for (int y = 0; y < NOC_N_TILES_Y; y++) {
      uint32_t tid = y + NOC_N_TILES_X * x;
      dst_tile_include_mask[tid] = 0;
      dut_fail[tid] = 0;
    }
  }

  // Randomly aggregate pair of tiles to conduct the test
  create_tile_pairs: for (int i = 0; i < N_TEST_TRANSACTIONS; i++) {
    src_id_xy[i] = int_to_xy(rand() % NOC_N_TILES);
    dst_id_xy[i] = int_to_xy(rand() % NOC_N_TILES);

    // The pairs must correspond to different tiles
    while(xy_to_int(src_id_xy[i])==xy_to_int(dst_id_xy[i]))
      dst_id_xy[i] = int_to_xy(rand() % NOC_N_TILES);

#ifdef __VERBOSE__
    printf("%d # -- TEST-%d: x%d-y%d (%d) --> x%d-y%d (%d)\n",     \
      (uint32_t)i,                                              \
      src_id_xy[i].x, src_id_xy[i].y, xy_to_int(src_id_xy[i]),  \
      dst_id_xy[i].x, dst_id_xy[i].y, xy_to_int(dst_id_xy[i])   \
    );
#endif

    // Update list of tiles to validate
    dst_tile_include_mask[xy_to_int(dst_id_xy[i])] = 1;
  }

#ifdef __VERBOSE__
  printf("%d # Resume of NoC tiles being tested:\n");
  for (int x = 0; x < NOC_N_TILES_X; x++) {
    for (int y = 0; y < NOC_N_TILES_Y; y++) {

      uint32_t tid = y + NOC_N_TILES_X * x;

      if (dst_tile_include_mask[tid]) {
        printf("%d # -- TILE-%d%d (%d) to test\n", x, y, tid);
      } else {
        printf("%d # -- TILE-%d%d (%d) excluded\n", x, y, tid);
      }
    }
  }
#endif

  //
  //  CALCULATE GOLDEN RESULTS
  //

#ifdef __VERBOSE__
  printf("%d # Calculate golden results\n");
#endif

  // Calculate golden results
  get_golden(golden, src_id_xy, dst_id_xy);

  for (int x = 0; x < NOC_N_TILES_X; x++) {
    for (int y = 0; y < NOC_N_TILES_Y; y++) {

      uint32_t tid = y + NOC_N_TILES_X * x;

#ifdef __DEBUG__
      if(dst_tile_include_mask[tid])
        printf("%d # Golden-%d%d (%d): %d\n", x, y, tid, golden[tid]);
#endif
    }
  }

  //
  //  RUN THE DUT
  //

#ifdef __VERBOSE__
  printf("%d # Execute NoC test\n");
#endif

  noc_dut(src_id_xy, dst_id_xy, traffic_dim, compute_dim, clk_counter);

  //
  //  VALIDATE RESULTS
  //

#ifdef __VERBOSE__
  printf("%d # Validate test results:\n");
#endif

  // Validate tile by tile
  for (int x = 0; x < NOC_N_TILES_X; x++) {
    for (int y = 0; y < NOC_N_TILES_Y; y++) {

      uint32_t tid = y + NOC_N_TILES_X * x;

      // Check if the tile has been tested or not
      if(dst_tile_include_mask[tid]){

        // Calculate destination address
        dst_addr = RICHIE_FLOONOC_VERIF_BASE_ADDR + tid * RICHIE_FLOONOC_OFFS_ADDR;

        // Read content of the tile memory (dimensioned on buffer size)
        i_tile_mem: for (int i_bank = 0; i_bank < TRAFFIC_CFG_BF_SIZE; i_bank++) {

          // Read value
          dut_vals[i_bank] = NOC_READ(dst_addr, i_bank * sizeof(uint32_t));

          // Check consistency with golden results
          if(golden[tid] != dut_vals[i_bank]) {
            dut_fail[tid] = 1;
#ifdef __DEBUG__
            printf("%d # -- Error:\n");
            printf("%d # -- >> Real value of Tile-%d%d (%d): %d\n", x, y, tid, dut_vals[i_bank]);
            printf("%d # -- >> Golden value of Tile-%d%d (%d): %d\n", x, y, tid, golden[tid]);
#endif
          } else {
#ifdef __DEBUG__
            printf("%d # -- >> Real value of Tile-%d%d (%d): %d\n", x, y, tid, dut_vals[i_bank]);
#endif
          }
        }
        // Validate results by comparing golden values with DUT values in the NoC memory subsystem
#ifdef __DEBUG__
        if(!dut_fail[tid])
          printf("%d # -- TILE-%d --> SUCCESS\n", tid);
        else
          printf("%d # -- TILE-%d --> FAILED\n", tid);
#endif

      } else {

#ifdef __DEBUG__
        printf("%d # -- TILE-%d --> NOT TESTED\n", tid);
#endif

      }
    }
  }

#if defined(__VERBOSE__)
  printf("%d # FlooNoC test - End\n");
#endif
}

#pragma omp end declare target

/*
 * ---------------------
 * Host processor - Main
 * ---------------------
 */

int main(int argc, char *argv[]) {

  uint32_t clk_counter[N_TEST_TRANSACTIONS];

  /* Warmup */
#pragma omp target device(BIGPULP_MEMCPY) \
  map(from: clk_counter[0:N_TEST_TRANSACTIONS-1])
{
  ;
}

  /* Experiment */
#pragma omp target device(BIGPULP_MEMCPY) \
  map(from: clk_counter[0:N_TEST_TRANSACTIONS-1])
  floonoc_test(clk_counter);

#ifdef TIME_MEASUREMENT
  /* Results */

  const unsigned pulp_freq = 50 * 1000000;

  i_tests: for (int i = 0; i < N_TEST_TRANSACTIONS; i++) {
    const unsigned long long  cycles_pulp           = clk_counter[i];
    const double              pulp_ms               = (double) (cycles_pulp * 1000) / pulp_freq;

    printf("# \nExperiment-%d:\n", i);
    printf("#   - Execution time (ck):    %llu ck\n",  cycles_pulp);
    printf("#   - Execution time (ms):    %.3f ms\n",  pulp_ms);
  }
#endif

  return 0;
}
