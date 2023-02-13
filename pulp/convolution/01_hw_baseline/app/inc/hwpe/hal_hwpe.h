
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
#ifndef __HAL_HWPE_H__
#define __HAL_HWPE_H__
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
 *     8  |  0x005c  |  31: 0  |  0xffffffff  ||  HWPE_DUMB
 * ================================================================================
 *
 */
/* LOW-LEVEL HAL */
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

static inline void hwpe_bytecode_set(unsigned int offs, unsigned int value) {
  HWPE_WRITE(value, HWPE_BYTECODE+offs);
}
// basic hal
static inline void hwpe_trigger_job() {
  HWPE_WRITE(0, HWPE_TRIGGER);
}
static inline int hwpe_acquire_job() {
  return HWPE_READ(HWPE_ACQUIRE);
}
static inline unsigned int hwpe_get_status() {
  return HWPE_READ(HWPE_STATUS);
}
static inline void hwpe_soft_clear() {
  volatile int i;
  HWPE_WRITE(0, HWPE_SOFT_CLEAR);
}
static inline void hwpe_cg_enable() {
  return;
}
static inline void hwpe_cg_disable() {
  return;
}

// TCDM address regs
static inline void hwpe_src_addr_set(int32_t ptr_buff_A, int32_t ptr_buff_B) {
  HWPE_WRITE(ptr_buff_A, HWPE_SRC_BUFFER_A_ADDR);
  HWPE_WRITE(ptr_buff_B, HWPE_SRC_BUFFER_B_ADDR);
}
static inline void hwpe_dst_addr_set(int32_t ptr_buff_A, int32_t ptr_buff_B) {
  HWPE_WRITE(ptr_buff_A, HWPE_DST_BUFFER_A_ADDR);
  HWPE_WRITE(ptr_buff_B, HWPE_DST_BUFFER_B_ADDR);
}

// Overlay flags
static inline void hwpe_src_buffer_ready() {
  HWPE_WRITE(1, HWPE_SRC_BUFFER_READY);
}

static inline void hwpe_dst_buffer_ready() {
  HWPE_WRITE(1, HWPE_DST_BUFFER_READY);
}

// ULOOP
static inline void hwpe_nb_iter_set(unsigned int value) {
  HWPE_WRITE(value, HWPE_NB_ITER);
}
static inline void hwpe_linestride_set(unsigned int value) {
  HWPE_WRITE(value, HWPE_LINESTRIDE);
}
static inline void hwpe_tilestride_set(unsigned int value) {
  HWPE_WRITE(value, HWPE_TILESTRIDE);
}

// FSM
static inline void hwpe_len_iter_set(unsigned int value) {
  HWPE_WRITE(value, HWPE_CNT_LIMIT);
}

// ADDRESS GENERATOR
static inline void hwpe_addr_gen_src(
  unsigned int trans_size,
  unsigned int tile_size,
  unsigned int line_stride,
  unsigned int line_length,
  unsigned int feat_stride,
  unsigned int feat_length,
  unsigned int feat_roll,
  unsigned int loop_outer,
  unsigned int realign_type,
  unsigned int buffer_select)
{
  HWPE_WRITE(trans_size,    HWPE_SRC_TRANS_SIZE   );
  HWPE_WRITE(tile_size,     HWPE_SRC_TILE_SIZE    );
  HWPE_WRITE(line_stride,   HWPE_SRC_LINE_STRIDE  );
  HWPE_WRITE(line_length,   HWPE_SRC_LINE_LENGTH  );
  HWPE_WRITE(feat_stride,   HWPE_SRC_FEAT_STRIDE  );
  HWPE_WRITE(feat_length,   HWPE_SRC_FEAT_LENGTH  );
  HWPE_WRITE(feat_roll,     HWPE_SRC_FEAT_ROLL    );
  HWPE_WRITE(loop_outer,    HWPE_SRC_LOOP_OUTER   );
  HWPE_WRITE(realign_type,  HWPE_SRC_REALIGN_TYPE );
  HWPE_WRITE(buffer_select, HWPE_SRC_BUFFER_SELECT);
}

static inline void hwpe_addr_gen_dst(
  unsigned int trans_size,
  unsigned int tile_size,
  unsigned int line_stride,
  unsigned int line_length,
  unsigned int feat_stride,
  unsigned int feat_length,
  unsigned int feat_roll,
  unsigned int loop_outer,
  unsigned int realign_type,
  unsigned int buffer_select)
{
  HWPE_WRITE(trans_size,    HWPE_DST_TRANS_SIZE   );
  HWPE_WRITE(tile_size,     HWPE_DST_TILE_SIZE    );
  HWPE_WRITE(line_stride,   HWPE_DST_LINE_STRIDE  );
  HWPE_WRITE(line_length,   HWPE_DST_LINE_LENGTH  );
  HWPE_WRITE(feat_stride,   HWPE_DST_FEAT_STRIDE  );
  HWPE_WRITE(feat_length,   HWPE_DST_FEAT_LENGTH  );
  HWPE_WRITE(feat_roll,     HWPE_DST_FEAT_ROLL    );
  HWPE_WRITE(loop_outer,    HWPE_DST_LOOP_OUTER   );
  HWPE_WRITE(realign_type,  HWPE_DST_REALIGN_TYPE );
  HWPE_WRITE(buffer_select, HWPE_DST_BUFFER_SELECT);
}

// custom hal
static inline void hwpe_width_set(int32_t value) {
  HWPE_WRITE(value, HWPE_WIDTH );
}
static inline void hwpe_height_set(int32_t value) {
  HWPE_WRITE(value, HWPE_HEIGHT );
}
static inline void hwpe_filter_coeffs_0_set(int32_t value) {
  HWPE_WRITE(value, HWPE_FILTER_COEFFS_0 );
}
static inline void hwpe_filter_coeffs_1_set(int32_t value) {
  HWPE_WRITE(value, HWPE_FILTER_COEFFS_1 );
}
static inline void hwpe_filter_coeffs_2_set(int32_t value) {
  HWPE_WRITE(value, HWPE_FILTER_COEFFS_2 );
}
static inline void hwpe_filter_coeffs_3_set(int32_t value) {
  HWPE_WRITE(value, HWPE_FILTER_COEFFS_3 );
}
static inline void hwpe_filter_coeffs_4_set(int32_t value) {
  HWPE_WRITE(value, HWPE_FILTER_COEFFS_4 );
}
static inline void hwpe_filter_coeffs_5_set(int32_t value) {
  HWPE_WRITE(value, HWPE_FILTER_COEFFS_5 );
}
static inline void hwpe_filter_coeffs_6_set(int32_t value) {
  HWPE_WRITE(value, HWPE_FILTER_COEFFS_6 );
}
static inline void hwpe_filter_coeffs_7_set(int32_t value) {
  HWPE_WRITE(value, HWPE_FILTER_COEFFS_7 );
}
static inline void hwpe_filter_coeffs_8_set(int32_t value) {
  HWPE_WRITE(value, HWPE_FILTER_COEFFS_8 );
}
static inline void hwpe_filter_coeffs_9_set(int32_t value) {
  HWPE_WRITE(value, HWPE_FILTER_COEFFS_9 );
}
static inline void hwpe_filter_coeffs_10_set(int32_t value) {
  HWPE_WRITE(value, HWPE_FILTER_COEFFS_10 );
}

#endif /* __HAL_HWPE_H__ */