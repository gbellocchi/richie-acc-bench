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

#ifndef MMULT_ACC_H
#define MMULT_ACC_H

#include <stdint.h>
using namespace std;

#include "ap_int.h"
typedef int32_t data_t;

/*
    Take a DATA_SIZE such that:
    - it is divisible by BLOCK_SIZE,
    - the BLOCK_SIZE can actually be allocated in the L1 memory when moving to HERO

    If Double buffering: L1 size (B) / 2 = 64x1024 = (DxBx2 + B^2)xsizeof(datatype) = Rectangular blocks + Squared output block, 
        where: 
            D (B) = data_size = matrix dimension (in Bytes)
            B (B) = block_size = block dimension (in Bytes)
*/

#define DATA_SIZE 512
#define BLOCK_SIZE 8

//Declaring the software function
void mmult_sw( data_t *in1, data_t *in2, data_t *out, data_t dim);

//Declaring the hardware function
void mmult_hw(data_t in1[DATA_SIZE], data_t in2[DATA_SIZE], data_t &out);

// mmul unit cell class
template <typename T0>
class systolic
{

private:
	T0 in1[512], in2[512];
	T0 out;

public:
	void exec(T0 in1[512], T0 in2[512], T0 &out)
	{
        T0 result = 0;
		for(int k = 0; k < 512; k++){
        #pragma HLS loop_tripcount min=512 max=512
            // printf("in1: %d, in2:%d \n", in1[k], in2[k]);
            result += in1[k] * in2[k];
        }
        // printf("out:%d \n", out);
        out = result;
	}
};

#endif
