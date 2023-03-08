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

#ifndef _XF_INFRA_PATCH_H_
#define _XF_INFRA_PATCH_H_

#ifndef __cplusplus
#error C++ is needed to use this file!
#endif

namespace xf {
namespace cv {

/*
Unpack a AXI video stream into a xf::cv::Mat<> object
 *input: AXI_video_strm
 *output: img
 */

// PATCH_8_3_23: 
// - Pipeline style set to flp
// - Removed controls based on start and last. Not necessary in our wrapper, as soon as internal HLS blocks do not require them.

template <int W, int TYPE, int ROWS, int COLS, int NPPC, int XFCVDEPTH>
int AXIvideo2xfMat_patch(hls::stream<ap_axiu<W, 1, 1, 1> >& AXI_video_strm,
                   xf::cv::Mat<TYPE, ROWS, COLS, NPPC, XFCVDEPTH>& img) {
    ap_axiu<W, 1, 1, 1> axi;
    int res = 0;

    const int m_pix_width = XF_PIXELWIDTH(TYPE, NPPC) * XF_NPIXPERCYCLE(NPPC);

    int rows = img.rows;
    int cols = img.cols >> XF_BITSHIFT(NPPC);
    int idx = 0;

    assert(img.rows <= ROWS);
    assert(img.cols <= COLS);

    // bool start = false;
    // bool last = false;

// loop_start_hunt:
//     while (!start) {
// // clang-format off
// #pragma HLS pipeline II=1
// #pragma HLS loop_tripcount avg=1 max=1
//         // clang-format on

//         AXI_video_strm >> axi;
//         start = axi.user.to_bool();
//     }

loop_row_axi2mat:
    for (int i = 0; i < rows; i++) {
        // last = false;

    loop_col_zxi2mat:
        for (int j = 0; j < cols; j++) {
// clang-format off
#pragma HLS loop_flatten off
#pragma HLS pipeline II=1 style=flp
            // clang-format on

            // if (start || last) {
            //     start = false;
            // } else {
            //     AXI_video_strm >> axi;

            //     bool user = axi.user.to_int();
            //     if (user) {
            //         res |= ERROR_IO_SOF_EARLY;
            //     }
            // }
            // if (last && (j != img.cols - 1)) { // checking end of each row
            //     res |= ERROR_IO_EOL_EARLY;
            // }

            // last = axi.last.to_bool();

            // img.write(idx++, axi.data(m_pix_width - 1, 0));

            AXI_video_strm >> axi;
            img.write(idx++, axi.data(m_pix_width - 1, 0));
        }

//     loop_last_hunt:
//         while (!last) {
// // clang-format off
// #pragma HLS pipeline II=1
// #pragma HLS loop_tripcount avg=1 max=1
//             // clang-format on

//             AXI_video_strm >> axi;
//             last = axi.last.to_bool();
//             res |= ERROR_IO_EOL_LATE;
//         }
    }

    return res;
}

// Pack the data of a xf::cv::Mat<> object into an AXI Video stream
/*
 *  input: img
 *  output: AXI_video_strm
 */

// PATCH_8_3_23: 
// - Pipeline style set to flp
// - Removed use of sof (start of frame)

template <int W, int TYPE, int ROWS, int COLS, int NPPC, int XFCVDEPTH>
int xfMat2AXIvideo_patch(xf::cv::Mat<TYPE, ROWS, COLS, NPPC, XFCVDEPTH>& img,
                   hls::stream<ap_axiu<W, 1, 1, 1> >& AXI_video_strm) {
    ap_axiu<W, 1, 1, 1> axi;
    int res = 0;

    int rows = img.rows;
    int cols = img.cols >> XF_BITSHIFT(NPPC);
    int idx = 0;

    assert(img.rows <= ROWS);
    assert(img.cols <= COLS);

    const int m_pix_width = XF_PIXELWIDTH(TYPE, NPPC) * XF_NPIXPERCYCLE(NPPC);

    // bool sof = true; // Indicates start of frame

loop_row_mat2axi:
    for (int i = 0; i < rows; i++) {
    loop_col_mat2axi:
        for (int j = 0; j < cols; j++) {
// clang-format off
#pragma HLS loop_flatten off
#pragma HLS pipeline II=1 style=flp
            // clang-format on

            // if (sof) {
            //     axi.user = 1;
            // } else {
            //     axi.user = 0;
            // }

            // sof = false;

            if (j == cols - 1) {
                axi.last = 1;
            } else {
                axi.last = 0;
            }

            axi.keep = -1;

            axi.data = 0;
            axi.data(m_pix_width - 1, 0) = img.read(i*rows + j);
            AXI_video_strm.write(axi);
        }
    }

    return res;
}

} // namespace cv
} // namespace xf

#endif
