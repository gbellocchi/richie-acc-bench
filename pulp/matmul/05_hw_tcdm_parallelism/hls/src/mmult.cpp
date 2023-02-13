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

/* Libraries. */

#include "mmult.h"

/*
 *
 * Matrix multiplication - SW execution.
 *
 */

void mmult_sw( data_t *in1,   
               data_t *in2,   
               data_t *out,   
               data_t mat_dim   
             )
{
    for (data_t i = 0; i < mat_dim; i++){
        for (data_t j = 0; j < mat_dim; j++){
            for (data_t k = 0; k < mat_dim; k++){
                out[i * mat_dim + j] += in1[i * mat_dim + k] * in2[k * mat_dim  + j];
            }
        }
    }
}

/*
 *
 * Matrix multiplication - HW execution.
 *
 */

void mmult_hw(data_t in1[DATA_SIZE], data_t in2[DATA_SIZE], data_t &out)
{

    /* Interface declaration. */

    #pragma HLS INTERFACE axis port=in1
    #pragma HLS INTERFACE axis port=in2
    #pragma HLS INTERFACE axis port=out

    #pragma HLS array_partition variable=in1 cyclic factor=16
    #pragma HLS array_partition variable=in2 cyclic factor=16

    /* Constants. */

    const int mat_size = DATA_SIZE;

    /* Matrix multiplication. */

    data_t res = 0;

    loop_A: for(int k = 0; k < mat_size; k+=16){
    #pragma HLS LOOP_TRIPCOUNT min=32 max=32
    #pragma HLS PIPELINE
        loop_B: for(int j = 0; j < 16; j++){
        #pragma HLS LOOP_TRIPCOUNT min=16 max=16
            res += in1[j+k]*in2[j+k];
        }
    }
    
    out = res;
    
}
