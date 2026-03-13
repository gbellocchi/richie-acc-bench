#ifndef FLOONOC_TEST_H
#define FLOONOC_TEST_H

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

/* System libraries */

#include "def_struct_soc_perf_eval.h"
#include <hero-target.h>

/* Xilinx libraries */

#include "xaxi_hls_tg_hw.h"

/* Third-party */

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c\n"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0')

/* Generic */

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1

#define XPAR_XTRAFFIC_GEN_NUM_INSTANCES 4

/* AXI HLS Traffic Generator */

// Richie memory mapping
#define RICHIE_FLOONOC_BASE_ADDR 0x18000000
#define RICHIE_FLOONOC_VERIF_BASE_ADDR 0x19000000
#define RICHIE_FLOONOC_OFFS_ADDR 0x00020000

// NoC dimensions
#define NOC_N_TILES 4
#define NOC_N_TILES_X 2
#define NOC_N_TILES_Y 2

// Traffic dimension
#define TRAFFIC_CFG_BF_SIZE_MAX_HW 16 // HW parameter - Internal buffer size
#define TRAFFIC_CFG_BF_SIZE 1 // SW parameter - Number of buffer banks that the test validates
#define TRAFFIC_CFG_SIZE 64 // Programmable - Granularity: 16 memory transfers
#define COMPUTE_CFG_SIZE 64 // Programmable - Granularity: 16 compute

// Validation test
#define N_TEST_TRANSACTIONS 8 // Number of transactions executed in the test

/* Setup */

// Measurement
#define TIME_MEASUREMENT

// Other arguments
// #define __VERBOSE__
#define __DEBUG__

/* Richie */

// NoC structs

typedef struct noc_id noc_id;

struct noc_id {
  uint32_t x;
  uint32_t y;
};

// Read/Write over NoC

#define noc_write32(add, val_) (*(volatile unsigned int *)(long)(add) = val_)

static inline uint32_t noc_read32(uint32_t add)
{
  __asm__ __volatile__ ("" : : : "memory");
  uint32_t result = *(volatile uint32_t *)add;
  asm volatile("nop;");
  __asm__ __volatile__ ("" : : : "memory");
  return result;
}

#define NOC_WRITE(base, offset, value) noc_write32((base) + (offset), (value))
#define NOC_READ(base, offset) noc_read32((base) + (offset))

// -------------------------------------------- //

// ID conversion

inline uint32_t xy_to_int(const noc_id id){
  return id.y + NOC_N_TILES_X * id.x;
}

inline noc_id int_to_xy(const uint32_t id){
  noc_id out;

  uint32_t local_id = id;

  out.x = 0;
  while(local_id>=NOC_N_TILES_X){
    local_id-=NOC_N_TILES_X;
    out.x++;
  }
  out.y=local_id;

  return out;
}

// -------------------------------------------- //

/* Traffic generator programming */

inline void noc_dut_program(
  const uint32_t traffic_dim,
  const uint32_t compute_dim,
  const noc_id src_id,
  const noc_id dst_id,
  const uint32_t src_addr,
  const uint32_t dst_addr
) {
  // Set traffic dimension (ref -> "XAxi_hls_tg_Set_traffic_dim")
  NOC_WRITE(src_addr, XAXI_HLS_TG_CONTROL_ADDR_TRAFFIC_DIM_DATA, (uint32_t)(traffic_dim));
  NOC_WRITE(src_addr, XAXI_HLS_TG_CONTROL_ADDR_TRAFFIC_DIM_DATA + 4, (uint32_t)(traffic_dim >> 32));

  // Set compute dimension (ref -> "XAxi_hls_tg_Set_traffic_dim")
  NOC_WRITE(src_addr, XAXI_HLS_TG_CONTROL_ADDR_COMPUTE_DIM_DATA, (uint32_t)(compute_dim));
  NOC_WRITE(src_addr, XAXI_HLS_TG_CONTROL_ADDR_COMPUTE_DIM_DATA + 4, (uint32_t)(compute_dim >> 32));

  // Set traffic generator ID (ref -> "XAxi_hls_tg_Set_traffic_idx")
  NOC_WRITE(src_addr, XAXI_HLS_TG_CONTROL_ADDR_TRAFFIC_ID_DATA, (uint32_t)(xy_to_int(src_id)));
  NOC_WRITE(src_addr, XAXI_HLS_TG_CONTROL_ADDR_TRAFFIC_ID_DATA + 4, (uint32_t)(xy_to_int(src_id) >> 32));

  // Set destination address (ref -> "XAxi_hls_tg_Set_traffic_dst_offset")
  NOC_WRITE(src_addr, XAXI_HLS_TG_CONTROL_ADDR_TRAFFIC_DST_OFFSET_DATA, (uint32_t)(dst_addr));
  NOC_WRITE(src_addr, XAXI_HLS_TG_CONTROL_ADDR_TRAFFIC_DST_OFFSET_DATA + 4, (uint32_t)(dst_addr >> 32));
}

// -------------------------------------------- //

/* Traffic generator execution */

inline void noc_dut_execute(
  const noc_id src_id,
  const uint32_t src_addr
) {
  uint32_t Data;

  // Start accelerator (ref -> "XAxi_hls_tg_Start")
  __asm__ __volatile__ ("" : : : "memory");
  Data = NOC_READ(src_addr, XAXI_HLS_TG_CONTROL_ADDR_AP_CTRL) & 0x80;
  NOC_WRITE(src_addr, XAXI_HLS_TG_CONTROL_ADDR_AP_CTRL, Data | 0x01);
  __asm__ __volatile__ ("" : : : "memory");
}

// -------------------------------------------- //

/* Check termination of traffic generator */

inline void noc_dut_wait_termination(
  const noc_id src_id,
  const uint32_t src_addr
) {

  uint32_t Data, Done = 0;

  // Check when accelerator operation terminates (ref -> "XAxi_hls_tg_IsDone")
  __asm__ __volatile__ ("" : : : "memory");
  while (!Done){
    Data = NOC_READ(src_addr, XAXI_HLS_TG_CONTROL_ADDR_AP_CTRL);
    Done = (Data >> 1) & 0x1;
  };
  __asm__ __volatile__ ("" : : : "memory");

// #ifdef __DEBUG__
//   printf("%d -- Data: %d (%c%c%c%c%c%c%c%c)\n", Data, BYTE_TO_BINARY(Data));
//   printf("%d -- Done: %d (%c%c%c%c%c%c%c%c)\n", Done, BYTE_TO_BINARY(Done));
// #endif
}

// -------------------------------------------- //

#endif
