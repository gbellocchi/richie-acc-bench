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

#include "xf_fast_config.h"

static constexpr int __XF_DEPTH = (HEIGHT * WIDTH * (XF_PIXELWIDTH(TYPE, NPC1)) / 8) / (PTR_WIDTH / 8);

void fast_corner_detect(
    stream_t &img_in, 
    stream_t &img_out, 
    int threshold, 
    int rows, 
    int cols
) {
    #pragma HLS INTERFACE axis register both port=img_in depth=__XF_DEPTH
    #pragma HLS INTERFACE axis register both port=img_out depth=__XF_DEPTH

    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPC1> imgInput(HEIGHT, WIDTH);
    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPC1> imgOutput(HEIGHT, WIDTH);

    #pragma HLS DATAFLOW

    // Convert stream in to xf::cv::Mat
    xf::cv::AXIvideo2xfMat<PTR_WIDTH, TYPE, HEIGHT, WIDTH, NPC1>(img_in, imgInput);

    // Run xfOpenCV kernel:
    xf::cv::fast<NMS, TYPE, HEIGHT, WIDTH, NPC1>(imgInput, imgOutput, threshold);

    // Convert xf::cv::Mat object to output stream:
    xf::cv::xfMat2AXIvideo<PTR_WIDTH, TYPE, HEIGHT, WIDTH, NPC1>(imgOutput, img_out);
}
