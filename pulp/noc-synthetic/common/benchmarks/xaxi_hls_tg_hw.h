// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2022.2.1 (64-bit)
// Tool Version Limit: 2019.12
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// ==============================================================
// control
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read/COR)
//        bit 7  - auto_restart (Read/Write)
//        bit 9  - interrupt (Read)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0 - enable ap_done interrupt (Read/Write)
//        bit 1 - enable ap_ready interrupt (Read/Write)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0 - ap_done (Read/TOW)
//        bit 1 - ap_ready (Read/TOW)
//        others - reserved
// 0x10 : Data signal of traffic_dst_offset
//        bit 31~0 - traffic_dst_offset[31:0] (Read/Write)
// 0x14 : Data signal of traffic_dst_offset
//        bit 31~0 - traffic_dst_offset[63:32] (Read/Write)
// 0x18 : reserved
// 0x1c : Data signal of traffic_dim
//        bit 31~0 - traffic_dim[31:0] (Read/Write)
// 0x20 : Data signal of traffic_dim
//        bit 31~0 - traffic_dim[63:32] (Read/Write)
// 0x24 : reserved
// 0x28 : Data signal of compute_dim
//        bit 31~0 - compute_dim[31:0] (Read/Write)
// 0x2c : Data signal of compute_dim
//        bit 31~0 - compute_dim[63:32] (Read/Write)
// 0x30 : reserved
// 0x34 : Data signal of traffic_id
//        bit 31~0 - traffic_id[31:0] (Read/Write)
// 0x38 : Data signal of traffic_id
//        bit 31~0 - traffic_id[63:32] (Read/Write)
// 0x3c : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XAXI_HLS_TG_CONTROL_ADDR_AP_CTRL                 0x00
#define XAXI_HLS_TG_CONTROL_ADDR_GIE                     0x04
#define XAXI_HLS_TG_CONTROL_ADDR_IER                     0x08
#define XAXI_HLS_TG_CONTROL_ADDR_ISR                     0x0c
#define XAXI_HLS_TG_CONTROL_ADDR_TRAFFIC_DST_OFFSET_DATA 0x10
#define XAXI_HLS_TG_CONTROL_BITS_TRAFFIC_DST_OFFSET_DATA 64
#define XAXI_HLS_TG_CONTROL_ADDR_TRAFFIC_DIM_DATA        0x1c
#define XAXI_HLS_TG_CONTROL_BITS_TRAFFIC_DIM_DATA        64
#define XAXI_HLS_TG_CONTROL_ADDR_COMPUTE_DIM_DATA        0x28
#define XAXI_HLS_TG_CONTROL_BITS_COMPUTE_DIM_DATA        64
#define XAXI_HLS_TG_CONTROL_ADDR_TRAFFIC_ID_DATA         0x34
#define XAXI_HLS_TG_CONTROL_BITS_TRAFFIC_ID_DATA         64

