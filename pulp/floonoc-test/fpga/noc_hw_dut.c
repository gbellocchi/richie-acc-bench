#include "floonoc_test.h"

/*
 * --------------------------------
 * PULP FlooNoC test - Hardware DUT
 * --------------------------------
 */

#pragma omp declare target

void noc_dut(
  __device noc_id* const src_id_xy,
  __device noc_id* const dst_id_xy,
  const size_t traffic_dim,
  const size_t compute_dim,
  uint32_t clk_counter[N_TEST_TRANSACTIONS]
) {

#ifdef TIME_MEASUREMENT
  /* Reset and start PULP counter */
  hero_reset_clk_counter();
  hero_start_clk_counter();

  /* Cycle counters */

  pulp_clk_struct t_experiment[N_TEST_TRANSACTIONS];
#endif

  /* Source/Destination addresses */

  __device uint32_t* src_addr;
  __device uint32_t* dst_addr;

  /* Run tests */

  i_tests: for (int i = 0; i < N_TEST_TRANSACTIONS; i++) {

#ifdef TIME_MEASUREMENT
    // Initialize counters
    t_experiment[i].cnt_0 = 0;
    t_experiment[i].cnt_1 = 0;

    // Start counter
    t_experiment[i].cnt_0 = hero_get_clk_counter();
#endif

#ifdef __DEBUG__
    printf("%d # -- TEST-%d: x%d-y%d (%d) --> x%d-y%d (%d)\n", (uint32_t)i, \
    src_id_xy[i].x, src_id_xy[i].y, xy_to_int(src_id_xy[i]), \
    dst_id_xy[i].x, dst_id_xy[i].y, xy_to_int(dst_id_xy[i]));
#endif

    // Source address calculation
    src_addr = RICHIE_FLOONOC_BASE_ADDR + xy_to_int(src_id_xy[i]) * RICHIE_FLOONOC_OFFS_ADDR;

    // Destination address calculation
    dst_addr = RICHIE_FLOONOC_BASE_ADDR + xy_to_int(dst_id_xy[i]) * RICHIE_FLOONOC_OFFS_ADDR;

    // Launch test
    noc_dut_program(traffic_dim, compute_dim, src_id_xy[i], dst_id_xy[i], src_addr, dst_addr);
    noc_dut_execute(src_id_xy[i], src_addr);
    noc_dut_wait_termination(src_id_xy[i], src_addr);

#ifdef TIME_MEASUREMENT
    // Stop counter
    t_experiment[i].cnt_1 = hero_get_clk_counter();
    clk_counter[i] = t_experiment[i].cnt_1 - t_experiment[i].cnt_0;
#endif
  }
}

#pragma omp end declare target
