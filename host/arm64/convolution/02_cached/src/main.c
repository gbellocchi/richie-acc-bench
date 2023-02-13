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

/* Include host timer struct. */
#include <arm-bench.h>

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

/* Host 2D convolution. */

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
        out[pixel] += out_H[pixel + offset] * coeffs[i + border_width];
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
      out[pixel] = out[border_width_offset + border_width];
    }
    Top_Row:for(int row = border_width; row < width - border_width; row++){
      int pixel = offset + row;
      out[pixel] = out[border_width_offset + row];
    }
    Top_Right:for(int row = width - border_width; row < width; row++){
      int pixel = offset + row;
      out[pixel] = out[border_width_offset + width - border_width - 1];
    }
  }

  Side_Border:for(int col = border_width; col < height - border_width; col++){
    int offset = col * width;
    for(int row = 0; row < border_width; row++){
      int pixel = offset + row;
      out[pixel] = out[offset + border_width];
    }
    for(int row = width - border_width; row < width; row++){
      int pixel = offset + row;
      out[pixel] = out[offset + width - border_width - 1];
    }
  }

  Bottom_Border:for(int col = height - border_width; col < height; col++){
    int offset = col * width;
    for(int row = 0; row < border_width; row++){
      int pixel = offset + row;
      out[pixel] = out[border_height_offset + border_width];
    }
    for(int row = border_width; row < width - border_width; row++){
      int pixel = offset + row;
      out[pixel] = out[border_height_offset + row];
    }
    for(int row = width - border_width; row < width; row++){
      int pixel = offset + row;
      out[pixel] = out[border_height_offset + width - border_width - 1];
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
  timer_host t_proc;
  timer_host t_clean;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|----------------------------------------------------|\n");
  printf("| DRAM - Declaration, allocation and initialization. |");
  printf("\n|----------------------------------------------------|\n\n");

clock_gettime(CLOCK_REALTIME, &t_alloc.t0);

  /* Matrix dimension. */

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

  int status;
  int fd;

  uint64_t map_dim = 256 * 4 * 1024;  // Need to map at least 4KB

  /* Allocate DRAM arrays. */

  uint32_t* l3_src = (uint32_t*)malloc(width*height*sizeof(uint32_t));
  uint32_t* l3_dst = (uint32_t*)malloc(width*height*sizeof(uint32_t)); 

  if ( (l3_src == NULL) || (l3_dst == NULL) ) {
    printf("ERROR: malloc() failed!\n");
    return -ENOMEM;
  }

  /* I/O arrays initialization. */

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

clock_gettime(CLOCK_REALTIME, &t_alloc.t1);

t_alloc.t_meas = ((t_alloc.t1.tv_sec - t_alloc.t0.tv_sec) + (t_alloc.t1.tv_nsec - t_alloc.t0.tv_nsec)/1000000000.0)*1000.0;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Golden results. */

  // Open file

  char *filename = "golden_results.txt";
  FILE *fp = fopen(filename, "r");
  if (fp == NULL)
  {
    printf("Error: could not open file %s", filename);
    return 1;
  }

  // Golden results

  uint32_t* l3_golden = (uint32_t*)malloc(width*height*sizeof(uint32_t)); 
  if ( l3_golden == NULL ) {
    printf("ERROR: malloc() failed!\n");
    return -ENOMEM;
  }

  for (int ii = 0; ii < height; ii++) {
    for (int jj = 0; jj < width; jj++) {
      int tmp = fscanf(fp, "%u", &l3_golden[ii*width + jj]);
      if (tmp != 1) printf("Failed to read integer.\n");
    }
  }

  // Close file

  fclose(fp);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Additional parameters. */

  const unsigned stripe_len        = width*stripe_height;
  const unsigned stripe_len_B      = stripe_len * sizeof(uint32_t);
  const float stripe_len_kB        = stripe_len_B / 1024.0;

  printf("Matrix multiplication parameters\n");
  printf("Width             - %d        \n", width                );
  printf("Height            - %d        \n", height               );
  printf("Stripe_len        - %d        \n", stripe_len           );    
  printf("Stripe_len (B)    - %d B      \n", stripe_len_B         );    
  printf("Stripe_len (kB)   - %.3f kB   \n", stripe_len_kB        ); 

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|------------------------|\n");
  printf("| Execute CONVO on ARM. |");
  printf("\n|------------------------|\n\n");

clock_gettime(CLOCK_REALTIME, &t_proc.t0);

  /* Execute 2D convolution on ARM. */

  convolution_sw( l3_src, l3_dst, filter_coeffs, width, height);

clock_gettime(CLOCK_REALTIME, &t_proc.t1);

t_proc.t_meas = ((t_proc.t1.tv_sec - t_proc.t0.tv_sec) + (t_proc.t1.tv_nsec - t_proc.t0.tv_nsec)/1000000000.0)*1000.0;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|-----------|\n");
  printf("| Checksum. |");
  printf("\n|-----------|\n\n");

  /* Post-computation checksum. */

  printf("Post-computation checksum... ");
  check_result(l3_dst, l3_golden, width, height);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|---------|\n");
  printf("| Cleanup. |");
  printf("\n|---------|\n\n");

clock_gettime(CLOCK_REALTIME, &t_clean.t0);

  /* Cleanup. */  

  free(l3_src);
  free(l3_dst);

clock_gettime(CLOCK_REALTIME, &t_clean.t1);

t_clean.t_meas = ((t_clean.t1.tv_sec - t_clean.t0.tv_sec) + (t_clean.t1.tv_nsec - t_clean.t0.tv_nsec)/1000000000.0)*1000.0;

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Results - ARM measurements. */

  printf("\n|-----------------------------|\n");
  printf("| Results - ARM measurements. |");
  printf("\n|-----------------------------|\n");

  printf("\n  - I/O arrays allocation and initialization:\n");
  printf("  -     - Execution time (ms):    %.3f ms\n", t_alloc.t_meas );

  printf("\n  - Host execution:\n");
  printf("  -     - Execution time (ms):    %.3f ms\n", t_proc.t_meas );

  printf("\n  - Cleaning:\n");
  printf("  -     - Execution time (ms):    %.3f ms\n", t_clean.t_meas );

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  printf("\n|-------------|\n");
  printf("| Test - End. |");
  printf("\n|-------------|\n\n");

  return 0;
}