#include "floonoc_test.h"

/* NoC SW reference */

// void noc_sw_ref(
//   noc_id src_id_xy,
//   noc_id dst_id_xy,
//   uint32_t traffic_dim
// ) {
//   uint32_t local_buffer[traffic_dim];
//   uint32_t base_addr = RICHIE_FLOONOC_VERIF_BASE_ADDR;

//   // Read
//   for (int i = 0; i < traffic_dim; i++) {
//     local_buffer[i] = NOC_READ(base_addr, xy_to_int(dst_id_xy) * RICHIE_FLOONOC_OFFS_ADDR + i * sizeof(uint32_t));
//   }

//   // Compute
//   for (int i = 0; i < traffic_dim; i++) {
//     local_buffer[i] = xy_to_int(src_id_xy);
//   }

//   // Write
//   for (int i = 0; i < traffic_dim; i++) {
//     NOC_WRITE(base_addr, xy_to_int(dst_id_xy) * RICHIE_FLOONOC_OFFS_ADDR + i * sizeof(uint32_t), local_buffer[i]);
//   }
// }


