/*
 * Copyright 2019 ETH Zurich, University of Bologna
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

/* Libraries. */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>

/* Include hero-target library. */
#include <hero-target.h>

/* Include accelerator library. */
#include "inc/hwpe/archi_hwpe.h"
#include "inc/hwpe/hal_hwpe.h"

/* Include EU library. */
#include "inc/eu/archi_eu_v3.h"
#include "inc/eu/hal_eu_eu_v3.h"

/* Include overlay-bench library. */
#include "../../common/overlay-bench.h"

// ---------------------- //

// #define RV_DEBUG

// Entire image dimension
#define MAX_IMG_ROWS 1080
#define MAX_IMG_COLS 1920

#define IM_UAV_ROWS 320
#define IM_UAV_COLS 320

#define UAV_FILTER_DIM 11

// Image block dimensions
#define IM_BLOCK_ROWS 16
#define IM_BLOCK_COLS 320
#define IM_BLOCK_SIZE (UAV_STRIPE_ROWS * UAV_STRIPE_COLS)

// EU events
#define EVENT_ENGINE    1
#define EVENT_SRC_TILE  2
#define EVENT_DST_TILE  4
#define EVENT_IO_TILE  6

/* Checksum. */

void check_result(
    uint32_t* test_res,
    uint32_t* golden_res, 
    unsigned width, unsigned height, unsigned stripe_height)
{
    uint32_t n_analyzed = 0;
    uint32_t n_errors = 0;
    uint32_t err_row = 0;
    uint32_t err_col = 0;

    loop_A: for (unsigned ii = 0; ii < width; ii++){
      loop_B: for (unsigned jj = 0; jj < height; jj++){
        if( test_res[ii * width + jj] != golden_res[ii * width + jj] ) { 
          n_errors++;
          if(n_errors==1) n_analyzed = ii * width + jj;
          if(n_errors==1) err_row = ii;
          if(n_errors==1) err_col = jj; 
        }
      }
    }

    if(n_errors == 0)
        printf("Checksum completed SUCCESFULLY!\n\n");
    else{ 
        printf("Number of data analyzed before first error: %d.\n", n_analyzed);
        printf("Number of errors: %d.\n", n_errors);
        printf("Total number of elements: %d.\n\n", width*height);
        printf("ERROR: Result mismatch in Row %u, Column %u!\n", err_row, err_col);
        printf("Tested result is %u.\n", test_res[err_row*width+err_col]);
        printf("Golden result is %u.\n\n", golden_res[err_row*width+err_col]);
    }
}

/* Golden result calculation. */

void convolution_sw(uint32_t* in, uint32_t* out, uint32_t coeffs[UAV_FILTER_DIM], uint32_t width, uint32_t height)
{

  const int conv_size = UAV_FILTER_DIM;

  uint32_t in_temp;
  uint32_t out_temp;
  uint32_t out_H[width*height];
  uint32_t out_V[width*height];

  const int border_width = (int)(conv_size / 2);

  // Horizontal convolution

  Clear_out_H:for(int i = 0; i < height * width; i++){
    out_H[i]=0;
  }

  HconvH: for(int col = 0; col < height; col++){
    HconvW: for(int row = border_width; row < width - border_width; row++){
      int pixel = col * width + row;
      Hconv: for(int i = - border_width; i <= border_width; i++){
        out_H[pixel] += in[pixel + i] * coeffs[i + border_width];
      }
    }
  }

  // Vertical convolution

  Clear_out_V:for(int i = 0; i < height * width; i++){
    out_V[i]=0;
  }

  VconvH: for(int col = border_width; col < height - border_width; col++){
    VconvW: for(int row = 0; row < width; row++){
      int pixel = col * width + row;
      Vconv: for(int i = - border_width; i <= border_width; i++){
        int offset = i * width;
        out_sw[pixel] += out_H[pixel + offset] * coeffs[i + border_width];
      }
    }
  }

  // Border pixels

  int border_width_offset = border_width * width;
  int border_height_offset = (height - border_width - 1) * width;

  // Border pixels

  Top_Border:for(int col = 0; col < border_width; col++){
    int offset = col * width;
    Top_Left:for(int row = 0; row < border_width; row++){
      int pixel = offset + row;
      out_sw[pixel] = out_sw[border_width_offset + border_width];
    }
    Top_Row:for(int row = border_width; row < width - border_width; row++){
      int pixel = offset + row;
      out_sw[pixel] = out_sw[border_width_offset + row];
    }
    Top_Right:for(int row = width - border_width; row < width; row++){
      int pixel = offset + row;
      out_sw[pixel] = out_sw[border_width_offset + width - border_width - 1];
    }
  }

  Side_Border:for(int col = border_width; col < height - border_width; col++){
    int offset = col * width;
    for(int row = 0; row < border_width; row++){
      int pixel = offset + row;
      out_sw[pixel] = out_sw[offset + border_width];
    }
    for(int row = width - border_width; row < width; row++){
      int pixel = offset + row;
      out_sw[pixel] = out_sw[offset + width - border_width - 1];
    }
  }

  Bottom_Border:for(int col = height - border_width; col < height; col++){
    int offset = col * width;
    for(int row = 0; row < border_width; row++){
      int pixel = offset + row;
      out_sw[pixel] = out_sw[border_height_offset + border_width];
    }
    for(int row = border_width; row < width - border_width; row++){
      int pixel = offset + row;
      out_sw[pixel] = out_sw[border_height_offset + row];
    }
    for(int row = width - border_width; row < width; row++){
      int pixel = offset + row;
      out_sw[pixel] = out_sw[border_height_offset + width - border_width - 1];
    }
  }

}

/* - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / */

/*
 *
 *     PULP - Functions.
 *
 */

/* Event unit - Program mask. */

/* Event unit - Wait and decode accelerator events. */

eu_struct hwpe_eu_wait_decode(){

  eu_struct s;

  s.clk_0 = hero_get_clk_counter();

  int evt_to_wait = 0b111;
  int hwpe_evt_offset = 12;

  int32_t master_evt_line;
  int evt_ohd;

  master_evt_line = eu_evt_maskWaitAndClr(evt_to_wait << hwpe_evt_offset);

  // one-hot decoding
  // master_evt_line is coded with one-hot coding
  // - 001 -> engine
  // - 010 -> src_tile
  // - 100 -> dst_tile
  s.req_eu = master_evt_line >> hwpe_evt_offset;

  s.clk_1 = hero_get_clk_counter();

  return s;
}

/* DMA - Stripe transfer from L3 to L1. */

load_struct l3_to_l1(
  __host uint32_t* const ext_src, 
  __device uint32_t* const local_dst, 
  __host uint32_t* const dram_offset,
  const size_t width,
  const size_t height,
  const size_t stripe_height)
{
  load_struct s;

  s.clk_0 = hero_get_clk_counter();

  const size_t size = width * stripe_height * sizeof(uint32_t);

  uint64_t ext_addr = (uint64_t)ext_src + (uint64_t)dram_offset;

  uint32_t local_addr = (uint32_t)local_dst;

  hero_dma_job_t dma;

  dma = hero_memcpy_host2dev_async(local_addr, ext_addr, size);

  s.dma_job_in = dma;

  s.clk_1 = hero_get_clk_counter();

  return s;
}

/* DMA - Wait for input tx to conclude before computation. */

load_struct wait_for_dma_in(hero_dma_job_t dma_in_towait)
{
  load_struct s;
  s.clk_0 = hero_get_clk_counter();

  hero_dma_wait(dma_in_towait);
  hwpe_src_buffer_ready();

  s.clk_1 = hero_get_clk_counter();
  return s;
}

store_struct wait_for_dma_out(hero_dma_job_t dma_out_towait)
{
  store_struct s;
  s.clk_0 = hero_get_clk_counter();

  hero_dma_wait(dma_out_towait);
  hwpe_dst_buffer_ready();

  s.clk_1 = hero_get_clk_counter();
  return s;
}

/* DMA - Stripe transfer from L1 to L3. */

store_struct l1_to_l3(
  __device  uint32_t* const local_src, 
  __host    uint32_t* const ext_dst,
  __host    uint32_t* const out_dram_offset,
  const size_t width,
  const size_t height,
  const size_t stripe_height)
{

  store_struct s;

  s.clk_0 = hero_get_clk_counter();

  const size_t size = width * stripe_height * sizeof(uint32_t);

  uint64_t ext_out_addr = (uint64_t)ext_dst + (uint64_t)out_dram_offset;
  uint32_t local_out_addr = (uint32_t)local_src;

  hero_dma_job_t dma;

  dma = hero_memcpy_dev2host_async( ext_out_addr, local_out_addr, size);
  
  s.dma_job_out = dma;

  s.clk_1 = hero_get_clk_counter();

  return s;
}

/* HWPE - Programming. */

void hwpe_buffer_set(
  __device uint32_t* const buffer0_in,
  __device uint32_t* const buffer1_in,
  __device uint32_t* const buffer0_out,
  __device uint32_t* const buffer1_out
)
{
  hwpe_src_addr_set( buffer0_in, buffer1_in);
  hwpe_dst_addr_set( buffer0_out, buffer1_out);
}

//

hwpe_progr_struct hwpe_programming(
  hwpe_addr_gen_struct addr_gen_in,
  hwpe_addr_gen_struct addr_gen_out,
  __device uint32_t* buffer0_in, 
  __device uint32_t* buffer1_in, 
  __device uint32_t* buffer0_out,
  __device uint32_t* buffer1_out,
  uint32_t filter_coeffs[UAV_FILTER_DIM],
  const size_t width,
  const size_t height,
  const size_t stripe_height)
{
  hwpe_progr_struct s;

  s.clk_0 = hero_get_clk_counter();

  const unsigned stripe_out_size = stripe_height * stripe_height;
  int offload_id_tmp, offload_id;

  hwpe_cg_enable();

  while((offload_id_tmp = hwpe_acquire_job()) < 0);

  /* Micro-code processor */

#ifdef HWPE_UCODE_PROGRAMMING

  /* This benchmark does not make use of the ucode processor. */

  /* Set up bytecode. */

  hwpe_bytecode_set(HWPE_LOOPS1_OFFS,           0x00000000);
  hwpe_bytecode_set(HWPE_BYTECODE5_LOOPS0_OFFS, 0x00000000);
  hwpe_bytecode_set(HWPE_BYTECODE4_OFFS,        0x00000000);
  hwpe_bytecode_set(HWPE_BYTECODE3_OFFS,        0x00000000);
  hwpe_bytecode_set(HWPE_BYTECODE2_OFFS,        0x00000000);
  hwpe_bytecode_set(HWPE_BYTECODE1_OFFS,        0x00000808);
  hwpe_bytecode_set(HWPE_BYTECODE0_OFFS,        0x09e22c24);

  /* Ucode parameters. */

  hwpe_nb_iter_set(stripe_height);
  hwpe_linestride_set(width * sizeof(uint32_t));
  hwpe_tilestride_set(stripe_height * sizeof(uint32_t));
  
#endif

  /* FSM. */

  hwpe_len_iter_set(stripe_out_size - 1); 

  /* Buffer pointers. */

  hwpe_buffer_set(buffer0_in, buffer1_in, buffer0_out, buffer1_out);

  /* Address generator. */

  hwpe_addr_gen_src(
    addr_gen_in.trans_size,
    addr_gen_in.tile_size,
    addr_gen_in.line_stride,
    addr_gen_in.line_length,
    addr_gen_in.feat_stride,
    addr_gen_in.feat_length,
    addr_gen_in.feat_roll,
    addr_gen_in.loop_outer,
    addr_gen_in.realign_type,
    addr_gen_in.buffer_select
  );

  hwpe_addr_gen_dst(
    addr_gen_out.trans_size,
    addr_gen_out.tile_size,
    addr_gen_out.line_stride,
    addr_gen_out.line_length,
    addr_gen_out.feat_stride,
    addr_gen_out.feat_length,
    addr_gen_out.feat_roll,
    addr_gen_out.loop_outer,
    addr_gen_out.realign_type,
    addr_gen_out.buffer_select
  );

  /* Custom registers. */

  hwpe_width_set( width );
  hwpe_height_set( height );

  hwpe_filter_coeffs_0_set( filter_coeffs[0] );
  hwpe_filter_coeffs_1_set( filter_coeffs[1] );
  hwpe_filter_coeffs_2_set( filter_coeffs[2] );
  hwpe_filter_coeffs_3_set( filter_coeffs[3] );
  hwpe_filter_coeffs_4_set( filter_coeffs[4] );
  hwpe_filter_coeffs_5_set( filter_coeffs[5] );
  hwpe_filter_coeffs_6_set( filter_coeffs[6] );
  hwpe_filter_coeffs_7_set( filter_coeffs[7] );
  hwpe_filter_coeffs_8_set( filter_coeffs[8] );
  hwpe_filter_coeffs_9_set( filter_coeffs[9] );
  hwpe_filter_coeffs_10_set( filter_coeffs[10] );

  s.clk_1 = hero_get_clk_counter();

  return s;
}

/* HWPE - Matrix multiplication on data stripe. */

compute_struct compute()
{
  load_struct f;
  compute_struct s;

  s.clk_0 = hero_get_clk_counter();

  /* Trigger accelerator execution. */

  hwpe_trigger_job();

  /* Wait for the accelerator to generate an event. */

  // eu_evt_maskWaitAndClr(1 << ARCHI_CL_EVT_ACC0);

  s.clk_1 = hero_get_clk_counter();

  return s;
}

/* RISCV - Matrix multiplication on data stripe. */

compute_struct compute_RV(
  __device uint32_t* const in,
  __device uint32_t* const out,
  const size_t width,
  const size_t stripe_height)
{
  compute_struct s;

  s.clk_0 = hero_get_clk_counter();

  unsigned sum = 0;

  // compute_0: for (unsigned i = 0; i < stripe_height; i++){
  //   compute_1: for (unsigned j = 0; j < stripe_height; j++){
  //     sum = 0;
  //     compute_2: for (unsigned k = 0; k < width; k++){
  //       sum += in1[i * width + k] * in2[j * width + k];
  //     }
  //     out[i * width + j] = sum;
  //   }
  // }

  s.clk_1 = hero_get_clk_counter();

  return s;
}

/* - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / */

/*
 *
 *     FPGA overlay - Offloaded program.
 *
 */

#pragma omp declare target

int convolution_hw(
  __host uint32_t * __restrict__ l3_in,
  __host uint32_t * __restrict__ l3_out,
  __host uint32_t * __restrict__ l3_golden,
  uint32_t filter_coeffs[UAV_FILTER_DIM],
  uint32_t width, uint32_t height, uint32_t stripe_height, uint32_t clk_counter[10])
{
  if (hero_rt_core_id() == 0){

    /* Cycle counters. */
    
    uint32_t t_pulp_0,  t_pulp_1;  
    uint32_t t_clean_0, t_clean_1; 
    uint32_t t_alloc_0, t_alloc_1;
    uint32_t t_dma_in     = 0;
    uint32_t t_hwpe_progr = 0;
    uint32_t t_proc       = 0;
    uint32_t t_dma_out    = 0;
    uint32_t t_eu_progr   = 0;

    /* Measure counter call overhead. */

    uint32_t t_overhead_0, t_overhead_1; 

    t_overhead_0  = hero_get_clk_counter();
    t_pulp_0      = hero_get_clk_counter();
    t_overhead_1  = hero_get_clk_counter();

    /* ------------------------------------------------------------------------------- */

    t_pulp_0 = hero_get_clk_counter();

    t_alloc_0 = hero_get_clk_counter();

    /* DRAM offsets. */

    uint32_t in_dram_offset;
    uint32_t out_dram_offset;

    /* Output structs. */

    load_struct         fetch;
    store_struct        store;
    load_struct         wait_for_in;
    store_struct        wait_for_out;
    hwpe_progr_struct   hw_progr;
    eu_struct           eu_progr;
    compute_struct      comp;

    /* Double buffering variables. */

    uint8_t sel_in;
    uint8_t sel_out;

    /* DMA jobs. */

    hero_dma_job_t dma_buffer0_in;
    hero_dma_job_t dma_buffer1_in;
    hero_dma_job_t dma_buffer0_out;
    hero_dma_job_t dma_buffer1_out;

    /* Event unit. */

    int eoc;

    /* Hardware accelerator. */

    hwpe_addr_gen_struct addr_gen_in;
    hwpe_addr_gen_struct addr_gen_out;

    /* Hardware accelerator - Address generator. */
    
    addr_gen_in.trans_size      = width * height;
    addr_gen_in.tile_size       = width * stripe_height;
    addr_gen_in.line_stride     = width * sizeof(uint32_t);
    addr_gen_in.line_length     = width;
    addr_gen_in.feat_stride     = 0;
    addr_gen_in.feat_length     = stripe_height;
    addr_gen_in.feat_roll       = 0;
    addr_gen_in.loop_outer      = 0;
    addr_gen_in.realign_type    = 0; 
    addr_gen_in.buffer_select   = 0;

    addr_gen_out.trans_size     = width * height;
    addr_gen_out.tile_size      = width * stripe_height;
    addr_gen_out.line_stride    = width * sizeof(uint32_t);
    addr_gen_out.line_length    = width;
    addr_gen_out.feat_stride    = 0;
    addr_gen_out.feat_length    = stripe_height;
    addr_gen_out.feat_roll      = 0;
    addr_gen_out.loop_outer     = 0;
    addr_gen_out.realign_type   = 0; 
    addr_gen_out.buffer_select  = 0;

    /* Allocate L1 with single buffers for each stripe data type. */

    __device uint32_t* l1_buffer0_in    = (__device uint32_t *)hero_l1malloc(width * stripe_height * sizeof(uint32_t));
    __device uint32_t* l1_buffer1_in    = (__device uint32_t *)hero_l1malloc(width * stripe_height * sizeof(uint32_t));
    __device uint32_t* l1_buffer0_out   = (__device uint32_t *)hero_l1malloc(width * stripe_height * sizeof(uint32_t));
    __device uint32_t* l1_buffer1_out   = (__device uint32_t *)hero_l1malloc(width * stripe_height * sizeof(uint32_t));

    if ((l1_buffer0_in == NULL) || (l1_buffer1_in == NULL)) {
      printf("ERROR 0: Memory allocation failed!\n");
      return 1;
    }

    if ((l1_buffer0_out == NULL) || (l1_buffer1_out == NULL)) {
      printf("ERROR 1: Memory allocation failed!\n");
      return 1;
    }

    /* Initialize DRAM offsets. */

    in_dram_offset = 0;
    out_dram_offset = 0;

    /* Initialize double buffering variables. */

    sel_in = 0;
    sel_out = 0;

    t_alloc_1 = hero_get_clk_counter();

    /* First in read from DRAM to Buffer #0. */

    fetch = l3_to_l1(l3_in, l1_buffer0_in, in_dram_offset, width, height, stripe_height);
    dma_buffer0_in = fetch.dma_job_in;
    t_dma_in += fetch.clk_1 - fetch.clk_0;

    /* Initialize accelerator and program address generator. */

    hw_progr = hwpe_programming(
      addr_gen_in, addr_gen_out, 
      l1_buffer0_in, l1_buffer1_in,
      l1_buffer0_out, l1_buffer1_out,
      filter_coeffs, width, height, stripe_height
    );
    t_hwpe_progr += hw_progr.clk_1 - hw_progr.clk_0;

    /* Convolution - Hardware acceleration. */
    
    // Transfer first input tile
    wait_for_in = wait_for_dma_in(dma_buffer0_in);

    // Initial condition
    sel_in = !sel_in;
    eoc = 0;
    hwpe_dst_buffer_ready();
    
    // Start execution
    comp = compute();

    t_dma_in += wait_for_in.clk_1 - wait_for_in.clk_0;
    t_proc += comp.clk_1 - comp.clk_0;

    // Data management policy
    // Double buffering implemented in strict 
    // collaboration with the hardware wrapper

    while(!eoc){

      // Wait and decode accelerator event
      eu_progr = hwpe_eu_wait_decode();
      t_eu_progr += eu_progr.clk_1 - eu_progr.clk_0;

      // Respond to accelerator event, then re-program eu till the end of computation

      // End of computation
      if (eu_progr.req_eu == EVENT_ENGINE){
        eoc = 1;
      }

      // Buffer load
      if (eu_progr.req_eu == EVENT_SRC_TILE){

        in_dram_offset += stripe_height * width * sizeof(uint32_t);

        if(!sel_in){

          fetch = l3_to_l1(l3_in, l1_buffer0_in, in_dram_offset, width, height, stripe_height);
          dma_buffer0_in = fetch.dma_job_in;
          t_dma_in += fetch.clk_1 - fetch.clk_0;

          wait_for_in = wait_for_dma_in(dma_buffer0_in);
          t_dma_in += wait_for_in.clk_1 - wait_for_in.clk_0;

          hwpe_src_buffer_ready();

        } else {

          fetch = l3_to_l1(l3_in, l1_buffer1_in, in_dram_offset, width, height, stripe_height);
          dma_buffer1_in = fetch.dma_job_in;
          t_dma_in += fetch.clk_1 - fetch.clk_0;

          wait_for_in = wait_for_dma_in(dma_buffer1_in);
          t_dma_in += wait_for_in.clk_1 - wait_for_in.clk_0;

          hwpe_src_buffer_ready();

        }

        sel_in = !sel_in;

      }

      // Buffer store
      if (eu_progr.req_eu == EVENT_DST_TILE){  

        eoc = 1;

        if(!sel_out){

          store = l1_to_l3(l1_buffer0_out, l3_out, out_dram_offset, width, height, stripe_height);
          dma_buffer0_out = store.dma_job_out;
          t_dma_out += store.clk_1 - store.clk_0;

          wait_for_out = wait_for_dma_out(dma_buffer0_out);
          t_dma_out += wait_for_out.clk_1 - wait_for_out.clk_0;

          hwpe_dst_buffer_ready();

        } else {

          store = l1_to_l3(l1_buffer1_out, l3_out, out_dram_offset, width, height, stripe_height);
          dma_buffer1_out = store.dma_job_out;
          t_dma_out += store.clk_1 - store.clk_0;

          wait_for_out = wait_for_dma_out(dma_buffer1_out);
          t_dma_out += wait_for_out.clk_1 - wait_for_out.clk_0;

          hwpe_dst_buffer_ready();

        }

        out_dram_offset += stripe_height * width * sizeof(uint32_t);
        sel_out = !sel_out;

      }

      // Buffer load/store
      if (eu_progr.req_eu == EVENT_IO_TILE){

        eoc = 1;
        
        in_dram_offset += stripe_height * width * sizeof(uint32_t);

        if(!sel_in){

          fetch = l3_to_l1(l3_in, l1_buffer0_in, in_dram_offset, width, height, stripe_height);
          dma_buffer0_in = fetch.dma_job_in;
          t_dma_in += fetch.clk_1 - fetch.clk_0;

          wait_for_in = wait_for_dma_in(dma_buffer0_in);
          t_dma_in += wait_for_in.clk_1 - wait_for_in.clk_0;

          hwpe_src_buffer_ready();

        } else {

          fetch = l3_to_l1(l3_in, l1_buffer1_in, in_dram_offset, width, height, stripe_height);
          dma_buffer1_in = fetch.dma_job_in;
          t_dma_in += fetch.clk_1 - fetch.clk_0;

          wait_for_in = wait_for_dma_in(dma_buffer1_in);
          t_dma_in += wait_for_in.clk_1 - wait_for_in.clk_0;

          hwpe_src_buffer_ready();

        }

        sel_in = !sel_in;

        //

        if(!sel_out){

          store = l1_to_l3(l1_buffer0_out, l3_out, out_dram_offset, width, height, stripe_height);
          dma_buffer0_out = store.dma_job_out;
          t_dma_out += store.clk_1 - store.clk_0;

          wait_for_out = wait_for_dma_out(dma_buffer0_out);
          t_dma_out += wait_for_out.clk_1 - wait_for_out.clk_0;

          hwpe_dst_buffer_ready();

        } else {

          store = l1_to_l3(l1_buffer1_out, l3_out, out_dram_offset, width, height, stripe_height);
          dma_buffer1_out = store.dma_job_out;
          t_dma_out += store.clk_1 - store.clk_0;

          wait_for_out = wait_for_dma_out(dma_buffer1_out);
          t_dma_out += wait_for_out.clk_1 - wait_for_out.clk_0;

          hwpe_dst_buffer_ready();

        }

        out_dram_offset += stripe_height * width * sizeof(uint32_t);
        sel_out = !sel_out;

      }
    }

    // wait_for_dma_out(dma_buffer0_out);


    // /* Stripe prefetching is implemented in software using loop_A and loop_B. */

    // loop_convolution: for(int ii = 0; ii < width; ii += stripe_height){

    //   /* Swap in buffers. */

    //   sel_in = !sel_in;

    //   /* Update in DRAM offset. */

    //   in_dram_offset = (ii + stripe_height) * width * sizeof(uint32_t);

    //   /* 
    //    * sel_in is the selector of the prefetcher target.
    //    * If "sel_in" is 0, then the buffer #0 of the corresponding input will be prefetched.
    //    * 
    //    * Example: sel_in = 0 --> fetch = l3_to_l1(l3_in, l1_buffer0_in, ..);
    //    */

    //   if(!sel_in){

    //     /* 
    //      * Transfer in stripe from L3 to L1 using DMA. 
    //      * 
    //      * Example: sel_in = 0 --> l1_buffer0_in is targeted for prefetching.
    //      */

    //     fetch = l3_to_l1(l3_in, l1_buffer0_in, in_dram_offset, width, height, stripe_height);
      //   dma_buffer0_in = fetch.dma_job_in;
      //   t_dma_in += fetch.clk_1 - fetch.clk_0;

      // } else{

      //   /* Transfer in stripe from L3 to L1 using DMA. */

      //   fetch = l3_to_l1(l3_in, l1_buffer1_in, in_dram_offset, width, height, stripe_height);
      //   dma_buffer1_in = fetch.dma_job_in;
      //   t_dma_in += fetch.clk_1 - fetch.clk_0;

      // }

      // /* 
      //   * Stripe processing. 
      //   * 
      //   * When it comes to stripe processing, sel_in is used to select the buffer 
      //   * that has been loaded with input data at the precedent iteration.
      //   * Before the computation to start, core wait for the end of the DMA
      //   * transactions concerning the buffers containing data to be processed.
      //   * 
      //   * Example: sel_in = 0 --> Data have been loaded in l1_buffer1_in during 
//         *                          the precedent iteration, so the data of this buffer
//         *                          are going to be processed.
//         */

//       if(!sel_in){
// #ifndef RV_DEBUG
//         // l1_buffer_out_offset = cnt_out_tx * stripe_height; 
//         wait_for_in = wait_for_dma(dma_buffer1_in);
//         comp = compute();
// #else
//         compute_RV(l1_buffer1_in, l1_buffer1_in, l1_buffer1_out + l1_buffer_out_offset, width, stripe_height); 
//         l1_buffer_out_offset = jj;
// #endif

//       } else {
// #ifndef RV_DEBUG
//         // l1_buffer_out_offset = cnt_out_tx * stripe_height;
//         wait_for_in = wait_for_dma(dma_buffer0_in);
//         comp = compute();
// #else
//         compute_RV(l1_buffer0_in, l1_buffer0_out + l1_buffer_out_offset, width, stripe_height); 
//         l1_buffer_out_offset = jj;
// #endif
//       }

//       t_dma_in += wait_for_in.clk_1 - wait_for_in.clk_0;
    //   t_proc += comp.clk_1 - comp.clk_0;

    //   /* - / - / - / - / - / - / - / - / - / - / - / - / - / - / - */

    //   /* 
    //     * DMA transfer - L1 to L3.
    //     *  
    //     * The idea is to: 
    //     *    - Trigger the write operation for the last buffer that has been written;
    //     *    - At the same time, wait for the conclusion of the data transfer concerning
    //     *      the buffer that is going to be used for the next prefetch.
    //     * 
    //     * Example: sel_in = 0 --> Issue the data transfer from l1_buffer1_out to DRAM, while wait for the
    //     *                          conclusion of the one concerning l1_buffer0_out. The former is going to
    //     *                          used by HWPE during the next iteration.
    //     */

    //   if(!sel_in){

    //     store = l1_to_l3(l1_buffer1_out, l3_out, out_dram_offset, dma_buffer0_out, width, height, stripe_height);
    //     dma_buffer0_out = store.dma_job_out;
    //     t_dma_out += store.clk_1 - store.clk_0;

    //   } else {

    //     store = l1_to_l3(l1_buffer0_out, l3_out, out_dram_offset, dma_buffer1_out, width, height, stripe_height);
    //     dma_buffer0_out = store.dma_job_out;
    //     t_dma_out += store.clk_1 - store.clk_0;

    //   }

    //   /* Update out DRAM offset. */

    //   out_dram_offset += stripe_height * width * sizeof(uint32_t);

    // }

    /* Cleanup. */

    t_clean_0 = hero_get_clk_counter();

    hero_l1free(l1_buffer0_in);
    hero_l1free(l1_buffer0_out);
    hero_l1free(l1_buffer1_in);
    hero_l1free(l1_buffer1_out);

    t_clean_1 = hero_get_clk_counter();

    /* ------------------------------------------------------------------------------- */

    t_pulp_1 = hero_get_clk_counter();

    clk_counter[0] = t_pulp_1 - t_pulp_0;
    clk_counter[1] = t_alloc_1 - t_alloc_0;
    clk_counter[2] = t_dma_in;
    clk_counter[3] = t_hwpe_progr;
    clk_counter[4] = t_proc;
    clk_counter[5] = t_dma_out;
    clk_counter[6] = t_eu_progr;
    clk_counter[7] = t_clean_1 - t_clean_0;

    clk_counter[8] = t_overhead_1 - t_overhead_0;
    
  }

  return 0;
}

#pragma omp end declare target

/* - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / */

/*
 *
 *     HOST processor - Main program.
 *
 */

int main(int argc, char *argv[])
{
  printf("\n|-------------------|\n");
  printf("| Test - Beginning. |");
  printf("\n|-------------------|\n");

  /* Performance measurement. */

  timer_host t_alloc;
  timer_host t_fpga_offload;
  timer_host t_clean;
  uint32_t clk_counter[10];

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|----------------------------------------------------|\n");
  printf("| DRAM - Declaration, allocation and initialization. |");
  printf("\n|----------------------------------------------------|\n\n");

clock_gettime(CLOCK_REALTIME, &t_alloc.t0);

  /* Image dimension. */

  unsigned width          = IM_UAV_ROWS;
  unsigned height         = IM_UAV_COLS;
  unsigned stripe_height  = IM_BLOCK_ROWS;

  /* Algorithm parameters declaration. */
    
  const int chkr_size = 5;
  const unsigned max_pix_val = 255;
  const unsigned min_pix_val = 0;

  /* Filter components. */

  uint32_t filter_coeffs[UAV_FILTER_DIM] = {
    36, 111, 266, 498, 724, 821, 724, 498, 266, 111, 36
  };

  /* General. */

  int pulp_error;

  /* Allocate DRAM arrays. */

  uint32_t* l3_src = (__host int32_t*)hero_l3malloc(width*height*sizeof(int32_t));
  uint32_t* l3_dst = (__host int32_t*)hero_l3malloc(width*height*sizeof(int32_t)); 

  if ( (l3_src == NULL) || (l3_dst == NULL) ) {
    printf("ERROR: malloc() failed!\n");
    return -ENOMEM;
  }

clock_gettime(CLOCK_REALTIME, &t_alloc.t1);

t_alloc.t_meas = ((t_alloc.t1.tv_sec - t_alloc.t0.tv_sec) + (t_alloc.t1.tv_nsec - t_alloc.t0.tv_nsec)/1000000000.0)*1000.0;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* I/O arrays initialization. */

  // for(int i=0; i<width*height; i++){
  //   l3_src[i] = rand() % 255;
  // }

  for (int i = 0; i < IM_UAV_ROWS; i++) {
    unsigned chkr_pair_val[2];
    if ((i / chkr_size) % 2 == 0) {
      chkr_pair_val[0] = max_pix_val; chkr_pair_val[1] = min_pix_val;
    } else {
      chkr_pair_val[0] = min_pix_val; chkr_pair_val[1] = max_pix_val;
    }
    for (int j = 0; j < IM_UAV_COLS; j++) {
      unsigned pix_val = chkr_pair_val[(j / chkr_size) % 2];
      l3_src[i * IM_UAV_COLS + j] = pix_val;
    }
  }

  memset(l3_dst, 0, width * height * sizeof(uint32_t));

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Allocate and initialize golden results. */

  uint32_t* l3_golden   = (__host int32_t*)hero_l3malloc(width*height*sizeof(int32_t)); 

  if ( (l3_golden == NULL) ) {
    printf("ERROR: malloc() failed!\n");
    return -ENOMEM;
  }

  memset(l3_golden, 0, width * height * sizeof(uint32_t));

  /* Check L3 test/golden results have been correctly inizialized. */

  printf("Initialization checksum... ");
  check_result(l3_dst, l3_golden, width, height, stripe_height);

  /* Calculate golden results. */

  convolution_sw( l3_src, l3_golden, filter_coeffs, width, height);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Additional parameters. */

  const unsigned n_buffers_in         = 4; // Double buffering.
  const unsigned n_buffers_out        = 2; // Double buffering.
  const unsigned output_stripe_height = stripe_height + 5;
  const unsigned stripe_len_in        = width*stripe_height;
  const unsigned stripe_len_out       = width*output_stripe_height;
  const unsigned stripe_in_len_B      = stripe_len_in * sizeof(uint32_t);
  const float stripe_in_len_kB        = stripe_in_len_B / 1024.0;
  const unsigned stripe_out_len_B     = stripe_len_out * sizeof(uint32_t);
  const float stripe_out_len_kB       = stripe_out_len_B / 1024.0;

  /* TCDM size. */
                            /* This is defined in the hardware using SV macros.
                                Change this iff you change the hardware. */

  const float TCDM_size_B = 128000.0;
  const float TCDM_size_kB = 128.0;

  /* TCDM occupation. */
                            /* ( Input stripe * Number of input data ports ) + 
                                ( Ouput stripe * Number of output data ports ) 
                                  + Coeffs + Delta-Var 
                                    ~= ( Input stripe * Number of input data ports ) 
                                      + ( Output stripe * Number of output data ports ) */

  const float TCDM_occupation_perc = (float)(((stripe_in_len_B * n_buffers_in + stripe_out_len_B * n_buffers_out) / TCDM_size_B) * 100.0); 

  printf("Matrix multiplication parameters\n");
  printf("Width                 - %d        \n", width                );
  printf("Height                - %d        \n", height               );
  printf("Stripe_len in         - %d        \n", stripe_len_in        );
  printf("Stripe_len in  (B)    - %d B      \n", stripe_in_len_B      );
  printf("Stripe_len in  (kB)   - %.3f kB   \n", stripe_in_len_kB     );
  printf("Stripe_len out        - %d        \n", stripe_len_out       );
  printf("Stripe_len out (B)    - %d B      \n", stripe_out_len_B     );
  printf("Stripe_len out (kB)   - %.3f kB   \n", stripe_out_len_kB    );
  printf("TCDM occupation (%%)   - %.3f %%  \n", TCDM_occupation_perc );
  printf("TCDM size (kB)        - %.0f kB  \n\n", TCDM_size_kB);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|------------------------|\n");
  printf("| Execute CONVO on PULP. |");
  printf("\n|------------------------|\n\n");

  /* Warm-up. */

  unsigned tmp_1 = 1;
  unsigned tmp_2 = 2;
  #pragma omp target device(BIGPULP_MEMCPY) \
    map(to:         l3_src[0:width*height], width, stripe_height)                            \
    map(tofrom:     l3_dst[0:width*height], clk_counter[0:10])
  {
      tmp_2 = tmp_1;
  }
  tmp_1 = tmp_2;

clock_gettime(CLOCK_REALTIME, &t_fpga_offload.t0);

  /* Offloaded application. */

  #pragma omp target device(BIGPULP_MEMCPY) \
              map(to:         l3_src[0:width*height], filter_coeffs[0:UAV_FILTER_DIM], width, stripe_height)                            \
              map(from:       l3_dst[0:width*height], clk_counter[0:10]) 
  {
    pulp_error = convolution_hw(l3_src, l3_dst, l3_golden, filter_coeffs, width, height, stripe_height, clk_counter);
  }

clock_gettime(CLOCK_REALTIME, &t_fpga_offload.t1);

t_fpga_offload.t_meas = ((t_fpga_offload.t1.tv_sec - t_fpga_offload.t0.tv_sec) + (t_fpga_offload.t1.tv_nsec - t_fpga_offload.t0.tv_nsec)/1000000000.0)*1000.0;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|-----------|\n");
  printf("| Checksum. |");
  printf("\n|-----------|\n\n");

  if(pulp_error) return 1;

  /* Post-computation checksum. */

  printf("Post-computation checksum... ");
  check_result(l3_dst, l3_golden, width, height, stripe_height);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|---------|\n");
  printf("| Cleanup. |");
  printf("\n|---------|\n\n");

clock_gettime(CLOCK_REALTIME, &t_clean.t0);

  /* Cleanup. */  

  hero_l3free(l3_src);
  hero_l3free(l3_dst);
  hero_l3free(l3_golden);

clock_gettime(CLOCK_REALTIME, &t_clean.t1);

t_clean.t_meas = ((t_clean.t1.tv_sec - t_clean.t0.tv_sec) + (t_clean.t1.tv_nsec - t_clean.t0.tv_nsec)/1000000000.0)*1000.0;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Results - ARM measurements. */

  printf("\n|-----------------------------|\n");
  printf("| Results - ARM measurements. |");
  printf("\n|-----------------------------|\n");

  printf("\n  - I/O arrays allocation and initialization:\n");
  printf("  -     - Execution time (ms):    %.3f ms\n", t_alloc.t_meas );

  printf("\n  - Offloading + PULP application:\n");
  printf("  -     - Execution time (ms):    %.3f ms\n", t_fpga_offload.t_meas);

  printf("\n  - Cleaning:\n");
  printf("  -     - Execution time (ms):    %.3f ms\n", t_clean.t_meas );

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Results - PULP measurements. */

  printf("\n|------------------------------|\n");
  printf("| Results - PULP measurements. |");
  printf("\n|------------------------------|\n");

  /* PULP application execution. */

  const unsigned pulp_freq = 88 * 1000000;

  const unsigned long long  cycles_pulp           = clk_counter[0];
  const double              pulp_ms               = (double) (cycles_pulp * 1000) / pulp_freq;

  printf("\nPULP EXECUTION:\n");
  printf("  - Execution time (ck):    %llu ck\n",  cycles_pulp);
  printf("  - Execution time (ms):    %.3f ms\n",  pulp_ms);

  /* PULP application breakdown. */
  
  printf("\nAPPLICATION BREAKDOWN:\n");

  // Allocation.
  const unsigned long long  cycles_alloc     = clk_counter[1];
  const double              alloc_ms         = (double) (cycles_alloc * 1000) / pulp_freq;
  printf("\n  - Allocation.\n");
  printf("  -     - Execution time (ck):    %llu ck\n",  cycles_alloc);
  printf("  -     - Execution time (ms):    %.3f ms\n",  alloc_ms);

  // DMA - Input stripe.
  const unsigned long long  cycles_dma_stripe_in  = clk_counter[2];
  const double              dma_stripe_in_ms      = (double) (cycles_dma_stripe_in * 1000) / pulp_freq;
  printf("\n  - DMA - Input stripe.\n");
  printf("  -     - Execution time (ck):    %llu ck\n",  cycles_dma_stripe_in);
  printf("  -     - Execution time (ms):    %.3f ms\n",  dma_stripe_in_ms);

  // Accelerator programming.
  const unsigned long long  cycles_hwpe_progr     = clk_counter[3];
  const double              hwpe_progr_ms         = (double) (cycles_hwpe_progr * 1000) / pulp_freq;
  printf("\n  - Accelerator programming.\n");
  printf("  -     - Execution time (ck):    %llu ck\n",  cycles_hwpe_progr);
  printf("  -     - Execution time (ms):    %.3f ms\n",  hwpe_progr_ms);

  // Processing.
  const unsigned long long  cycles_processing     = clk_counter[4];
  const double              processing_ms         = (double) (cycles_processing * 1000) / pulp_freq;
  printf("\n  - Processing.\n");
  printf("  -     - Execution time (ck):    %llu ck\n",  cycles_processing);
  printf("  -     - Execution time (ms):    %.3f ms\n",  processing_ms);

  // DMA - Output stripe.
  const unsigned long long  cycles_dma_stripe_out  = clk_counter[5];
  const double              dma_stripe_out_ms      = (double) (cycles_dma_stripe_out * 1000) / pulp_freq;
  printf("\n  - DMA - Output stripe.\n");
  printf("  -     - Execution time (ck):    %llu ck\n",  cycles_dma_stripe_out);
  printf("  -     - Execution time (ms):    %.3f ms\n",  dma_stripe_out_ms);

  // Event unit.
  const unsigned long long  cycles_eu  = clk_counter[6];
  const double              eu_ms      = (double) (cycles_eu * 1000) / pulp_freq;
  printf("\n  - Event unit.\n");
  printf("  -     - Execution time (ck):    %llu ck\n",  cycles_eu);
  printf("  -     - Execution time (ms):    %.3f ms\n",  eu_ms);

  // Cleaning.
  const unsigned long long  cycles_cleaning     = clk_counter[7];
  const double              cleaning_ms         = (double) (cycles_cleaning * 1000) / pulp_freq;
  printf("\n  - Cleaning.\n");
  printf("  -     - Execution time (ck):    %llu ck\n",  cycles_cleaning);
  printf("  -     - Execution time (ms):    %.3f ms\n",  cleaning_ms);

  // Cycle counter overhead.
  const unsigned long long  cycles_t_overhead     = clk_counter[8];
  const double              t_overhead_ms         = (double) (cycles_t_overhead * 1000) / pulp_freq;
  printf("\n  - Timer overhead.\n");
  printf("  -     - Single timer overhead (ck):    %llu ck\n",  cycles_t_overhead);
  printf("  -     - Single timer overhead (ms):    %.3f ms\n",  t_overhead_ms);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|-------------|\n");
  printf("| Test - End. |");
  printf("\n|-------------|\n\n");

  return 0;
}