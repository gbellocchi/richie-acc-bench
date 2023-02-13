
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

#ifndef __HAL_HWPE_CL0_LIC0_H__
#define __HAL_HWPE_CL0_LIC0_H__

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
 * ================================================================================
 *
 */

#define HWPE_ADDR_BASE ARCHI_FC_HWPE_ADDR
#define HWPE_ADDR_SPACE 0x00000100

// For all the following functions we use __builtin_pulp_OffsetedWrite and __builtin_pulp_OffsetedRead
// instead of classic load/store because otherwise the compiler is not able to correctly factorize
// the HWPE base in case several accesses are done, ending up with twice more code

// #define HWPE_WRITE(value, offset) *(volatile int *)(ARCHI_HWPE_ADDR_BASE + offset) = value
// #define HWPE_READ(offset) *(volatile int *)(ARCHI_HWPE_ADDR_BASE + offset)

#define hwpe_write32(add, val_) (*(volatile unsigned int *)(long)(add) = val_)

static inline uint32_t hwpe_read32(uint32_t add)
{
  __asm__ __volatile__ ("" : : : "memory");
  uint32_t result = *(volatile uint32_t *)add;
  asm volatile("nop;");
  __asm__ __volatile__ ("" : : : "memory");
  return result;
}

#define HWPE_WRITE(value, offset) hwpe_write32(ARCHI_HWPE_ADDR_BASE + (offset), (value))
#define HWPE_READ(offset) hwpe_read32(ARCHI_HWPE_ADDR_BASE + (offset))

/* uloop hal */

static inline void hwpe_nb_iter_set(unsigned int value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_NB_ITER);
}

static inline void hwpe_linestride_set(unsigned int value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_LINESTRIDE);
}

static inline void hwpe_tilestride_set(unsigned int value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_TILESTRIDE);
}

static inline void hwpe_len_iter_set_img_out(unsigned int value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_CNT_LIMIT_IMG_OUT);
}

/* custom hal */
static inline void hwpe_rows_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_ROWS );
}
static inline void hwpe_cols_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_COLS );
}
static inline void hwpe_low_thresh_0_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_LOW_THRESH_0 );
}
static inline void hwpe_low_thresh_1_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_LOW_THRESH_1 );
}
static inline void hwpe_low_thresh_2_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_LOW_THRESH_2 );
}
static inline void hwpe_low_thresh_3_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_LOW_THRESH_3 );
}
static inline void hwpe_low_thresh_4_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_LOW_THRESH_4 );
}
static inline void hwpe_low_thresh_5_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_LOW_THRESH_5 );
}
static inline void hwpe_low_thresh_6_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_LOW_THRESH_6 );
}
static inline void hwpe_low_thresh_7_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_LOW_THRESH_7 );
}
static inline void hwpe_low_thresh_8_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_LOW_THRESH_8 );
}
static inline void hwpe_high_thresh_0_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_HIGH_THRESH_0 );
}
static inline void hwpe_high_thresh_1_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_HIGH_THRESH_1 );
}
static inline void hwpe_high_thresh_2_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_HIGH_THRESH_2 );
}
static inline void hwpe_high_thresh_3_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_HIGH_THRESH_3 );
}
static inline void hwpe_high_thresh_4_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_HIGH_THRESH_4 );
}
static inline void hwpe_high_thresh_5_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_HIGH_THRESH_5 );
}
static inline void hwpe_high_thresh_6_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_HIGH_THRESH_6 );
}
static inline void hwpe_high_thresh_7_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_HIGH_THRESH_7 );
}
static inline void hwpe_high_thresh_8_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_HIGH_THRESH_8 );
}
static inline void hwpe_process_shape_0_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_PROCESS_SHAPE_0 );
}
static inline void hwpe_process_shape_1_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_PROCESS_SHAPE_1 );
}
static inline void hwpe_process_shape_2_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_PROCESS_SHAPE_2 );
}
static inline void hwpe_process_shape_3_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_PROCESS_SHAPE_3 );
}
static inline void hwpe_process_shape_4_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_PROCESS_SHAPE_4 );
}
static inline void hwpe_process_shape_5_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_PROCESS_SHAPE_5 );
}
static inline void hwpe_process_shape_6_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_PROCESS_SHAPE_6 );
}
static inline void hwpe_process_shape_7_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_PROCESS_SHAPE_7 );
}
static inline void hwpe_process_shape_8_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_PROCESS_SHAPE_8 );
}

/* address generator hal - inputs */
static inline void hwpe_addr_gen_img_in(
  unsigned int img_in_trans_size,
  unsigned int img_in_line_stride,
  unsigned int img_in_line_length,
  unsigned int img_in_feat_stride,
  unsigned int img_in_feat_length,
  unsigned int img_in_feat_roll,
  unsigned int img_in_loop_outer,
  unsigned int img_in_realign_type,
  unsigned int img_in_step)
{
  HWPE_WRITE(img_in_trans_size,    COLOR_DETECT_REG_IMG_IN_TRANS_SIZE  );
  HWPE_WRITE(img_in_line_stride,   COLOR_DETECT_REG_IMG_IN_LINE_STRIDE );
  HWPE_WRITE(img_in_line_length,   COLOR_DETECT_REG_IMG_IN_LINE_LENGTH );
  HWPE_WRITE(img_in_feat_stride,   COLOR_DETECT_REG_IMG_IN_FEAT_STRIDE );
  HWPE_WRITE(img_in_feat_length,   COLOR_DETECT_REG_IMG_IN_FEAT_LENGTH );
  HWPE_WRITE(img_in_feat_roll,     COLOR_DETECT_REG_IMG_IN_FEAT_ROLL   );
  HWPE_WRITE(img_in_loop_outer,    COLOR_DETECT_REG_IMG_IN_LOOP_OUTER  );
  HWPE_WRITE(img_in_realign_type,  COLOR_DETECT_REG_IMG_IN_REALIGN_TYPE);
  HWPE_WRITE(img_in_step,          COLOR_DETECT_REG_IMG_IN_STEP        );
}

/* address generator hal - outputs */
static inline void hwpe_addr_gen_img_out(
  unsigned int  img_out_trans_size,
  unsigned int  img_out_line_stride,
  unsigned int  img_out_line_length,
  unsigned int  img_out_feat_stride,
  unsigned int  img_out_feat_length,
  unsigned int  img_out_feat_roll,
  unsigned int  img_out_loop_outer,
  unsigned int  img_out_realign_type,
  unsigned int  img_out_step)
{
  HWPE_WRITE(img_out_trans_size,    COLOR_DETECT_REG_IMG_OUT_TRANS_SIZE  );
  HWPE_WRITE(img_out_line_stride,   COLOR_DETECT_REG_IMG_OUT_LINE_STRIDE );
  HWPE_WRITE(img_out_line_length,   COLOR_DETECT_REG_IMG_OUT_LINE_LENGTH );
  HWPE_WRITE(img_out_feat_stride,   COLOR_DETECT_REG_IMG_OUT_FEAT_STRIDE );
  HWPE_WRITE(img_out_feat_length,   COLOR_DETECT_REG_IMG_OUT_FEAT_LENGTH );
  HWPE_WRITE(img_out_feat_roll,     COLOR_DETECT_REG_IMG_OUT_FEAT_ROLL   );
  HWPE_WRITE(img_out_loop_outer,    COLOR_DETECT_REG_IMG_OUT_LOOP_OUTER  );
  HWPE_WRITE(img_out_realign_type,  COLOR_DETECT_REG_IMG_OUT_REALIGN_TYPE);
  HWPE_WRITE(img_out_step,          COLOR_DETECT_REG_IMG_OUT_STEP        );
}

/* basic hal */

static inline void hwpe_trigger_job() {
  HWPE_WRITE(0, COLOR_DETECT_REG_TRIGGER);
}

static inline int hwpe_acquire_job() {
  return HWPE_READ(COLOR_DETECT_REG_ACQUIRE);
}

static inline int hwpe_get_finished() {
  return HWPE_READ(COLOR_DETECT_REG_FINISHED);
}

static inline unsigned int hwpe_get_status() {
  return HWPE_READ(COLOR_DETECT_REG_STATUS);
}

static inline void hwpe_soft_clear() {
  HWPE_WRITE(0, COLOR_DETECT_REG_SOFT_CLEAR);
}

static inline void hwpe_cg_enable() {
  return;
}

static inline void hwpe_cg_disable() {
  return;
}

static inline void hwpe_bytecode_set(unsigned int offs, unsigned int value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_BYTECODE+offs);
}

/* tcdm master port hal */

// input img_in
static inline void hwpe_img_in_addr_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_IMG_IN_ADDR);
}

// output img_out
static inline void hwpe_img_out_addr_set(int32_t value) {
  HWPE_WRITE(value, COLOR_DETECT_REG_IMG_OUT_ADDR);
}

#endif /* __HAL_HWPE_H__ */

