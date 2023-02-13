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

void mmult_hw(data_t in1[BLOCK_SIZE * DATA_SIZE], data_t in2[DATA_SIZE * BLOCK_SIZE], data_t out[BLOCK_SIZE * BLOCK_SIZE])
{

    #pragma HLS INTERFACE axis port=in1
    #pragma HLS INTERFACE axis port=in2
    #pragma HLS INTERFACE axis port=out

    const int mat_size = DATA_SIZE;
    const int block_size = BLOCK_SIZE;

    int local_in1[BLOCK_SIZE][DATA_SIZE];
    int local_in2[DATA_SIZE][BLOCK_SIZE];
    int local_out[BLOCK_SIZE][BLOCK_SIZE]; 

    #pragma HLS ARRAY_PARTITION variable=local_in1 complete dim=2 
    #pragma HLS ARRAY_PARTITION variable=local_in2 complete dim=1 

    read_in: for(int iter = 0, i=0, j=0; iter< mat_size*block_size; iter++,j++){
    #pragma HLS PIPELINE
    #pragma HLS LOOP_TRIPCOUNT min=mat_size*block_size max=mat_size*block_size
        if( j== mat_size){ j = 0; i++; }
        local_in1[i][j] = in1[iter];
        local_in2[j][i] = in2[j*mat_size + i]; 
    }

    loop_1: for (int i = 0 ; i < block_size ; i++){
    #pragma HLS LOOP_TRIPCOUNT min=block_size max=block_size
        loop_2: for(int j = 0 ; j < block_size ; j++){
        #pragma HLS LOOP_TRIPCOUNT min=block_size max=block_size
        #pragma HLS PIPELINE
            int res = 0;
            loop_3: for(int k = 0; k < mat_size; k++){
                res += local_in1[i][k]*local_in2[k][j];
            }
            local_out[i][j] = res;
        }
    }

    write_out: for(int iter = 0, i = 0, j = 0; iter < block_size * block_size; iter++, j++){
    #pragma HLS PIPELINE
    #pragma HLS LOOP_TRIPCOUNT min=block_size*block_size max=block_size*block_size
        if(j == block_size){ j = 0; i++; }
        out[j + i*mat_size] = local_out[i][j];
    }
    
}
