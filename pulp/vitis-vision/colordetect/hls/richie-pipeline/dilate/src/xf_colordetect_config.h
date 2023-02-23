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

#ifndef _XF_COLORDETECT_CONFIG_H_
#define _XF_COLORDETECT_CONFIG_H_

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

// #include "imgproc/xf_inrange.hpp"

// Application
#include "imgproc/xf_channel_combine.hpp"
#include "imgproc/xf_colorthresholding.hpp"
#include "imgproc/xf_bgr2hsv.hpp"
#include "imgproc/xf_erosion.hpp"
#include "imgproc/xf_dilation.hpp"
//#include "xf_config_params.h"

//#define MAXCOLORS 3

/* Set the image height and width */
#define HEIGHT 128
#define WIDTH 128

/* Color thresholding parameters */
#define MAXCOLORS 3

/* Erode and Dilate parameters */
#define FILTER_SIZE 3
#define KERNEL_SHAPE 0 // 0 - rectangle, 1 - ellipse, 2 - cross
#define ITERATIONS 1

// Set the optimization type:
#define NPC1 XF_NPPC1

// Set the input and output pixel depth:
#define IN_TYPE XF_8UC3
#define OUT_TYPE XF_8UC1
#define PULP_PTR_WIDTH 32
#define INPUT_PTR_WIDTH 32
#define OUTPUT_PTR_WIDTH 8

// Resolve mask shape:
#if KERNEL_SHAPE == 0
#define XF_KERNEL_SHAPE XF_SHAPE_RECT
#elif KERNEL_SHAPE == 1
#define XF_KERNEL_SHAPE XF_SHAPE_ELLIPSE
#elif KERNEL_SHAPE == 2
#define XF_KERNEL_SHAPE XF_SHAPE_CROSS
#else
#define XF_KERNEL_SHAPE XF_SHAPE_RECT
#endif

// Streaming interface
typedef ap_axiu<INPUT_PTR_WIDTH,1,1,1> interface_in_t;
typedef hls::stream<interface_in_t> stream_in_t;

typedef ap_axiu<OUTPUT_PTR_WIDTH,1,1,1> interface_out_t;
typedef hls::stream<interface_out_t> stream_out_t;

void dilate_cv(
    stream_out_t &img_in, 
    stream_out_t &img_out, 
    int rows,
    int cols
);

#endif