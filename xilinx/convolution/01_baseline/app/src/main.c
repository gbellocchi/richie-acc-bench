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
#include <xfilter11x11_orig.h>
#include <xfilter11x11_orig_hw.h>

/* Include host timer struct. */
#include <xil-bench.h>

/* 
 * Reserved address in Contiguous Memory. 
 * To check whether CMA has been correctly allocated: 'dmesg | grep Reserved'
 */ 

#define CMA_ADDR 0x10000000

/* Arrays. */
#define MAX_IMG_ROWS 1080
#define MAX_IMG_COLS 1920
#define IM_UAV_ROWS 320 // Inpired by AI-deck (nano-sized) 
#define IM_UAV_COLS 320 // Inpired by AI-deck (nano-sized)
#define UAV_DATA_SIZE UAV_ROWS*UAV_COLS
#define UAV_FILTER_DIM 11 // Window size


/* Checksum. */

void check_result(
    uint32_t* test_res,
    uint32_t* golden_res, 
    unsigned width, unsigned height)
{
    uint32_t n_analyzed = 0;
    uint32_t n_errors = 0;
    uint32_t err_row = 0;
    uint32_t err_col = 0;

    loop_A: for (unsigned i = 0; i < width; i++){
      loop_B: for (unsigned j = 0; j < height; j++){
        if( test_res[i * width + j] != golden_res[i * width + j] ) { 
          n_errors++;
          if(n_errors==1) n_analyzed = i * width + j;
          if(n_errors==1) err_row = i;
          if(n_errors==1) err_col = j; 
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

/* - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / - / */

/* Accelerator - Programming. */

timer_xil_exec xil_exec( 
  XFilter11x11_orig hw_acc,
  uint64_t const buffer_src,
  uint64_t const buffer_dst) 
{

  /* Timers. */

  timer_host      t_acc_progr;
  timer_host      t_proc;
  timer_xil_exec  t_out;

  /* DRAM offsets. */

  uint64_t src_dram_offset;
  uint64_t dst_dram_offset;

  /* Initialize timers. */

  t_acc_progr.t_meas = 0.0;
  t_proc.t_meas = 0.0;

  if (XFilter11x11_orig_IsReady(&hw_acc)) {

    /* Accelerator programming. */

    clock_gettime(CLOCK_REALTIME, &t_acc_progr.t0);

    /* Update DRAM offsets. */

    src_dram_offset = buffer_src; 
    dst_dram_offset = buffer_dst;

    /* Accelerator programming. */

    XFilter11x11_orig_Set_src(&hw_acc, (uint64_t)(src_dram_offset));
    XFilter11x11_orig_Set_dst(&hw_acc, (uint64_t)(dst_dram_offset));

    clock_gettime(CLOCK_REALTIME, &t_acc_progr.t1);
    t_acc_progr.t_meas += ((t_acc_progr.t1.tv_sec - t_acc_progr.t0.tv_sec) + (t_acc_progr.t1.tv_nsec - t_acc_progr.t0.tv_nsec)/1000000000.0)*1000.0;

    /* Processing. */

    clock_gettime(CLOCK_REALTIME, &t_proc.t0);

    XFilter11x11_orig_Start(&hw_acc);
    while(!XFilter11x11_orig_IsDone(&hw_acc));

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

  /* General. */

  int status;
  int fd;

  /* Algorithm parameters declaration. */
    
  const int chkr_size = 5;
  const uint32_t max_pix_val = 255;
  const uint32_t min_pix_val = 0;
  int err_cnt = 0;
  int ret_val = 20;

  uint32_t width = IM_UAV_ROWS; 
  uint32_t height = IM_UAV_COLS; 

  /* Filter components. */
  
  uint32_t filter_coeffs[UAV_FILTER_DIM] = {
      36, 111, 266, 498, 724, 821, 724, 498, 266, 111, 36
  };

  uint64_t map_dim = 256 * 4 * 1024;  // Need to map at least 4KB

  /* Allocate DRAM arrays. */

  uint32_t* l3_src_img     = (uint32_t*)malloc(width*height*sizeof(uint32_t));
  uint32_t* l3_dst_img     = (uint32_t*)malloc(width*height*sizeof(uint32_t)); 

  if ( (l3_src_img == NULL) || (l3_dst_img == NULL) ) {
    printf("ERROR: malloc() failed!\n");
    return -ENOMEM;
  }

  /* Map reserved addresses in memory. */

  if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
      printf("\n\n\n/dev/mem could not be opened.\n");
      perror("open");
      return -1;
  } else {
      printf("\n\n\n/dev/mem opened.\n\n");
  }

  uint64_t* _l3_src_img = (uint64_t *) mmap(NULL, map_dim, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CMA_ADDR);
  uint64_t* _l3_dst_img = (uint64_t *) mmap(NULL, map_dim, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CMA_ADDR + map_dim);

  if( (_l3_src_img == MAP_FAILED) || (_l3_dst_img == MAP_FAILED) ){
    printf("Mmap Failed: %s\n",strerror(errno));
    return -1;
  }

  /* Accelerator. */

  XFilter11x11_orig hw_acc;

clock_gettime(CLOCK_REALTIME, &t_alloc.t1);

t_alloc.t_meas = ((t_alloc.t1.tv_sec - t_alloc.t0.tv_sec) + (t_alloc.t1.tv_nsec - t_alloc.t0.tv_nsec)/1000000000.0)*1000.0;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* I/O arrays initialization. */

  for (int i = 0; i < IM_UAV_ROWS; i++) {
    uint32_t chkr_pair_val[2];
    if ((i / chkr_size) % 2 == 0) {
      chkr_pair_val[0] = max_pix_val; chkr_pair_val[1] = min_pix_val;
    } else {
      chkr_pair_val[0] = min_pix_val; chkr_pair_val[1] = max_pix_val;
    }
    for (int j = 0; j < IM_UAV_COLS; j++) {
      uint32_t pix_val = chkr_pair_val[(j / chkr_size) % 2];
      l3_src_img[i * IM_UAV_COLS + j] = pix_val;
    }
  }

  memset(l3_dst_img, 0, width * height * sizeof(uint32_t));

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Golden results. */

  char *filename = "golden_results.txt";
  FILE *fp = fopen(filename, "r");
  if (fp == NULL)
  {
    printf("Error: could not open file %s", filename);
    return 1;
  }

  uint32_t* l3_golden = (uint32_t*)malloc(width*height*sizeof(uint32_t)); 
  if ( l3_golden == NULL ) {
    printf("ERROR: malloc() failed!\n");
    return -ENOMEM;
  }

  for (int ii = 0; ii < height; ii++) {
    for (int jj = 0; jj < width; jj++) {
      fscanf(fp, "%u", &l3_golden[ii*width + jj]);
    }
  }

  fclose(fp);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|---------------|\n");
  printf("| Memcpy to CMA. |");
  printf("\n|---------------|\n\n");

clock_gettime(CLOCK_REALTIME, &t_memcpy_in.t0);

  /* Memcpy to CMA. */

  memcpy(_l3_src_img, l3_src_img, width*height*sizeof(uint32_t) );

clock_gettime(CLOCK_REALTIME, &t_memcpy_in.t1);

t_memcpy_in.t_meas = ((t_memcpy_in.t1.tv_sec - t_memcpy_in.t0.tv_sec) + (t_memcpy_in.t1.tv_nsec - t_memcpy_in.t0.tv_nsec)/1000000000.0)*1000.0;

  /* Initialize CMA output portion. */

  memcpy(_l3_dst_img, l3_dst_img, width*height*sizeof(uint32_t) );

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

clock_gettime(CLOCK_REALTIME, &t_acc_progr.t0);

  /* Accelerator initialization. */

  /* 
   * Former argument of the following API must match the content of '/sys/class/uio/UIO_DEVICE/name'. 
   * 'UIO_DEVICE' might vary form case to case. Check it on the board after boot.
   */

  status = XFilter11x11_orig_Initialize(&hw_acc,"filter11x11_orig"); 

  if (status != XST_SUCCESS) {
    printf("Init Error RM %d\n",status);
    return status;
  }

clock_gettime(CLOCK_REALTIME, &t_acc_progr.t1);

t_acc_progr.t_meas = ((t_acc_progr.t1.tv_sec - t_acc_progr.t0.tv_sec) + (t_acc_progr.t1.tv_nsec - t_acc_progr.t0.tv_nsec)/1000000000.0)*1000.0;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|------------------------|\n");
  printf("| Execute CONVO on FPGA. |");
  printf("\n|------------------------|\n\n");

  /* Execute hardware convolution on FPGA. */

  t_acc_exec = xil_exec( hw_acc, (uint64_t)(CMA_ADDR), (uint64_t)(CMA_ADDR + map_dim)); 

t_acc_progr.t_meas += t_acc_exec.t_meas_progr;
t_proc.t_meas = t_acc_exec.t_meas_compute;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|-----------------|\n");
  printf("| Memcpy from CMA. |");
  printf("\n|-----------------|\n\n");

clock_gettime(CLOCK_REALTIME, &t_memcpy_out.t0);

  /* Memcpy from CMA. */

  memcpy(l3_dst_img, _l3_dst_img, width*height*sizeof(uint32_t) );

clock_gettime(CLOCK_REALTIME, &t_memcpy_out.t1);
t_memcpy_out.t_meas = ((t_memcpy_out.t1.tv_sec - t_memcpy_out.t0.tv_sec) + (t_memcpy_out.t1.tv_nsec - t_memcpy_out.t0.tv_nsec)/1000000000.0)*1000.0;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|-----------|\n");
  printf("| Checksum. |");
  printf("\n|-----------|\n\n");

  /* Post-computation checksum. */

  printf("Post-computation checksum... ");
  check_result(l3_dst_img, l3_golden, width, height);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|---------|\n");
  printf("| Cleanup. |");
  printf("\n|---------|\n\n");

clock_gettime(CLOCK_REALTIME, &t_clean.t0);

  /* Cleanup. */  

  munmap(_l3_src_img,  map_dim);
  munmap(_l3_dst_img, map_dim);

  free(l3_src_img);
  free(l3_dst_img);
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