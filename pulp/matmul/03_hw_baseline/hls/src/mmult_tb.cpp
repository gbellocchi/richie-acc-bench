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

/*****
* Changes from Lab 01:
In this example, matrix multiplication is performed on the hardware and 
the results are compared with the results of software matrix multiplication. 
*****/

#include <iostream>
#include <stdlib.h>
// #include "sds_utils.h"
#include "mmult.h"

#ifndef NUM_TIMES
#define NUM_TIMES 2  
#endif

int main(int argc, char** argv)
{   
    int mat_dim =  DATA_SIZE;
    int block_dim =  BLOCK_SIZE;

    size_t square_matrix_size_bytes = sizeof(int) * mat_dim * mat_dim;
    size_t rect_matrix_size_bytes = sizeof(int) * mat_dim * block_dim;

    data_t *in1 = (int *) malloc(square_matrix_size_bytes);
    data_t *in2 = (int *) malloc(square_matrix_size_bytes);
    data_t out;
    data_t *hw_result = (int *) malloc(square_matrix_size_bytes);
    data_t *sw_result = (int *) malloc(square_matrix_size_bytes);

    bool match = true;

    //Create test data
    for (int i = 0; i < mat_dim * mat_dim; i++) {
        in1[i] = 1;
        in2[i] = 1;
        sw_result[i] = 0;
        hw_result[i] = 0;
    }
        
    //Launch the software solution
    mmult_sw( in1, in2, sw_result, mat_dim);

    //Launch the Hardware solution
    for(int i = 0; i < mat_dim; i += block_dim){
        for(int j = 0; j < mat_dim; j += block_dim){
            for(int ii = 0; ii < block_dim; ii++){
                for(int jj = 0; jj < block_dim; jj++){
                    mmult_hw( in1 + i*mat_dim + ii*block_dim, in2 + j + jj, out );
                    hw_result[i*mat_dim + j + ii*mat_dim + jj] = out;
                }
            }   
        }
    }

    // Compare the results of hardware to the software 
    for(int i=0; i< mat_dim * mat_dim; i++)
    {
        if( sw_result[i] != hw_result[i] )
        {
            std::cout << "Results Mismatch on " << "Row:" << i/mat_dim << "Col:" << i - (i/mat_dim)*mat_dim << std::endl;
            std::cout << "CPU output:" << sw_result[i] <<"\t Hardware output:" << hw_result[i] << std::endl;
            match = false;
            break;
        }
    }

    free(in1);
    free(in2);
    free(hw_result);
    free(sw_result);

    std::cout << "\n\nTEST " << (match? "PASSED\n\n": "FAILED\n\n") << std::endl;
    return(match? EXIT_SUCCESS: EXIT_FAILURE);
}



