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
    ap_uint<PTR_WIDTH>* img_in, 
    ap_uint<PTR_WIDTH>* img_out, 
    unsigned char threshold, 
    int rows, 
    int cols
) {
    #pragma HLS INTERFACE axis register register_mode=off port=img_in
    #pragma HLS INTERFACE axis register register_mode=off port=img_out

    // #pragma HLS INTERFACE axis port=threshold

    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPC1> imgInput(rows, cols);
    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPC1> imgOutput(rows, cols);

    #pragma HLS DATAFLOW

    // common
    xf::cv::accel_utils utils;
    const int ch_width = XF_DTPIXELDEPTH(TYPE, NPC1);

    // Array2xfMat - Conversion from input array -> stream -> Mat
    hls::stream<ap_uint<PTR_WIDTH> > stream_img_in;
    int imgInput_cols_align_npc = ((cols + (NPC1 - 1)) >> XF_BITSHIFT(NPC1)) << XF_BITSHIFT(NPC1);

    utils.Array2hlsStrm<PTR_WIDTH, HEIGHT, WIDTH, NPC1, XF_CHANNELS(TYPE, NPC1), ch_width,
                    ((HEIGHT * WIDTH * XF_CHANNELS(TYPE, NPC1) * ch_width) / PTR_WIDTH)>(img_in, stream_img_in, rows, cols);
    utils.hlsStrm2xfMat<PTR_WIDTH, TYPE, HEIGHT, WIDTH, NPC1, (HEIGHT * WIDTH) / NPC1>(stream_img_in, imgInput, imgInput_cols_align_npc);

    // // AXIvideo2xfMat - Conversion from stream -> Mat
    // xf::cv::AXIvideo2xfMat<PTR_WIDTH, TYPE, HEIGHT, WIDTH, NPC1>(stream_img_in, imgInput);

    // Run xfOpenCV kernel:
    xf::cv::fast<NMS, TYPE, HEIGHT, WIDTH, NPC1>(imgInput, imgOutput, threshold);

    // // xfMat2AXIvideo - Conversion from Mat -> stream
    // xf::cv::xfMat2AXIvideo<PTR_WIDTH, TYPE, HEIGHT, WIDTH, NPC1>(imgOutput, stream_img_out);

    // xfMat2Array - Conversion from Mat -> stream -> input array
    hls::stream<ap_uint<PTR_WIDTH> > stream_img_out;
    int imgOutput_cols_align_npc = ((cols + (NPC1 - 1)) >> XF_BITSHIFT(NPC1)) << XF_BITSHIFT(NPC1);

    utils.xfMat2hlsStrm<PTR_WIDTH, TYPE, HEIGHT, WIDTH, NPC1, HEIGHT *((WIDTH + NPC1 - 1) / NPC1)>(imgOutput, stream_img_out,
                                                                                    imgOutput_cols_align_npc);
    utils.hlsStrm2Array<PTR_WIDTH, HEIGHT, WIDTH, NPC1, XF_CHANNELS(TYPE, NPC1), ch_width,
                    ((HEIGHT * WIDTH * XF_CHANNELS(TYPE, NPC1) * ch_width) / PTR_WIDTH)>(stream_img_out, img_out, rows, cols);

    return;
} // End of kernel
