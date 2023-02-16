/*
 * Copyright 2019 Xilinx, Inc.
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

#ifndef _XF_FAST_CONFIG_H_
#define _XF_FAST_CONFIG_H_

// Stream<-->Mat conversion (TB)
#if !defined (__SYNTHESIS__)
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#endif

#include "hls_stream.h"
#include "ap_int.h"

#include "common/xf_common.hpp"
#include "common/xf_utility.hpp"

// Stream<-->Mat conversion (HLS)
#include "common/xf_infra.hpp"

#if !defined (__SYNTHESIS__)
#include "common/xf_axi.hpp"
#endif

#include "imgproc/xf_inrange.hpp"

// Application
#include "features/xf_fast.hpp"
#include "xf_config_params.h"

// // Extra
// #include "common/xf_sw_utils.hpp"

// Resolve optimization type:
// #if RO
// #define NPC1 XF_NPPC8
// #define PTR_WIDTH 64
// #endif
// #if NO
#define NPC1 XF_NPPC1
#define PTR_WIDTH 32
// #endif

// Set the pixel depth:
#define TYPE XF_8UC1

// Streaming interface
typedef ap_axiu<PTR_WIDTH,1,1,1> interface_t;
typedef hls::stream<interface_t> stream_t;

// // Definition of ap_axiu
// template<int D,int U,int TI,int TD>
// struct ap_axiu{
// ap_uint<D>       data;
// ap_uint<(D+7)/8> keep;
// ap_uint<(D+7)/8> strb;
// ap_uint<U>       user;
// ap_uint<1>       last;
// ap_uint<TI>      id;
// ap_uint<TD>      dest;
// };

void fast_corner_detect(
    stream_t &img_in, 
    stream_t &img_out, 
    int threshold, 
    int rows, 
    int cols
);

#endif
