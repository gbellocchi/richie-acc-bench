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

#define _GNU_SOURCE
#include <sys/mman.h> // for mmap
#include <fcntl.h>

/* Include host timer struct. */
#include <arm-bench.h>

/* 
 * Reserved address in Contiguous Memory. 
 * To check whether CMA has been correctly allocated: 'dmesg | grep Reserved'
 */ 

#define CMA_ADDR 0x10000000

/* Host matrix multiplication. */

void mmult_sw(uint32_t* in1, uint32_t* in2, uint32_t* out, uint32_t mat_dim, uint32_t stripe_height)
{
  for (unsigned i = 0; i < mat_dim; i++){
    for (unsigned j = 0; j < mat_dim; j++){
      for (unsigned k = 0; k < mat_dim; k++){
        out[i * mat_dim + j] += in1[i * mat_dim + k] * in2[j * mat_dim  + k];
      }
    }
  }
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
  timer_host t_proc;
  timer_host t_memcpy_out;
  timer_host t_clean;

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

clock_gettime(CLOCK_REALTIME, &t_alloc.t1);

t_alloc.t_meas = ((t_alloc.t1.tv_sec - t_alloc.t0.tv_sec) + (t_alloc.t1.tv_nsec - t_alloc.t0.tv_nsec)/1000000000.0)*1000.0;

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

  printf("\n|---------------|\n");
  printf("| Memcpy to CMA. |");
  printf("\n|---------------|\n\n");

clock_gettime(CLOCK_REALTIME, &t_memcpy_in.t0);

  /* Memcpy to CMA. */

  memcpy(_l3_in1, l3_in1, width*height*sizeof(uint32_t) );
  memcpy(_l3_in2, l3_in2, width*height*sizeof(uint32_t) );

clock_gettime(CLOCK_REALTIME, &t_memcpy_in.t1);

t_memcpy_in.t_meas = ((t_memcpy_in.t1.tv_sec - t_memcpy_in.t0.tv_sec) + (t_memcpy_in.t1.tv_nsec - t_memcpy_in.t0.tv_nsec)/1000000000.0)*1000.0;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|------------------------|\n");
  printf("| Execute MMULT on ARM. |");
  printf("\n|------------------------|\n\n");

clock_gettime(CLOCK_REALTIME, &t_proc.t0);

  /* Execute hardware mmult on ARM. */

  mmult_sw( _l3_in1, _l3_in2, _l3_test, width, stripe_height);

clock_gettime(CLOCK_REALTIME, &t_proc.t1);

t_proc.t_meas = ((t_proc.t1.tv_sec - t_proc.t0.tv_sec) + (t_proc.t1.tv_nsec - t_proc.t0.tv_nsec)/1000000000.0)*1000.0;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|-----------------|\n");
  printf("| Memcpy from CMA. |");
  printf("\n|-----------------|\n\n");

clock_gettime(CLOCK_REALTIME, &t_memcpy_out.t0);

  /* Memcpy from CMA. */

  memcpy(l3_test, _l3_test, width*height*sizeof(uint32_t) );

clock_gettime(CLOCK_REALTIME, &t_memcpy_out.t1);

t_memcpy_out.t_meas = ((t_memcpy_out.t1.tv_sec - t_memcpy_out.t0.tv_sec) + (t_memcpy_out.t1.tv_nsec - t_memcpy_out.t0.tv_nsec)/1000000000.0)*1000.0;

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

  printf("\n  - Host execution:\n");
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