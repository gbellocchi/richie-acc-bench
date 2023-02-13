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

  data_t local_in1[stripe_height][mat_dim];
  data_t local_in2[stripe_height][mat_dim];
  data_t local_out[stripe_height][stripe_height]; 

  /* DRAM offsets. */

  data_t in1_dram_offset;
  data_t in2_dram_offset;
  data_t out_dram_offset;

  #pragma HLS ARRAY_PARTITION variable=local_in1 complete dim=2 
  #pragma HLS ARRAY_PARTITION variable=local_in2 complete dim=2 

  /* Matrix multiplication. */

  loop_A: for(int ii = 0; ii < mat_dim; ii += stripe_height){
  #pragma HLS LOOP_TRIPCOUNT min=fsm_calls max=fsm_calls

    /* Calculate DRAM offset. */

    in1_dram_offset = ii * mat_dim;

    loop_B: for(int jj = 0; jj < mat_dim; jj += stripe_height){
    #pragma HLS LOOP_TRIPCOUNT min=fsm_calls max=fsm_calls

      /* Calculate DRAM offset. */

      in2_dram_offset = jj * mat_dim;

      /* Prefetching. */

      read_in: for(int iter=0, i=0, j=0; iter < mat_dim*stripe_height; iter++, j++){
      #pragma HLS PIPELINE
      #pragma HLS LOOP_TRIPCOUNT min=mat_dim*stripe_height max=mat_dim*stripe_height
        if( j== mat_dim){ j = 0; i++; }
        local_in1[i][j] = in1[iter + in1_dram_offset];
        local_in2[i][j] = in2[iter + in2_dram_offset];
      }

      /* Block processing. */

      compute_1: for (int i = 0 ; i < stripe_height ; i++){
      #pragma HLS LOOP_TRIPCOUNT min=stripe_height max=stripe_height
        compute_2: for(int j = 0 ; j < stripe_height ; j++){
        #pragma HLS LOOP_TRIPCOUNT min=stripe_height max=stripe_height
        #pragma HLS PIPELINE
          data_t result = 0;
          compute_3: for(int k = 0; k < mat_dim; k++){
            result += local_in1[i][k]*local_in2[j][k];
          }
          local_out[i][j] = result;
        }
      }

      /* Calculate DRAM offset. */

      out_dram_offset = ii * mat_dim + jj;

      /* Write out to DRAM. */

      write_out: for(int iter = 0, i = 0, j = 0; iter < stripe_height * stripe_height; iter++, j++){
      #pragma HLS PIPELINE
      #pragma HLS LOOP_TRIPCOUNT min=stripe_height*stripe_height max=stripe_height*stripe_height
        if(j == stripe_height){ j = 0; i++; }
        out[i*mat_dim + j + out_dram_offset] = local_out[i][j];
      }

    }
  }
}
