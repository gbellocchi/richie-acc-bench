
/*
 * Copyright (C) 2018-2019 ETH Zurich and University of Bologna
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * Authors:     Francesco Conti <fconti@iis.ee.ethz.ch>
 * Contribute:  Gianluca Bellocchi <gianluca.bellocchi@unimore.it>
 */
#ifndef __ARCHI_HWPE_H__
#define __ARCHI_HWPE_H__
/*
 * Control and generic configuration register layout
 * ================================================================================
 *  # reg |  offset  |  bits   |   bitmask    ||  content
 * -------+----------+---------+--------------++-----------------------------------
 *     0  |  0x0000  |  31: 0  |  0xffffffff  ||  TRIGGER
 *     1  |  0x0004  |  31: 0  |  0xffffffff  ||  ACQUIRE
 *     2  |  0x0008  |  31: 0  |  0xffffffff  ||  EVT_ENABLE
 *     3  |  0x000c  |  31: 0  |  0xffffffff  ||  STATUS
 *     4  |  0x0010  |  31: 0  |  0xffffffff  ||  RUNNING_JOB
 *     5  |  0x0014  |  31: 0  |  0xffffffff  ||  SOFT_CLEAR
 *   6-7  |          |         |              ||  Reserved
 *     8  |  0x0020  |  31: 0  |  0xffffffff  ||  BYTECODE0 [31:0]
 *     9  |  0x0024  |  31: 0  |  0xffffffff  ||  BYTECODE1 [63:32]
 *    10  |  0x0028  |  31: 0  |  0xffffffff  ||  BYTECODE2 [95:64]
 *    11  |  0x002c  |  31: 0  |  0xffffffff  ||  BYTECODE3 [127:96]
 *    12  |  0x0030  |  31: 0  |  0xffffffff  ||  BYTECODE4 [159:128]
 *    13  |  0x0034  |  31:16  |  0xffff0000  ||  LOOPS0    [15:0]
 *        |          |  15: 0  |  0x0000ffff  ||  BYTECODE5 [175:160]
 *    14  |  0x0038  |  31: 0  |  0xffffffff  ||  LOOPS1    [47:16]
 *    15  |          |  31: 0  |  0xffffffff  ||  Reserved
 * ================================================================================
 *
 * Job-dependent registers layout
 * ================================================================================
 *  # reg |  offset  |  bits   |   bitmask    ||  content
 * -------+----------+---------+--------------++-----------------------------------
 *     0  |  0x0040  |  31: 0  |  0xffffffff  ||  A_ADDR
 *     1  |  0x0044  |  31: 0  |  0xffffffff  ||  B_ADDR
 *     2  |  0x0048  |  31: 0  |  0xffffffff  ||  C_ADDR
 *     3  |  0x004c  |  31: 0  |  0xffffffff  ||  NB_ITER
 *     4  |  0x0050  |  31: 0  |  0xffffffff  ||  LEN_ITER
 *     5  |  0x0054  |  31:16  |  0xffff0000  ||  SHIFT
 *        |          |   0: 0  |  0x00000001  ||  SIMPLEMUL
 *     6  |  0x0058  |  31: 0  |  0xffffffff  ||  VECTSTRIDE
 *     7  |  0x005c  |  31: 0  |  0xffffffff  ||  VECTSTRIDE2
 *
 * ================================================================================
 *
 */
#define ARCHI_CL_EVT_ACC0 12
#define ARCHI_HWPE_ADDR_BASE 0x1b201000
#define __builtin_bitinsert(a,b,c,d) (a | (((b << (32-c)) >> (32-c)) << d))
// flag regs
#define HWPE_TRIGGER          0x00
#define HWPE_ACQUIRE          0x04
#define HWPE_FINISHED         0x08
#define HWPE_STATUS           0x0c
#define HWPE_RUNNING_JOB      0x10
#define HWPE_SOFT_CLEAR       0x14
// Microcode-processor regs
#define HWPE_BYTECODE         0x20
#define HWPE_BYTECODE0_OFFS        0x00
#define HWPE_BYTECODE1_OFFS        0x04
#define HWPE_BYTECODE2_OFFS        0x08
#define HWPE_BYTECODE3_OFFS        0x0c
#define HWPE_BYTECODE4_OFFS        0x10
#define HWPE_BYTECODE5_LOOPS0_OFFS 0x14
#define HWPE_LOOPS1_OFFS           0x18
// TCDM address regs
#define HWPE_A_ADDR           0x40
#define HWPE_B_ADDR           0x44
#define HWPE_C_ADDR           0x48

// Standard registers - ULOOP
#define HWPE_NB_ITER          0x4c
// #define HWPE_LEN_ITER         0x50
// #define HWPE_SHIFT_SIMPLEMUL  0x54
#define HWPE_LINESTRIDE       0x50
#define HWPE_TILESTRIDE       0x54

// Standard registers - FSM

// Controls - FSM counter limit (FSM_COMPUTE state)
#define HWPE_CNT_LIMIT        0x58

// Controls - I/O tiles
#define HWPE_IN1_TRANS_SIZE         0x5c
#define HWPE_IN1_LINE_STRIDE        0x60
#define HWPE_IN1_LINE_LENGTH        0x64
#define HWPE_IN1_FEAT_STRIDE        0x68
#define HWPE_IN1_FEAT_LENGTH        0x6c
#define HWPE_IN1_FEAT_ROLL          0x70
#define HWPE_IN1_LOOP_OUTER         0x74
#define HWPE_IN1_REALIGN_TYPE       0x78
#define HWPE_IN2_TRANS_SIZE         0x7c
#define HWPE_IN2_LINE_STRIDE        0x80
#define HWPE_IN2_LINE_LENGTH        0x84
#define HWPE_IN2_FEAT_STRIDE        0x88
#define HWPE_IN2_FEAT_LENGTH        0x8c
#define HWPE_IN2_FEAT_ROLL          0x90
#define HWPE_IN2_LOOP_OUTER         0x94
#define HWPE_IN2_REALIGN_TYPE       0x98
#define HWPE_Y_TRANS_SIZE           0x9c
#define HWPE_Y_LINE_STRIDE          0xa0
#define HWPE_Y_LINE_LENGTH          0xa4
#define HWPE_Y_FEAT_STRIDE          0xa8
#define HWPE_Y_FEAT_LENGTH          0xac
#define HWPE_Y_FEAT_ROLL            0xb0
#define HWPE_Y_LOOP_OUTER           0xb4
#define HWPE_Y_REALIGN_TYPE         0xb8
#endif