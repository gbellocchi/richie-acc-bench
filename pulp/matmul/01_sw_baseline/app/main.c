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

#include <hero-target.h>

#include "../../../common/overlay-bench.h"

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

    loop_A: for (unsigned ii = 0; ii < width; ii+=stripe_height){
      loop_B: for (unsigned jj = 0; jj < height; jj+=stripe_height){
        loop_C: for (unsigned i = 0; i < stripe_height; i++){
          loop_D: for (unsigned j = 0; j < stripe_height; j++){
            if( test_res[(ii + i) * width + jj + j] != golden_res[(ii + i) * width + jj + j] ) { 
                n_errors++;
                if(n_errors==1) n_analyzed = (ii + i) * width + jj + j;
                if(n_errors==1) err_row = ii + i;
                if(n_errors==1) err_col = jj + j; 
            }
          }
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
        printf("Tested result is %d.\n", test_res[err_row*width+err_col]);
        printf("Golden result is %d.\n\n", golden_res[err_row*width+err_col]);
    }
}

/* Golden result calculation. */

void mmult_sw(uint32_t* in1, uint32_t* in2, uint32_t* out_sw, uint32_t mat_dim, uint32_t stripe_height)
{
  loop_A: for (unsigned ii = 0; ii < mat_dim; ii+=stripe_height){
    loop_B: for (unsigned jj = 0; jj < mat_dim; jj+=stripe_height){
      loop_C: for (unsigned i = 0; i < stripe_height; i++){
        loop_D: for (unsigned j = 0; j < stripe_height; j++){
          loop_E: for (unsigned k = 0; k < mat_dim; k++){
            out_sw[(ii + i) * mat_dim + jj + j] += in1[(ii + i) * mat_dim + k] * in2[(jj + j) * mat_dim + k];
          }
        }
      }
    }
  }
}

/* - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / */

/*
 *
 *     PULP - Functions.
 *
 */

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

  hero_dma_job_t dma = hero_memcpy_host2dev_async(local_addr, ext_addr, size);

  hero_dma_wait(dma);

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

  uint64_t ext_out_addr = (uint64_t)ext_dst + (uint64_t)out_dram_offset;
  uint32_t local_out_addr = (uint32_t)local_src;

  const size_t size = width * stripe_height * sizeof(uint32_t);

  hero_dma_job_t dma = hero_memcpy_dev2host_async(ext_out_addr, local_out_addr, size);
  hero_dma_wait(dma);

  s.clk_1 = hero_get_clk_counter();

  return s;
}

/* RISCV - Matrix multiplication on data stripe. */

compute_struct compute(
  __device uint32_t* const in1,
  __device uint32_t* const in2,
  __device uint32_t* const out,
  const size_t width,
  const size_t stripe_height)
{
  compute_struct s;

  s.clk_0 = hero_get_clk_counter();

  unsigned sum = 0;

  compute_0: for (unsigned i = 0; i < stripe_height; i++){
    compute_1: for (unsigned j = 0; j < stripe_height; j++){
      sum = 0;
      compute_2: for (unsigned k = 0; k < width; k++){
        sum += in1[i * width + k] * in2[j * width + k];
      }
      out[i * width + j] = sum;
    }
  }

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

int mmult_riscv(
  __host uint32_t * __restrict__ l3_in1,
  __host uint32_t * __restrict__ l3_in2,
  __host uint32_t * __restrict__ l3_out,
  __host uint32_t * __restrict__ l3_golden,
  uint32_t width, uint32_t height, uint32_t stripe_height, uint32_t clk_counter[10])
{
  if (hero_rt_core_id() == 0){

    /* Cycle counters. */
    
    uint32_t t_pulp_0,  t_pulp_1;  
    uint32_t t_clean_0, t_clean_1; 
    uint32_t t_alloc_0, t_alloc_1;
    uint32_t t_dma_in   = 0;
    uint32_t t_proc     = 0;
    uint32_t t_dma_out  = 0;

    /* ------------------------------------------------------------------------------- */

    t_pulp_0 = hero_get_clk_counter();

    /* Errors. */

    int n_errors = 0;

    t_alloc_0 = hero_get_clk_counter();

    /* DRAM offsets. */

    uint32_t in1_dram_offset;
    uint32_t in2_dram_offset;
    uint32_t out_dram_offset;

    /* TCDM ouput buffer offsets. */

    uint32_t l1_buffer_out_offset;

    /* Output structs. */

    load_struct    fetch;
    compute_struct  comp;
    store_struct    store;

    /* Allocate L1 with single buffers for each stripe data type. */

    __device uint32_t* l1_buffer_in1  = (__device uint32_t *)hero_l1malloc(width * stripe_height * sizeof(uint32_t));
    __device uint32_t* l1_buffer_in2  = (__device uint32_t *)hero_l1malloc(width * stripe_height * sizeof(uint32_t));
    __device uint32_t* l1_buffer_out   = (__device uint32_t *)hero_l1malloc(width * stripe_height * sizeof(uint32_t));

    if (l1_buffer_in1 == NULL) {
      printf("ERROR 0: Memory allocation failed!\n");
      return 1;
    }

    if (l1_buffer_in2 == NULL) {
      printf("ERROR 1: Memory allocation failed!\n");
      return 1;
    }

    if (l1_buffer_out == NULL) {
      printf("ERROR 2: Memory allocation failed!\n");
      return 1;
    }

    /* Initialize DRAM offsets. */

    in1_dram_offset = 0;
    in2_dram_offset = 0;
    out_dram_offset = 0;

    /* Initialize TCDM offset. */

    l1_buffer_out_offset = 0;

    t_alloc_1 = hero_get_clk_counter();

    /* Stripe prefetching is implemented in software using loop_A and loop_B. */

    loop_A: for(int ii = 0; ii < width; ii += stripe_height){

      /* Update in1 DRAM offset. */

      in1_dram_offset = ii * width * sizeof(uint32_t);

      /* Transfer in1 stripe from L3 to L1 using DMA. */

      fetch = l3_to_l1(l3_in1,  l1_buffer_in1,  in1_dram_offset, width, height, stripe_height);

      t_dma_in += fetch.clk_1 - fetch.clk_0;

      loop_B: for(int jj = 0; jj < height; jj += stripe_height){

        /* Update in2 DRAM offset. */

        in2_dram_offset = jj * height * sizeof(uint32_t);

        /* Transfer in2 stripe from L3 to L1 using DMA. */

        fetch = l3_to_l1(l3_in2, l1_buffer_in2, in2_dram_offset, width, height, stripe_height);

        t_dma_in += fetch.clk_1 - fetch.clk_0;

        /* Stripe processing. */

        l1_buffer_out_offset = jj;

        comp = compute(l1_buffer_in1, l1_buffer_in2, l1_buffer_out + l1_buffer_out_offset, width, stripe_height);

        t_proc += comp.clk_1 - comp.clk_0;

      }

      /* Transfer output stripe from L1 to L3 using DMA. */

      out_dram_offset = (ii * width) * sizeof(uint32_t);

      store = l1_to_l3(l1_buffer_out, l3_out, out_dram_offset, width, height, stripe_height);

      t_dma_out += store.clk_1 - store.clk_0;

    }

    /* Cleanup. */

    t_clean_0 = hero_get_clk_counter();

    hero_l1free(l1_buffer_in1);
    hero_l1free(l1_buffer_in2);
    hero_l1free(l1_buffer_out);

    t_clean_1 = hero_get_clk_counter();

    /* ------------------------------------------------------------------------------- */

    t_pulp_1 = hero_get_clk_counter();

    clk_counter[0] = t_pulp_1 - t_pulp_0;
    clk_counter[1] = t_alloc_1 - t_alloc_0;
    clk_counter[2] = t_dma_in;
    clk_counter[3] = t_proc;
    clk_counter[4] = t_dma_out;
    clk_counter[5] = t_clean_1 - t_clean_0;

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

  struct timespec t_decl_0, t_decl_1;
  struct timespec t_app_0, t_app_1;
  struct timespec t_clean_0, t_clean_1;
  uint32_t clk_counter[10];

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|----------------------------------------------------|\n");
  printf("| DRAM - Declaration, allocation and initialization. |");
  printf("\n|----------------------------------------------------|\n\n");

clock_gettime(CLOCK_REALTIME, &t_decl_0);

  /* Matrix dimension. */

  unsigned width          = 512;
  unsigned height         = 512;
  unsigned stripe_height  = 8;

  /* General. */

  int pulp_error;

  /* Allocate DRAM arrays. */

  uint32_t* l3_in1      = (__host int32_t*)hero_l3malloc(width*height*sizeof(int32_t));
  uint32_t* l3_in2      = (__host int32_t*)hero_l3malloc(width*height*sizeof(int32_t)); 
  uint32_t* l3_test     = (__host int32_t*)hero_l3malloc(width*height*sizeof(int32_t)); 

  if ( (l3_in1 == NULL) || (l3_in2 == NULL) || (l3_test == NULL) ) {
    printf("ERROR: malloc() failed!\n");
    return -ENOMEM;
  }

clock_gettime(CLOCK_REALTIME, &t_decl_1);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* I/O arrays initialization. */

  for(int i=0; i<width*height; i++){
    l3_in1[i] = rand() % 255;
    l3_in2[i] = rand() % 255;
  }
  memset(l3_test, 0, width * height * sizeof(uint32_t));

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
  check_result(l3_test, l3_golden, width, height, stripe_height);

  /* Calculate golden results. */

  mmult_sw( l3_in1, l3_in2, l3_golden, width, stripe_height);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Other parameters. Not offloaded to PULP. */

  const unsigned stripe_len_in        = width*stripe_height;
  const unsigned stripe_len_out       = stripe_height*stripe_height;
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

  const float TCDM_occupation_perc = (float)(((stripe_in_len_B * 2 + stripe_out_len_B * 1) / TCDM_size_B) * 100.0); 

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
  printf("| Execute MMULT on PULP. |");
  printf("\n|------------------------|\n\n");

  /* Warm-up. */

  unsigned tmp_1 = 1;
  unsigned tmp_2 = 2;
  #pragma omp target device(1) map(to: tmp_1) map(from: tmp_2)
  {
      tmp_2 = tmp_1;
  }
  tmp_1 = tmp_2;

clock_gettime(CLOCK_REALTIME, &t_app_0);

  /* Offloaded application. */

  #pragma omp target device(BIGPULP_MEMCPY) \
              map(to:         l3_in1[0:width*height], l3_in2[0:width*height], l3_golden[0:width*height], width, stripe_height)                            \
              map(tofrom:     l3_test[0:width*height], clk_counter[0:10]) 
  {
    pulp_error = mmult_riscv(l3_in1, l3_in2, l3_test, l3_golden, width, height, stripe_height, clk_counter);
  }

clock_gettime(CLOCK_REALTIME, &t_app_1);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|-----------|\n");
  printf("| Checksum. |");
  printf("\n|-----------|\n\n");

  if(pulp_error) return 1;

  /* Post-computation checksum. */

  printf("Post-computation checksum... ");
  check_result(l3_test, l3_golden, width, height, stripe_height);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|---------|\n");
  printf("| Cleanup. |");
  printf("\n|---------|\n\n");

clock_gettime(CLOCK_REALTIME, &t_clean_0);

  /* Cleanup. */  

  hero_l3free(l3_in1);
  hero_l3free(l3_in2);
  hero_l3free(l3_test);
  hero_l3free(l3_golden);

clock_gettime(CLOCK_REALTIME, &t_clean_1);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Results - ARM measurements. */

  printf("\n|-----------------------------|\n");
  printf("| Results - ARM measurements. |");
  printf("\n|-----------------------------|\n");

  printf("\n  - I/O arrays allocation and initialization:\n");
  printf("  -     - Execution time (ms):    %.3f ms\n", ((t_decl_1.tv_sec - t_decl_0.tv_sec) + (t_decl_1.tv_nsec - t_decl_0.tv_nsec)/1000000000.0)*1000.0 );

  printf("\n  - Offloading + PULP application:\n");
  printf("  -     - Execution time (ms):    %.3f ms\n", ((t_app_1.tv_sec  - t_app_0.tv_sec) + (t_app_1.tv_nsec - t_app_0.tv_nsec)/1000000000.0)*1000.0 );

  printf("\n  - Cleaning:\n");
  printf("  -     - Execution time (ms):    %.3f ms\n", ((t_clean_1.tv_sec  - t_clean_0.tv_sec) + (t_clean_1.tv_nsec - t_clean_0.tv_nsec)/1000000000.0)*1000.0 );

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Results - PULP measurements. */

  printf("\n|------------------------------|\n");
  printf("| Results - PULP measurements. |");
  printf("\n|------------------------------|\n");

  /* PULP application execution. */

  const unsigned pulp_freq = 50 * 1000000;

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

  // Processing.
  const unsigned long long  cycles_processing     = clk_counter[3];
  const double              processing_ms         = (double) (cycles_processing * 1000) / pulp_freq;
  printf("\n  - Processing.\n");
  printf("  -     - Execution time (ck):    %llu ck\n",  cycles_processing);
  printf("  -     - Execution time (ms):    %.3f ms\n",  processing_ms);

  // DMA - Output stripe.
  const unsigned long long  cycles_dma_stripe_out  = clk_counter[4];
  const double              dma_stripe_out_ms      = (double) (cycles_dma_stripe_out * 1000) / pulp_freq;
  printf("\n  - DMA - Output stripe.\n");
  printf("  -     - Execution time (ck):    %llu ck\n",  cycles_dma_stripe_out);
  printf("  -     - Execution time (ms):    %.3f ms\n",  dma_stripe_out_ms);

  // Cleaning.
  const unsigned long long  cycles_cleaning     = clk_counter[5];
  const double              cleaning_ms         = (double) (cycles_cleaning * 1000) / pulp_freq;
  printf("\n  - Cleaning.\n");
  printf("  -     - Execution time (ck):    %llu ck\n",  cycles_cleaning);
  printf("  -     - Execution time (ms):    %.3f ms\n",  cleaning_ms);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|-------------|\n");
  printf("| Test - End. |");
  printf("\n|-------------|\n\n");

  return 0;
}