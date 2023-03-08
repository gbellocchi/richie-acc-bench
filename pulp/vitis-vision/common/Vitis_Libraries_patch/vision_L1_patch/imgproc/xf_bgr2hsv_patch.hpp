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

#ifndef _XF_BGR2HSV_PATCH_HPP_
#define _XF_BGR2HSV_PATCH_HPP_

#include "imgproc/xf_bgr2hsv.hpp"

namespace xf {
namespace cv {

template <int SRC_T,
          int ROWS,
          int COLS,
          int NPC,
          int XFCVDEPTH_IN_1,
          int XFCVDEPTH_OUT_1>
void bgr2hsv_patch(xf::cv::Mat<SRC_T, ROWS, COLS, NPC, XFCVDEPTH_IN_1>& _src_mat,
             xf::cv::Mat<SRC_T, ROWS, COLS, NPC, XFCVDEPTH_OUT_1>& _dst_mat) {
// clang-format off
#pragma HLS INLINE OFF
    // clang-format on

    int hdiv[256] = {
        0,    122880, 61440, 40960, 30720, 24576, 20480, 17554, 15360, 13653, 12288, 11171, 10240, 9452, 8777, 8192,
        7680, 7228,   6827,  6467,  6144,  5851,  5585,  5343,  5120,  4915,  4726,  4551,  4389,  4237, 4096, 3964,
        3840, 3724,   3614,  3511,  3413,  3321,  3234,  3151,  3072,  2997,  2926,  2858,  2793,  2731, 2671, 2614,
        2560, 2508,   2458,  2409,  2363,  2318,  2276,  2234,  2194,  2156,  2119,  2083,  2048,  2014, 1982, 1950,
        1920, 1890,   1862,  1834,  1807,  1781,  1755,  1731,  1707,  1683,  1661,  1638,  1617,  1596, 1575, 1555,
        1536, 1517,   1499,  1480,  1463,  1446,  1429,  1412,  1396,  1381,  1365,  1350,  1336,  1321, 1307, 1293,
        1280, 1267,   1254,  1241,  1229,  1217,  1205,  1193,  1182,  1170,  1159,  1148,  1138,  1127, 1117, 1107,
        1097, 1087,   1078,  1069,  1059,  1050,  1041,  1033,  1024,  1016,  1007,  999,   991,   983,  975,  968,
        960,  953,    945,   938,   931,   924,   917,   910,   904,   897,   890,   884,   878,   871,  865,  859,
        853,  847,    842,   836,   830,   825,   819,   814,   808,   803,   798,   793,   788,   783,  778,  773,
        768,  763,    759,   754,   749,   745,   740,   736,   731,   727,   723,   719,   714,   710,  706,  702,
        698,  694,    690,   686,   683,   679,   675,   671,   668,   664,   661,   657,   654,   650,  647,  643,
        640,  637,    633,   630,   627,   624,   621,   617,   614,   611,   608,   605,   602,   599,  597,  594,
        591,  588,    585,   582,   580,   577,   574,   572,   569,   566,   564,   561,   559,   556,  554,  551,
        549,  546,    544,   541,   539,   537,   534,   532,   530,   527,   525,   523,   521,   518,  516,  514,
        512,  510,    508,   506,   504,   502,   500,   497,   495,   493,   492,   490,   488,   486,  484,  482};
    int sdiv[256] = {
        0,     1044480, 522240, 348160, 261120, 208896, 174080, 149211, 130560, 116053, 104448, 94953, 87040, 80345,
        74606, 69632,   65280,  61440,  58027,  54973,  52224,  49737,  47476,  45412,  43520,  41779, 40172, 38684,
        37303, 36017,   34816,  33693,  32640,  31651,  30720,  29842,  29013,  28229,  27486,  26782, 26112, 25475,
        24869, 24290,   23738,  23211,  22706,  22223,  21760,  21316,  20890,  20480,  20086,  19707, 19342, 18991,
        18651, 18324,   18008,  17703,  17408,  17123,  16846,  16579,  16320,  16069,  15825,  15589, 15360, 15137,
        14921, 14711,   14507,  14308,  14115,  13926,  13743,  13565,  13391,  13221,  13056,  12895, 12738, 12584,
        12434, 12288,   12145,  12006,  11869,  11736,  11605,  11478,  11353,  11231,  11111,  10995, 10880, 10768,
        10658, 10550,   10445,  10341,  10240,  10141,  10043,  9947,   9854,   9761,   9671,   9582,  9495,  9410,
        9326,  9243,    9162,   9082,   9004,   8927,   8852,   8777,   8704,   8632,   8561,   8492,  8423,  8356,
        8290,  8224,    8160,   8097,   8034,   7973,   7913,   7853,   7795,   7737,   7680,   7624,  7569,  7514,
        7461,  7408,    7355,   7304,   7253,   7203,   7154,   7105,   7057,   7010,   6963,   6917,  6872,  6827,
        6782,  6739,    6695,   6653,   6611,   6569,   6528,   6487,   6447,   6408,   6369,   6330,  6292,  6254,
        6217,  6180,    6144,   6108,   6073,   6037,   6003,   5968,   5935,   5901,   5868,   5835,  5803,  5771,
        5739,  5708,    5677,   5646,   5615,   5585,   5556,   5526,   5497,   5468,   5440,   5412,  5384,  5356,
        5329,  5302,    5275,   5249,   5222,   5196,   5171,   5145,   5120,   5095,   5070,   5046,  5022,  4998,
        4974,  4950,    4927,   4904,   4881,   4858,   4836,   4813,   4791,   4769,   4748,   4726,  4705,  4684,
        4663,  4642,    4622,   4601,   4581,   4561,   4541,   4522,   4502,   4483,   4464,   4445,  4426,  4407,
        4389,  4370,    4352,   4334,   4316,   4298,   4281,   4263,   4246,   4229,   4212,   4195,  4178,  4161,
        4145,  4128,    4112,   4096};

    XF_SNAME(XF_WORDWIDTH(SRC_T, NPC)) in_pix;
    XF_SNAME(XF_WORDWIDTH(SRC_T, NPC)) out_pix;
    ap_uint<8> r, g, b;

    int rows = _src_mat.rows;
    int cols = (_src_mat.cols >> XF_BITSHIFT(NPC));
    int h;
    int s, v;
    ap_uint<8> vmin;
    ap_uint<8> diff;
    int vr, vg;
    const int STEP = XF_DTPIXELDEPTH(SRC_T, NPC);
    for (uint16_t row = 0; row < rows; row++) {
// clang-format off
#pragma HLS LOOP_TRIPCOUNT max=ROWS
        // clang-format on
        for (uint16_t col = 0; col < cols; col++) {
// clang-format off
#pragma HLS LOOP_TRIPCOUNT max=COLS
#pragma HLS pipeline style=flp
            // clang-format on
            in_pix = _src_mat.read(row * cols + col);

            for (int p = 0; p < (XF_NPIXPERCYCLE(NPC) * XF_CHANNELS(SRC_T, NPC)); p = p + XF_CHANNELS(SRC_T, NPC)) {
                b = in_pix.range(p * STEP + STEP - 1, p * STEP);
                g = in_pix.range(p * STEP + (2 * STEP) - 1, p * STEP + STEP);
                r = in_pix.range(p * STEP + (3 * STEP) - 1, p * STEP + 2 * STEP);

                v = b;
                vmin = b;

                CV_CALC_MAX_8U(v, g);
                CV_CALC_MAX_8U(v, r);
                CV_CALC_MIN_8U(vmin, g);
                CV_CALC_MIN_8U(vmin, r);

                diff = v - vmin;
                vr = v == r ? -1 : 0;
                vg = v == g ? -1 : 0;

                s = (diff * sdiv[v] + (1 << (11))) >> 12;
                h = (vr & (g - b)) + (~vr & ((vg & (b - r + 2 * diff)) + ((~vg) & (r - g + 4 * diff))));
                h = (h * hdiv[diff] + (1 << (11))) >> 12;
                h += h < 0 ? 180 : 0;

                out_pix.range(p * STEP + STEP - 1, p * STEP) = (unsigned char)(h);
                out_pix.range(p * STEP + (2 * STEP) - 1, p * STEP + STEP) = (unsigned char)(s);
                out_pix.range(p * STEP + (3 * STEP) - 1, p * STEP + 2 * STEP) = (unsigned char)(v);
            }

            _dst_mat.write(row * cols + col, (out_pix));
        }
    }
}
} // namespace cv
} // namespace xf
#endif
