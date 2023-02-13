/*
 * Copyright 2020 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CONVOLUTION_H_
#define CONVOLUTION_H_

#include <assert.h>
#include <stdint.h>
#include <hls_stream.h>

/* Original parameters */

#define MAX_IMG_ROWS 1080
#define MAX_IMG_COLS 1920

#define TEST_IMG_ROWS 135
#define TEST_IMG_COLS 240
#define TEST_IMG_SIZE (TEST_IMG_ROWS * TEST_IMG_COLS)

/* 
    Modified terms on the basis of what is possible on
    our FPGA overlay.

    If Double buffering: L1 size (B) / 2 = (128x1024) / 2
                                         = 64x1024
                                         = (BLOCK_IMAGE_IN + BLOCK_IMAGE_OUT)xsizeof(datatype) 
                                         = Rectangular blocks + Squared output block, 
        where: 
            D (B) = data_size = matrix dimension (in Bytes)
            B (B) = block_size = block dimension (in Bytes)
*/

#define IM_UAV_ROWS 320 // Inpired by AI-deck (nano-sized) 
#define IM_UAV_COLS 320 // Inpired by AI-deck (nano-sized)

#define UAV_DATA_SIZE UAV_ROWS*UAV_COLS

#define UAV_FILTER_DIM 11 // Window size

typedef uint32_t data_t;

// External function prototypes
void filter11x11_orig(
        int w, int h,
        const data_t *src_image, data_t *dst_image);

void filter11x11_strm(
        int w, 
        int h,
        data_t filter_coeffs[UAV_FILTER_DIM],
        hls::stream<data_t> &src_image, 
        hls::stream<data_t> &dst_image);

#endif // CONVOLUTION_H_ not defined

