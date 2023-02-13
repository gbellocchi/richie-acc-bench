/**********
Copyright (c) 2018, Xilinx, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********/

#include "mmult.h"

/*
 *
 * Matrix multiplication - SW execution.
 *
 */

void mmult_sw(data_t *in1, data_t *in2, data_t *out, data_t mat_dim, data_t stripe_height)
{
  loop_A: for (unsigned ii = 0; ii < mat_dim; ii+=stripe_height){
    loop_B: for (unsigned jj = 0; jj < mat_dim; jj+=stripe_height){
      loop_C: for (unsigned i = 0; i < stripe_height; i++){
        loop_D: for (unsigned j = 0; j < stripe_height; j++){
          loop_E: for (unsigned k = 0; k < mat_dim; k++){
            out[(ii + i) * mat_dim + jj + j] += in1[(ii + i) * mat_dim + k] * in2[(jj + j) * mat_dim + k];
          }
        }
      }
    }
  }
}

/*
 *
 * Matrix multiplication - Functions.
 *
 */

void prefetch(data_t *in1, data_t * in2, data_t buffer_in1[STRIPE_HEIGHT][MAT_DIM], data_t buffer_in2[STRIPE_HEIGHT][MAT_DIM], data_t in1_dram_offset, data_t in2_dram_offset);
void comp(data_t buffer_in1[STRIPE_HEIGHT][MAT_DIM], data_t buffer_in2[STRIPE_HEIGHT][MAT_DIM], data_t *out, data_t out_dram_offset);

/*
 *
 * Matrix multiplication - HW execution.
 *
 */

void mmult_hw(data_t *in1, data_t *in2, data_t *out)
{

  /* Interface declaration. */

  #pragma HLS INTERFACE m_axi port=in1 offset=slave bundle=port_in1
  #pragma HLS INTERFACE m_axi port=in2 offset=slave bundle=port_in2
  #pragma HLS INTERFACE m_axi port=out offset=slave bundle=port_out

  #pragma HLS INTERFACE s_axilite port=return	bundle=control

  /* Constants. */

  const int mat_dim           = MAT_DIM;
  const int stripe_height     = STRIPE_HEIGHT;
  const int fsm_calls         = LOOP_ITERS;

  /* Local buffers. */

  data_t buffer_in1_A[stripe_height][mat_dim];
  data_t buffer_in1_B[stripe_height][mat_dim];

  data_t buffer_in2_A[stripe_height][mat_dim];
  data_t buffer_in2_B[stripe_height][mat_dim];

  /* Double buffering variables. */

  bool sel;

  /* DRAM offsets. */

  data_t in1_dram_offset;
  data_t in2_dram_offset;
  data_t out_dram_offset;

  #pragma HLS ARRAY_PARTITION variable=buffer_in1_A complete dim=2 
  #pragma HLS ARRAY_PARTITION variable=buffer_in1_B complete dim=2

  #pragma HLS ARRAY_PARTITION variable=buffer_in2_A complete dim=2 
  #pragma HLS ARRAY_PARTITION variable=buffer_in2_B complete dim=2

  /* Initialization. */ 

  in1_dram_offset = 0;
  in2_dram_offset = 0;
  out_dram_offset = 0;

  sel = 1;

  /* Matrix multiplication. */

  loop_A: for(int ii = 0; ii < mat_dim; ii += stripe_height){
  #pragma HLS LOOP_TRIPCOUNT min=fsm_calls max=fsm_calls

    /* Calculate DRAM offset. */

    in1_dram_offset = ii * mat_dim;

    loop_B: for(int jj = 0; jj < mat_dim; jj += stripe_height){
    #pragma HLS LOOP_TRIPCOUNT min=fsm_calls max=fsm_calls

      /* Calculate DRAM offset. */

      in2_dram_offset = jj * mat_dim;

      /* Double buffering. */

      if (sel) {
        prefetch(in1, in2, buffer_in1_A, buffer_in2_A, in1_dram_offset, in2_dram_offset);
        comp(buffer_in1_B, buffer_in2_B, out, out_dram_offset); 
      } else {
        prefetch(in1, in2, buffer_in1_B, buffer_in2_B, in1_dram_offset, in2_dram_offset);
        comp(buffer_in1_A, buffer_in2_A, out, out_dram_offset);
      }

      /* Calculate DRAM offset. */

      out_dram_offset = ii * mat_dim + jj;
      sel = !sel;

    }
  }

  comp(buffer_in1_B, buffer_in2_B, out, out_dram_offset);

}

void prefetch(data_t *in1, data_t *in2, data_t buffer_in1[STRIPE_HEIGHT][MAT_DIM], data_t buffer_in2[STRIPE_HEIGHT][MAT_DIM], data_t in1_dram_offset, data_t in2_dram_offset)
{

  /* Constants. */

  const int mat_dim           = MAT_DIM;
  const int stripe_height     = STRIPE_HEIGHT;

  /* Fetch in1. */

  // read_in1_0: for(int i=0; i<stripe_height; i++){
  // #pragma HLS LOOP_TRIPCOUNT min=stripe_height max=stripe_height
  //   read_in1_1: for(int j=0; j<mat_dim; j++){
  //   #pragma HLS PIPELINE II=1
  //   #pragma HLS LOOP_TRIPCOUNT min=mat_dim max=mat_dim
  //     buffer_in1[i][j] = in1[i*mat_dim + j + in1_dram_offset];
  //   }
  // }

  // /* Fetch in2. */

  // read_in2_0: for(int i=0; i<stripe_height; i++){
  // #pragma HLS LOOP_TRIPCOUNT min=stripe_height max=stripe_height
  //   read_in2_1: for(int j=0; j<mat_dim; j++){
  //   #pragma HLS PIPELINE II=1
  //   #pragma HLS LOOP_TRIPCOUNT min=mat_dim max=mat_dim
  //     buffer_in2[i][j] = in2[i*mat_dim + j + in2_dram_offset];
  //   }
  // }

  read_in: for(int iter=0, i=0, j=0; iter < mat_dim*stripe_height; iter++, j++){
  #pragma HLS PIPELINE
  #pragma HLS LOOP_TRIPCOUNT min=mat_dim*stripe_height max=mat_dim*stripe_height
    if( j== mat_dim){ j = 0; i++; }
    buffer_in1[i][j] = in1[iter + in1_dram_offset];
    buffer_in2[i][j] = in2[iter + in2_dram_offset];
  }
}

void comp(data_t buffer_in1[STRIPE_HEIGHT][MAT_DIM], data_t buffer_in2[STRIPE_HEIGHT][MAT_DIM], data_t *out, data_t out_dram_offset)
{

  /* Constants. */

  const int mat_dim           = MAT_DIM;
  const int stripe_height     = STRIPE_HEIGHT;

  data_t buffer_out[stripe_height][stripe_height];

  /* Block processing. */

  comp_loop_1: for (int i = 0 ; i < stripe_height ; i++){
  #pragma HLS LOOP_TRIPCOUNT min=stripe_height max=stripe_height
    comp_loop_2: for(int j = 0 ; j < stripe_height ; j++){
    #pragma HLS LOOP_TRIPCOUNT min=stripe_height max=stripe_height
    #pragma HLS PIPELINE
      int res = 0;
      comp_loop_3: for(int k = 0; k < mat_dim; k++){
        res += buffer_in1[i][k]*buffer_in2[j][k];
      }
      buffer_out[i][j] = res;
    }
  }

  /* Write out to DRAM. */

  write_out: for(int iter = 0, i = 0, j = 0; iter < stripe_height * stripe_height; iter++, j++){
  #pragma HLS PIPELINE
  #pragma HLS LOOP_TRIPCOUNT min=stripe_height*stripe_height max=stripe_height*stripe_height
      if(j == stripe_height){ j = 0; i++; }
      out[j + i*mat_dim + out_dram_offset] = buffer_out[i][j];
  }

}
