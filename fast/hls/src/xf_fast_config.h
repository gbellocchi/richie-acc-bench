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

#include "hls_stream.h"
#include "ap_int.h"
#include "common/xf_common.hpp"
#include "common/xf_infra.hpp" // AXIvideo2xfMat, xfMat2AXIvideo
#include "common/xf_utility.hpp"
#include "features/xf_fast.hpp"
#include "xf_config_params.h"

#include "imgproc/xf_inrange.hpp"

// Resolve optimization type:
#if RO
#define NPC1 XF_NPPC8
#define PTR_WIDTH 64
#endif
#if NO
#define NPC1 XF_NPPC1
#define PTR_WIDTH 32
#endif

// Set the pixel depth:
#define TYPE XF_32UC1 //XF_8UC1

// Streaming interface
typedef ap_axiu<PTR_WIDTH,1,1,1> interface_t;
typedef hls::stream<interface_t> stream_t;

void fast_corner_detect(
    // Data
    stream_t &img_in, 
    stream_t &img_out, 
    // Parameters
    int threshold, 
    int rows, 
    int cols
);

#endif
