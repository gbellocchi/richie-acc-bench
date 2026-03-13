#ifndef FLOONOC_TEST_H
#define FLOONOC_TEST_H

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <hero-target.h>
#include "xaxi_hls_tg_hw.h"

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
#define TRAFFIC_CFG_SIZE 16 // Programmable - Granularity: 16 memory transfers
#define COMPUTE_CFG_SIZE 16 // Programmable - Granularity: 16 compute

// Validation test
#define N_TEST_TRANSACTIONS 1 // Number of transactions executed in the test

// Other arguments
// #define __DEBUG__
// #define __VERBOSE__

typedef struct noc_id noc_id;

struct noc_id {
  int x;
  int y;
};

/* Richie */

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

inline int xy_to_int(noc_id id){
  return id.y + NOC_N_TILES_X * id.x;
}

inline noc_id int_to_xy(int id){
  noc_id out;

  out.x = 0;
  while(id>=NOC_N_TILES_X){
    id-=NOC_N_TILES_X;
    out.x++;
  }
  out.y=id;

  return out;
}

// -------------------------------------------- //

/* Traffic generator programming */

inline void noc_dut_program(
  uint32_t traffic_dim,
  uint32_t compute_dim,
  noc_id src_id,
  noc_id dst_id,
  uint64_t src_addr,
  uint64_t dst_addr
) {
#ifdef __VERBOSE__
  printf("-- Programming\n", src_id.x, src_id.y);
#endif

  // Set traffic dimension (ref -> "XAxi_hls_tg_Set_traffic_dim")
  NOC_WRITE(src_addr, XAXI_HLS_TG_CONTROL_ADDR_TRAFFIC_DIM_DATA, (uint32_t)(traffic_dim));
  NOC_WRITE(src_addr, XAXI_HLS_TG_CONTROL_ADDR_TRAFFIC_DIM_DATA + 4, (uint32_t)(traffic_dim >> 32));

  // Set traffic dimension (ref -> "XAxi_hls_tg_Set_traffic_dim")
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
  noc_id src_id,
  uint64_t src_addr
) {
#ifdef __VERBOSE__
  printf("-- Execution\n", src_id.x, src_id.y);
#endif

  uint32_t Data;

  // Start accelerator (ref -> "XAxi_hls_tg_Start")
  Data = NOC_READ(src_addr, XAXI_HLS_TG_CONTROL_ADDR_AP_CTRL) & 0x80;
  NOC_WRITE(src_addr, XAXI_HLS_TG_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

// -------------------------------------------- //

/* Check termination of traffic generator */

inline void noc_dut_wait_termination(
  noc_id src_id,
  uint64_t src_addr
) {

  uint32_t Data, Done = 0;

  // Check when accelerator operation terminates (ref -> "XAxi_hls_tg_IsDone")
  while (!Done){
    Data = NOC_READ(src_addr, XAXI_HLS_TG_CONTROL_ADDR_AP_CTRL);
    Done = (Data >> 1) & 0x1;
  };

#ifdef __VERBOSE__
  printf("-- Termination\n", src_id.x, src_id.y);
#endif
}

// -------------------------------------------- //

#endif
