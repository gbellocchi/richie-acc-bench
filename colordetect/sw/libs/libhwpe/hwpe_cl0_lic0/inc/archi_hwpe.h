

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

 *     6  |  0x005c  |  31: 0  |  0xffffffff  ||  HWPE_ROWS

 *     7  |  0x0060  |  31: 0  |  0xffffffff  ||  HWPE_COLS

 *     8  |  0x0064  |  31: 0  |  0xffffffff  ||  HWPE_LOW_THRESH_0

 *     9  |  0x0068  |  31: 0  |  0xffffffff  ||  HWPE_LOW_THRESH_1

 *     10  |  0x006c  |  31: 0  |  0xffffffff  ||  HWPE_LOW_THRESH_2

 *     11  |  0x0070  |  31: 0  |  0xffffffff  ||  HWPE_LOW_THRESH_3

 *     12  |  0x0074  |  31: 0  |  0xffffffff  ||  HWPE_LOW_THRESH_4

 *     13  |  0x0078  |  31: 0  |  0xffffffff  ||  HWPE_LOW_THRESH_5

 *     14  |  0x007c  |  31: 0  |  0xffffffff  ||  HWPE_LOW_THRESH_6

 *     15  |  0x0080  |  31: 0  |  0xffffffff  ||  HWPE_LOW_THRESH_7

 *     16  |  0x0084  |  31: 0  |  0xffffffff  ||  HWPE_LOW_THRESH_8

 *     17  |  0x0088  |  31: 0  |  0xffffffff  ||  HWPE_HIGH_THRESH_0

 *     18  |  0x008c  |  31: 0  |  0xffffffff  ||  HWPE_HIGH_THRESH_1

 *     19  |  0x0090  |  31: 0  |  0xffffffff  ||  HWPE_HIGH_THRESH_2

 *     20  |  0x0094  |  31: 0  |  0xffffffff  ||  HWPE_HIGH_THRESH_3

 *     21  |  0x0098  |  31: 0  |  0xffffffff  ||  HWPE_HIGH_THRESH_4

 *     22  |  0x009c  |  31: 0  |  0xffffffff  ||  HWPE_HIGH_THRESH_5

 *     23  |  0x00a0  |  31: 0  |  0xffffffff  ||  HWPE_HIGH_THRESH_6

 *     24  |  0x00a4  |  31: 0  |  0xffffffff  ||  HWPE_HIGH_THRESH_7

 *     25  |  0x00a8  |  31: 0  |  0xffffffff  ||  HWPE_HIGH_THRESH_8

 *     26  |  0x00ac  |  31: 0  |  0xffffffff  ||  HWPE_PROCESS_SHAPE_0

 *     27  |  0x00b0  |  31: 0  |  0xffffffff  ||  HWPE_PROCESS_SHAPE_1

 *     28  |  0x00b4  |  31: 0  |  0xffffffff  ||  HWPE_PROCESS_SHAPE_2

 *     29  |  0x00b8  |  31: 0  |  0xffffffff  ||  HWPE_PROCESS_SHAPE_3

 *     30  |  0x00bc  |  31: 0  |  0xffffffff  ||  HWPE_PROCESS_SHAPE_4

 *     31  |  0x00c0  |  31: 0  |  0xffffffff  ||  HWPE_PROCESS_SHAPE_5

 *     32  |  0x00c4  |  31: 0  |  0xffffffff  ||  HWPE_PROCESS_SHAPE_6

 *     33  |  0x00c8  |  31: 0  |  0xffffffff  ||  HWPE_PROCESS_SHAPE_7

 *     34  |  0x00cc  |  31: 0  |  0xffffffff  ||  HWPE_PROCESS_SHAPE_8
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

#define COLOR_DETECT_REG_TRIGGER                                 0x0

#define COLOR_DETECT_REG_ACQUIRE                                 0x4

#define COLOR_DETECT_REG_FINISHED                                0x8

#define COLOR_DETECT_REG_STATUS                                  0xc

#define COLOR_DETECT_REG_RUNNING_JOB                             0x10

#define COLOR_DETECT_REG_SOFT_CLEAR                              0x14

/* Microcode processor registers archi */

  /* Microcode processor */

#define COLOR_DETECT_REG_BYTECODE                                 0x20

#define COLOR_DETECT_REG_BYTECODE0_OFFS                           0x0

#define COLOR_DETECT_REG_BYTECODE1_OFFS                           0x4

#define COLOR_DETECT_REG_BYTECODE2_OFFS                           0x8

#define COLOR_DETECT_REG_BYTECODE3_OFFS                           0xc

#define COLOR_DETECT_REG_BYTECODE4_OFFS                           0x10

#define COLOR_DETECT_REG_BYTECODE5_LOOPS0_OFFS                    0x14

#define COLOR_DETECT_REG_LOOPS1_OFFS                              0x18

/* TCDM registers archi */

// Input master ports
#define COLOR_DETECT_REG_IMG_IN_ADDR                         0x40

// Output master ports
#define COLOR_DETECT_REG_IMG_OUT_ADDR                        0x44

/* Standard registers archi */

#define COLOR_DETECT_REG_NB_ITER                         0x48

#define COLOR_DETECT_REG_LINESTRIDE                0x4c

#define COLOR_DETECT_REG_TILESTRIDE                0x50

#define COLOR_DETECT_REG_CNT_LIMIT_IMG_OUT                 0x54

/* Custom registers archi */

// custom regs
#define COLOR_DETECT_REG_ROWS                0x58

#define COLOR_DETECT_REG_COLS                0x5c

#define COLOR_DETECT_REG_LOW_THRESH_0                0x60

#define COLOR_DETECT_REG_LOW_THRESH_1                0x64

#define COLOR_DETECT_REG_LOW_THRESH_2                0x68

#define COLOR_DETECT_REG_LOW_THRESH_3                0x6c

#define COLOR_DETECT_REG_LOW_THRESH_4                0x70

#define COLOR_DETECT_REG_LOW_THRESH_5                0x74

#define COLOR_DETECT_REG_LOW_THRESH_6                0x78

#define COLOR_DETECT_REG_LOW_THRESH_7                0x7c

#define COLOR_DETECT_REG_LOW_THRESH_8                0x80

#define COLOR_DETECT_REG_HIGH_THRESH_0                0x84

#define COLOR_DETECT_REG_HIGH_THRESH_1                0x88

#define COLOR_DETECT_REG_HIGH_THRESH_2                0x8c

#define COLOR_DETECT_REG_HIGH_THRESH_3                0x90

#define COLOR_DETECT_REG_HIGH_THRESH_4                0x94

#define COLOR_DETECT_REG_HIGH_THRESH_5                0x98

#define COLOR_DETECT_REG_HIGH_THRESH_6                0x9c

#define COLOR_DETECT_REG_HIGH_THRESH_7                0xa0

#define COLOR_DETECT_REG_HIGH_THRESH_8                0xa4

#define COLOR_DETECT_REG_PROCESS_SHAPE_0                0xa8

#define COLOR_DETECT_REG_PROCESS_SHAPE_1                0xac

#define COLOR_DETECT_REG_PROCESS_SHAPE_2                0xb0

#define COLOR_DETECT_REG_PROCESS_SHAPE_3                0xb4

#define COLOR_DETECT_REG_PROCESS_SHAPE_4                0xb8

#define COLOR_DETECT_REG_PROCESS_SHAPE_5                0xbc

#define COLOR_DETECT_REG_PROCESS_SHAPE_6                0xc0

#define COLOR_DETECT_REG_PROCESS_SHAPE_7                0xc4

#define COLOR_DETECT_REG_PROCESS_SHAPE_8                0xc8

/* Address generator archi */

// Input stream - img_in (programmable)
#define COLOR_DETECT_REG_IMG_IN_TRANS_SIZE                  0xcc
#define COLOR_DETECT_REG_IMG_IN_LINE_STRIDE                 0xd0
#define COLOR_DETECT_REG_IMG_IN_LINE_LENGTH                 0xd4
#define COLOR_DETECT_REG_IMG_IN_FEAT_STRIDE                 0xd8
#define COLOR_DETECT_REG_IMG_IN_FEAT_LENGTH                 0xdc
#define COLOR_DETECT_REG_IMG_IN_FEAT_ROLL                   0xe0
#define COLOR_DETECT_REG_IMG_IN_LOOP_OUTER                  0xe4
#define COLOR_DETECT_REG_IMG_IN_REALIGN_TYPE                0xe8
#define COLOR_DETECT_REG_IMG_IN_STEP                        0xec

// Input stream - img_out (programmable)
#define COLOR_DETECT_REG_IMG_OUT_TRANS_SIZE                 0xf0
#define COLOR_DETECT_REG_IMG_OUT_LINE_STRIDE                0xf4
#define COLOR_DETECT_REG_IMG_OUT_LINE_LENGTH                0xf8
#define COLOR_DETECT_REG_IMG_OUT_FEAT_STRIDE                0xfc
#define COLOR_DETECT_REG_IMG_OUT_FEAT_LENGTH                0x100
#define COLOR_DETECT_REG_IMG_OUT_FEAT_ROLL                  0x104
#define COLOR_DETECT_REG_IMG_OUT_LOOP_OUTER                 0x108
#define COLOR_DETECT_REG_IMG_OUT_REALIGN_TYPE               0x10c
#define COLOR_DETECT_REG_IMG_OUT_STEP                       0x110

#endif
