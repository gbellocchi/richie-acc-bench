

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

#ifndef __ARCHI_HWPE_CL0_LIC0_H__
#define __ARCHI_HWPE_CL0_LIC0_H__

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

 *     2  |  0x0048  |  31: 0  |  0xffffffff  ||  NB_ITER
 *     3  |  0x004c  |  31: 0  |  0xffffffff  ||  LEN_ITER
 *     4  |  0x0050  |  31:16  |  0xffff0000  ||  SHIFT
 *        |          |   0: 0  |  0x00000001  ||  SIMPLEMUL
 *     5  |  0x0054  |  31: 0  |  0xffffffff  ||  VECTSTRIDE
 *     6  |  0x0058  |  31: 0  |  0xffffffff  ||  VECTSTRIDE2

 *     6  |  0x005c  |  31: 0  |  0xffffffff  ||  HWPE_THRESHOLD

 *     7  |  0x0060  |  31: 0  |  0xffffffff  ||  HWPE_ROWS

 *     8  |  0x0064  |  31: 0  |  0xffffffff  ||  HWPE_COLS
 *
 * ================================================================================
 *
 */

/* HWPE base address (with cluster alias) */

#define ARCHI_HWPE_ADDR_BASE 0x1b202000

/* Event unit */
// NB: Might be also referred to as ARCHI_HWPE_EU_OFFSET

#ifndef ARCHI_CL_EVT_ACC0
#define ARCHI_CL_EVT_ACC0 12
#endif

/* Basic archi */

#define FAST_CORNER_DETECT_REG_TRIGGER                                 0x0

#define FAST_CORNER_DETECT_REG_ACQUIRE                                 0x4

#define FAST_CORNER_DETECT_REG_FINISHED                                0x8

#define FAST_CORNER_DETECT_REG_STATUS                                  0xc

#define FAST_CORNER_DETECT_REG_RUNNING_JOB                             0x10

#define FAST_CORNER_DETECT_REG_SOFT_CLEAR                              0x14

/* Microcode processor registers archi */

  /* Microcode processor */

#define FAST_CORNER_DETECT_REG_BYTECODE                                 0x20

#define FAST_CORNER_DETECT_REG_BYTECODE0_OFFS                           0x0

#define FAST_CORNER_DETECT_REG_BYTECODE1_OFFS                           0x4

#define FAST_CORNER_DETECT_REG_BYTECODE2_OFFS                           0x8

#define FAST_CORNER_DETECT_REG_BYTECODE3_OFFS                           0xc

#define FAST_CORNER_DETECT_REG_BYTECODE4_OFFS                           0x10

#define FAST_CORNER_DETECT_REG_BYTECODE5_LOOPS0_OFFS                    0x14

#define FAST_CORNER_DETECT_REG_LOOPS1_OFFS                              0x18

/* TCDM registers archi */

// Input master ports
#define FAST_CORNER_DETECT_REG_IMG_IN_ADDR                         0x40

// Output master ports
#define FAST_CORNER_DETECT_REG_IMG_OUT_ADDR                        0x44

/* Standard registers archi */

#define FAST_CORNER_DETECT_REG_NB_ITER                         0x48

#define FAST_CORNER_DETECT_REG_LINESTRIDE                0x4c

#define FAST_CORNER_DETECT_REG_TILESTRIDE                0x50

#define FAST_CORNER_DETECT_REG_CNT_LIMIT_IMG_OUT                 0x54

/* Custom registers archi */

// custom regs
#define FAST_CORNER_DETECT_REG_THRESHOLD                0x58

#define FAST_CORNER_DETECT_REG_ROWS                0x5c

#define FAST_CORNER_DETECT_REG_COLS                0x60

/* Address generator archi */

// Input stream - img_in (programmable)
#define FAST_CORNER_DETECT_REG_IMG_IN_TRANS_SIZE                  0x64
#define FAST_CORNER_DETECT_REG_IMG_IN_LINE_STRIDE                 0x68
#define FAST_CORNER_DETECT_REG_IMG_IN_LINE_LENGTH                 0x6c
#define FAST_CORNER_DETECT_REG_IMG_IN_FEAT_STRIDE                 0x70
#define FAST_CORNER_DETECT_REG_IMG_IN_FEAT_LENGTH                 0x74
#define FAST_CORNER_DETECT_REG_IMG_IN_FEAT_ROLL                   0x78
#define FAST_CORNER_DETECT_REG_IMG_IN_LOOP_OUTER                  0x7c
#define FAST_CORNER_DETECT_REG_IMG_IN_REALIGN_TYPE                0x80
#define FAST_CORNER_DETECT_REG_IMG_IN_STEP                        0x84

// Input stream - img_out (programmable)
#define FAST_CORNER_DETECT_REG_IMG_OUT_TRANS_SIZE                 0x88
#define FAST_CORNER_DETECT_REG_IMG_OUT_LINE_STRIDE                0x8c
#define FAST_CORNER_DETECT_REG_IMG_OUT_LINE_LENGTH                0x90
#define FAST_CORNER_DETECT_REG_IMG_OUT_FEAT_STRIDE                0x94
#define FAST_CORNER_DETECT_REG_IMG_OUT_FEAT_LENGTH                0x98
#define FAST_CORNER_DETECT_REG_IMG_OUT_FEAT_ROLL                  0x9c
#define FAST_CORNER_DETECT_REG_IMG_OUT_LOOP_OUTER                 0xa0
#define FAST_CORNER_DETECT_REG_IMG_OUT_REALIGN_TYPE               0xa4
#define FAST_CORNER_DETECT_REG_IMG_OUT_STEP                       0xa8

#endif
