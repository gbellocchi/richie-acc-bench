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

/* Include accelerator drivers. */
#include <xmmult_hw.h>
#include <xmmult_hw_hw.h>

/* Include host timer struct. */
#include <xil-bench.h>

/* 
 * Reserved address in Contiguous Memory. 
 * To check whether CMA has been correctly allocated: 'dmesg | grep Reserved'
 */ 

#define CMA_ADDR 0x10000000


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

/* Accelerator - Programming. */

timer_xil_exec xil_exec( 
  XMmult_hw hw_acc,
  uint32_t const buffer_in1,
  uint32_t const buffer_in2,
  uint32_t const buffer_out) 
{

  /* Timers. */

  timer_host      t_acc_progr;
  timer_host      t_proc;
  timer_xil_exec  t_out;

  /* DRAM offsets. */

  uint32_t in1_dram_offset;
  uint32_t in2_dram_offset;
  uint32_t out_dram_offset;

  /* Initialize timers. */

  t_acc_progr.t_meas = 0.0;
  t_proc.t_meas = 0.0;

  if (XMmult_hw_IsReady(&hw_acc)) {

    /* Accelerator programming. */

    clock_gettime(CLOCK_REALTIME, &t_acc_progr.t0);

    /* Update DRAM offsets. */

    in1_dram_offset = buffer_in1; 
    in2_dram_offset = buffer_in2;
    out_dram_offset = buffer_out;

    /* Accelerator programming. */

    XMmult_hw_Set_in1(&hw_acc, (uint32_t)(in1_dram_offset));
    XMmult_hw_Set_in2(&hw_acc, (uint32_t)(in2_dram_offset));
    XMmult_hw_Set_out_r(&hw_acc, (uint32_t)(out_dram_offset));

    clock_gettime(CLOCK_REALTIME, &t_acc_progr.t1);
    t_acc_progr.t_meas += ((t_acc_progr.t1.tv_sec - t_acc_progr.t0.tv_sec) + (t_acc_progr.t1.tv_nsec - t_acc_progr.t0.tv_nsec)/1000000000.0)*1000.0;

    /* Processing. */

    clock_gettime(CLOCK_REALTIME, &t_proc.t0);

    XMmult_hw_Start(&hw_acc);
    while(!XMmult_hw_IsDone(&hw_acc));

    clock_gettime(CLOCK_REALTIME, &t_proc.t1);
    t_proc.t_meas += ((t_proc.t1.tv_sec - t_proc.t0.tv_sec) + (t_proc.t1.tv_nsec - t_proc.t0.tv_nsec)/1000000000.0)*1000.0;

  } else {

    printf("Accelerator is not ready..\n");

  }

  t_out.t_meas_progr    = t_acc_progr.t_meas;
  t_out.t_meas_compute  = t_proc.t_meas;

  return t_out;
}


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
  timer_host t_memcpy_in;
  timer_host t_acc_progr;
  timer_host t_proc;
  timer_host t_memcpy_out;
  timer_host t_clean;

  timer_xil_exec t_acc_exec;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|----------------------------------------------------|\n");
  printf("| DRAM - Declaration, allocation and initialization. |");
  printf("\n|----------------------------------------------------|\n\n");

clock_gettime(CLOCK_REALTIME, &t_alloc.t0);

  /* Matrix dimension. */

  unsigned width          = 512;
  unsigned height         = 512;
  unsigned stripe_height  = 8;

  /* General. */

  int status;
  int fd;

  uint64_t map_dim = 256 * 4 * 1024;  // Need to map at least 4KB

  /* Allocate DRAM arrays. */

  uint32_t* l3_in1      = (int32_t*)malloc(width*height*sizeof(uint32_t));
  uint32_t* l3_in2      = (int32_t*)malloc(width*height*sizeof(uint32_t)); 
  uint32_t* l3_test     = (int32_t*)malloc(width*height*sizeof(uint32_t)); 

  if ( (l3_in1 == NULL) || (l3_in2 == NULL) || (l3_test == NULL) ) {
    printf("ERROR: malloc() failed!\n");
    return -ENOMEM;
  }

  /* I/O arrays initialization. */

  for(int i=0; i<width*height; i++){
    l3_in1[i]   = rand() % 255;
    l3_in2[i]   = rand() % 255;
  }
  memset(l3_test, 0, width * height * sizeof(uint32_t));

  /* Map reserved addresses in memory. */

  if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
      printf("\n\n\n/dev/mem could not be opened.\n");
      perror("open");
      return -1;
  } else {
      printf("\n\n\n/dev/mem opened.\n\n");
  }

  uint32_t* _l3_in1     = (uint32_t*) mmap(NULL, map_dim, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CMA_ADDR);
  uint32_t* _l3_in2     = (uint32_t*) mmap(NULL, map_dim, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CMA_ADDR + map_dim);
  uint32_t* _l3_test    = (uint32_t*) mmap(NULL, map_dim, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CMA_ADDR + 2 * map_dim);

  if( (_l3_in1 == MAP_FAILED) || (_l3_in2 == MAP_FAILED) || (_l3_test == MAP_FAILED) ){
    printf("Mmap Failed: %s\n",strerror(errno));
    return -1;
  }

  /* Accelerator. */

  XMmult_hw hw_acc;

clock_gettime(CLOCK_REALTIME, &t_alloc.t1);

t_alloc.t_meas = ((t_alloc.t1.tv_sec - t_alloc.t0.tv_sec) + (t_alloc.t1.tv_nsec - t_alloc.t0.tv_nsec)/1000000000.0)*1000.0;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Allocate and initialize golden results. */

  uint32_t* l3_golden   = (int32_t*)malloc(width*height*sizeof(int32_t)); 

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

  /* Additional parameters. */

  const unsigned stripe_len_in        = width*stripe_height;
  const unsigned stripe_len_out       = stripe_height*stripe_height;
  const unsigned stripe_in_len_B      = stripe_len_in * sizeof(uint32_t);
  const float stripe_in_len_kB        = stripe_in_len_B / 1024.0;
  const unsigned stripe_out_len_B     = stripe_len_out * sizeof(uint32_t);
  const float stripe_out_len_kB       = stripe_out_len_B / 1024.0;

  printf("Matrix multiplication parameters\n");
  printf("Width                 - %d        \n", width                );
  printf("Height                - %d        \n", height               );
  printf("Stripe_len in         - %d        \n", stripe_len_in        );
  printf("Stripe_len in  (B)    - %d B      \n", stripe_in_len_B      );
  printf("Stripe_len in  (kB)   - %.3f kB   \n", stripe_in_len_kB     );
  printf("Stripe_len out        - %d        \n", stripe_len_out       );
  printf("Stripe_len out (B)    - %d B      \n", stripe_out_len_B     );
  printf("Stripe_len out (kB)   - %.3f kB   \n", stripe_out_len_kB    );

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

clock_gettime(CLOCK_REALTIME, &t_memcpy_in.t0);

  /* Memcpy to CMA. */

  memcpy(_l3_in1, l3_in1, width*height*sizeof(uint32_t) );
  memcpy(_l3_in2, l3_in2, width*height*sizeof(uint32_t) );

clock_gettime(CLOCK_REALTIME, &t_memcpy_in.t1);

t_memcpy_in.t_meas = ((t_memcpy_in.t1.tv_sec - t_memcpy_in.t0.tv_sec) + (t_memcpy_in.t1.tv_nsec - t_memcpy_in.t0.tv_nsec)/1000000000.0)*1000.0;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

clock_gettime(CLOCK_REALTIME, &t_acc_progr.t0);

  /* Accelerator initialization. */

  /* 
   * Former argument of the following API must match the content of '/sys/class/uio/UIO_DEVICE/name'. 
   * 'UIO_DEVICE' might vary form case to case. Check it on the board after boot.
   */

  status = XMmult_hw_Initialize(&hw_acc,"mmult_hw"); 

  if (status != XST_SUCCESS) {
    printf("Init Error RM %d\n",status);
    return status;
  }

clock_gettime(CLOCK_REALTIME, &t_acc_progr.t1);

t_acc_progr.t_meas = ((t_acc_progr.t1.tv_sec - t_acc_progr.t0.tv_sec) + (t_acc_progr.t1.tv_nsec - t_acc_progr.t0.tv_nsec)/1000000000.0)*1000.0;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|------------------------|\n");
  printf("| Execute MMULT on FPGA. |");
  printf("\n|------------------------|\n\n");

  /* Execute hardware mmult on FPGA. */

  t_acc_exec = xil_exec( hw_acc, (uint32_t)(CMA_ADDR), (uint32_t)(CMA_ADDR + map_dim), (uint32_t)(CMA_ADDR + 2*map_dim)); 

  t_acc_progr.t_meas += t_acc_exec.t_meas_progr;
  t_proc.t_meas = t_acc_exec.t_meas_compute;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

clock_gettime(CLOCK_REALTIME, &t_memcpy_out.t0);

  /* Memcpy from CMA. */

  memcpy(l3_test, _l3_test, width*height*sizeof(uint32_t) );

clock_gettime(CLOCK_REALTIME, &t_memcpy_out.t1);

t_memcpy_out.t_meas = ((t_memcpy_out.t1.tv_sec - t_memcpy_out.t0.tv_sec) + (t_memcpy_out.t1.tv_nsec - t_memcpy_out.t0.tv_nsec)/1000000000.0)*1000.0;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|-----------|\n");
  printf("| Checksum. |");
  printf("\n|-----------|\n\n");

  /* Post-computation checksum. */

  printf("Post-computation checksum... ");
  check_result(l3_test, l3_golden, width, height, stripe_height);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|---------|\n");
  printf("| Cleanup. |");
  printf("\n|---------|\n\n");

clock_gettime(CLOCK_REALTIME, &t_clean.t0);

  /* Cleanup. */  

  munmap(_l3_in1,  map_dim);
  munmap(_l3_in2,  map_dim);
  munmap(_l3_test, map_dim);

  free(l3_in1);
  free(l3_in2);
  free(l3_test);
  free(l3_golden);

clock_gettime(CLOCK_REALTIME, &t_clean.t1);

t_clean.t_meas = ((t_clean.t1.tv_sec - t_clean.t0.tv_sec) + (t_clean.t1.tv_nsec - t_clean.t0.tv_nsec)/1000000000.0)*1000.0;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Results - ARM measurements. */

  printf("\n|-----------------------------|\n");
  printf("| Results - ARM measurements. |");
  printf("\n|-----------------------------|\n");

  printf("\n  - I/O arrays allocation and initialization:\n");
  printf("  -     - Execution time (ms):    %.3f ms\n", t_alloc.t_meas );

  printf("\n  - Memcpy to CMA:\n");
  printf("  -     - Execution time (ms):    %.3f ms\n", t_memcpy_in.t_meas );

  printf("\n  - Accelerator programming:\n");
  printf("  -     - Execution time (ms):    %.3f ms\n", t_acc_progr.t_meas );

  printf("\n  - Accelerator execution:\n");
  printf("  -     - Execution time (ms):    %.3f ms\n", t_proc.t_meas );

  printf("\n  - Memcpy from CMA:\n");
  printf("  -     - Execution time (ms):    %.3f ms\n", t_memcpy_out.t_meas );

  printf("\n  - Cleaning:\n");
  printf("  -     - Execution time (ms):    %.3f ms\n", t_clean.t_meas );

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|-------------|\n");
  printf("| Test - End. |");
  printf("\n|-------------|\n\n");

  return 0;
}