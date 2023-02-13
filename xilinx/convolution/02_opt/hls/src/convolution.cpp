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

/* Libraries. */

#include "convolution.h"

/*
 *
 * 2D convolution - Reference (body).
 *
 */

template<typename T, int K>
static void convolution_orig(
        int width, int height,
        const T *src, T *dst,
        const T *hcoeff, const T *vcoeff)
{
    // Convolution kernel size
    const int conv_size = K;
    // Half the convolution window - rounded down - i.e. the border width
    const int border_width = int(conv_size / 2);
#ifndef __SYNTHESIS__
    T * const local = new T[MAX_IMG_ROWS*MAX_IMG_COLS];
#else // Static storage allocation for HLS, dynamic otherwise
    T local[MAX_IMG_ROWS*MAX_IMG_COLS];
#endif

    // Clear local frame buffer
    Clear_Local:for(int i = 0; i < height * width; i++){
        local[i]=0;
    }
    // Horizontal convolution pass - makes O(K*K) reads from input image
    // per output pixel
    HconvH:for(int col = 0; col < height; col++){
        HconvW:for(int row = border_width; row < width - border_width; row++){
            Hconv:int pixel = col * width + row;
            for(int i = - border_width; i <= border_width; i++){
                local[pixel] += src[pixel + i] * hcoeff[i + border_width];
            }
        }
    }
    // Clear dst storage
    Clear_Dst:for(int i = 0; i < height * width; i++){
        dst[i]=0;
    }
    // Vertical convolution pass - makes O(K*K) reads from frame buffer -
    // resulting in only interior, i.e.
    // (border_width < col < height - border_width && border_width < row < width - border_width), pixels being valid
    VconvH:for(int col = border_width; col < height - border_width; col++){
        VconvW:for(int row = 0; row < width; row++){
            int pixel = col * width + row;
            Vconv:for(int i = - border_width; i <= border_width; i++){
                int offset = i * width;
                dst[pixel] += local[pixel + offset] * vcoeff[i + border_width];
            }
        }
    }
    // Populate borders by replicating adjacent valid pixels - uses a separate
    // set of loop nest for each vertical border region - top border; left/right
    // of valid vertical range; bottom. This is problematic for performance...
    int border_width_offset = border_width * width;
    int border_height_offset = (height - border_width - 1) * width;
    Top_Border:for(int col = 0; col < border_width; col++){
        int offset = col * width;
        Top_Left:for(int row = 0; row < border_width; row++){
            int pixel = offset + row;
            dst[pixel] = dst[border_width_offset + border_width];
        }
        Top_Row:for(int row = border_width; row < width - border_width; row++){
            int pixel = offset + row;
            dst[pixel] = dst[border_width_offset + row];
        }
        Top_Right:for(int row = width - border_width; row < width; row++){
            int pixel = offset + row;
            dst[pixel] = dst[border_width_offset + width - border_width - 1];
        }
    }
    Side_Border:for(int col = border_width; col < height - border_width; col++){
        int offset = col * width;
        Left_Col:for(int row = 0; row < border_width; row++){
            int pixel = offset + row;
            dst[pixel] = dst[offset + border_width];
        }
        Right_Col:for(int row = width - border_width; row < width; row++){
            int pixel = offset + row;
            dst[pixel] = dst[offset + width - border_width - 1];
        }
    }
    Bottom_Border:for(int col = height - border_width; col < height; col++){
        int offset = col * width;
        Bottom_Left:for(int row = 0; row < border_width; row++){
            int pixel = offset + row;
            dst[pixel] = dst[border_height_offset + border_width];
        }
        Bottom_Row:for(int row = border_width; row < width - border_width; row++){
            int pixel = offset + row;
            dst[pixel] = dst[border_height_offset + row];
        }
        Bottom_Right:for(int row = width - border_width; row < width; row++){
            int pixel = offset + row;
            dst[pixel] = dst[border_height_offset + width - border_width - 1];
        }
    }
}

/*
 *
 * 2D convolution - Streaming accelerator (body).
 *
 */

template<typename T, int K>
static void convolution_strm(
    int width, int height,
    hls::stream<T> &src, 
    hls::stream<T> &dst,
    const T *hcoeff, const T *vcoeff)
{
    /* Algorithm parameters. */
    const int border_width = int(K / 2);
    const int vconv_xlim = width - (K - 1);

    /* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

    /* Optimizations. */
    #pragma HLS INLINE // Into a DATAFLOW region

    /* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */
    
    /* Pixel windows (cache). */
    // Horizontal.
    T hwin[K];
    hls::stream<T> hconv("hconv");

    // Vertical.
    // T vwin[K];
    hls::stream<T> vconv("vconv");

    /* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

    /* Line buffers. */

    // Line-buffers allowing full pixel reuse in vertical pass.
    static T linebuf[K - 1][MAX_IMG_COLS];
    #pragma HLS ARRAY_PARTITION variable=linebuf dim=1 complete
    
    // Line-buffer for border pixel replication.
    T borderbuf[MAX_IMG_COLS - (K - 1)];

    // These assertions let HLS know the upper bounds of loops
    assert(height < MAX_IMG_ROWS);
    assert(width < MAX_IMG_COLS);
    assert(vconv_xlim < MAX_IMG_COLS - (K - 1));

    /* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

    /* 
        - Horizontal convolution -
        This consumes each pixel in source image
        exactly once, reusing values cached in hwin[], producing a stream
        of pixels required for the following vertical convolution.
    */

    HConv_A: for(int col = 0; col < height; col++) {
    #pragma HLS LOOP_TRIPCOUNT min=height max=height
        HConv_B: for(int row = 0; row < width; row++) {
        #pragma HLS LOOP_TRIPCOUNT min=width max=width
        #pragma HLS PIPELINE

            // Read input stream.
            T in_val = src.read();

            // Reset pixel value on-the-fly - eliminates an O(height*width) loop.
            T out_val = 0;
            HConv:for(int i = 0; i < K; i++) {
            #pragma HLS LOOP_TRIPCOUNT min=K max=K
                hwin[i] = i < K - 1 ? hwin[i + 1] : in_val;
                out_val += hwin[i] * hcoeff[i];
            }

            if (row >= K - 1)
                hconv << out_val;
        }
    }

    /* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

    /* 
        - Vertical convolution -
        This consumes stream generated by the horizontal
        pass; generates a stream of only the pixels in the valid interior
        region, i.e. (height - (K - 1)) * (width - (K - 1)) values.
    */

    VConv_A: for(int col = 0; col < height; col++) {
    #pragma HLS LOOP_TRIPCOUNT min=height max=height
        VConv_B: for(int row = 0; row < vconv_xlim; row++) {
        #pragma HLS LOOP_TRIPCOUNT min=vconv_xlim max=vconv_xlim
        #pragma HLS DEPENDENCE variable=linebuf inter false
        #pragma HLS PIPELINE

            // Read stream from HConv.
            T in_val = hconv.read();

            // Reset pixel value on-the-fly - eliminates an O(height*width) loop
            T out_val = 0;
            VConv:for(int i = 0; i < K; i++) {
            #pragma HLS LOOP_TRIPCOUNT min=K max=K
                T vwin_val = i < K - 1 ? linebuf[i][row] : in_val;
                out_val += vwin_val * vcoeff[i];
                if (i > 0)
                    linebuf[i - 1][row] = vwin_val;
            }
            if (col >= K - 1)
                vconv << out_val;
        }
    }

    /* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

    /* 
        - Borders -
        Handle border by replicating the exact same pixels as orig, but in
        a single loop taking the minimum (height*width) number of cycles.
    */
    
    border_A :for (int i = 0; i < height; i++) {
    #pragma HLS LOOP_TRIPCOUNT min=height max=height
        border_B: for (int j = 0; j < width; j++) {
        #pragma HLS LOOP_TRIPCOUNT min=width max=width
        #pragma HLS PIPELINE
            
            // Loop variables
            T pix_in, l_edge_pix, r_edge_pix, pix_out;
            
            if (i == 0 || (i > border_width && i < height - border_width)) {

                // read a pixel out of the input stream and cache it for
                // immediate use and later replication purposes
                if (j < width - (K - 1)) {
                    pix_in = vconv.read();
                    borderbuf[j] = pix_in;
                }
                if (j == 0) {
                    l_edge_pix = pix_in;
                }
                if (j == width - K) {
                    r_edge_pix = pix_in;
                }
            }

            // Select output value from the appropriate cache resource
            if (j <= border_width) {
                pix_out = l_edge_pix;
            } else if (j >= width - border_width - 1) {
                pix_out = r_edge_pix;
            } else {
                pix_out = borderbuf[j - border_width];
            }
            dst << pix_out;
        }
    }

    /* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */
}

/*
 *
 * 2D convolution - Reference (top).
 *
 */

void filter11x11_orig(int width, int height, const data_t *src, data_t *dst)
{

#pragma HLS INTERFACE m_axi port=src depth=32400 offset=slave bundle=port_src
#pragma HLS INTERFACE m_axi port=dst depth=32400 offset=slave bundle=port_dst

// #pragma HLS INTERFACE m_axi port=src depth=32400 bundle=port_src 
// #pragma HLS INTERFACE m_axi port=dst depth=32400 bundle=port_dst 

#pragma HLS INTERFACE s_axilite port=width  bundle=control 
#pragma HLS INTERFACE s_axilite port=height bundle=control 
#pragma HLS INTERFACE s_axilite port=return bundle=control 

#pragma HLS INLINE
#pragma HLS DATAFLOW  

    const data_t filt11_coeff[11] = {
        36, 111, 266, 498, 724, 821, 724, 498, 266, 111, 36
    };

    /* Image variables. */
    const int im_w = IM_UAV_COLS;
    const int im_h = IM_UAV_ROWS;

    convolution_orig<data_t, 11>(
        im_w, im_h,
        src, dst,
        filt11_coeff, filt11_coeff);
}

/*
 *
 * 2D convolution - Streaming accelerator (top).
 *
 */

void filter11x11_strm(
	hls::stream<data_t> &src, 
    hls::stream<data_t> &dst)
{

    /* Data streaming interface. */
    #pragma HLS INTERFACE axis port=&src 
    #pragma HLS INTERFACE axis port=&dst 

    /* Hardware optimizations. */
    #pragma HLS DATAFLOW
    #pragma HLS INLINE // bring loops in sub-functions to this DATAFLOW region

    /* Image variables. */
    const int im_w = IM_UAV_COLS;
    const int im_h = IM_UAV_ROWS;

    /* Filter components. */
    const data_t filter_coeffs[UAV_FILTER_DIM] = {
        36, 111, 266, 498, 724, 821, 724, 498, 266, 111, 36
    };

    /* Convolutional 2D filter. */
    convolution_strm<data_t, UAV_FILTER_DIM>(
        im_w, 
        im_h,
        src, dst,
        filter_coeffs, filter_coeffs);
}
