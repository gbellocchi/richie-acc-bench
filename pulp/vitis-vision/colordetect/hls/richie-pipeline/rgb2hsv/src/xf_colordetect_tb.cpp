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

#include "common/xf_headers.hpp"
#include "xf_colordetect_config.h"

#include <gen_Hfile.h> // to generate header files for stimuli, golden data, etc.

/* ===================================================================== */

// OpenCV reference function
void colordetect_ref(cv::Mat& _src, cv::Mat& _dst, unsigned char* nLowThresh, unsigned char* nHighThresh) {
    // Temporary matrices for processing
    cv::Mat mask1, mask2, mask3, _imgrange, _imghsv;

    // Convert the input to the HSV colorspace. Using BGR here since it is the default of OpenCV.
    // Using RGB yields different results, requiring a change of the threshold ranges
    cv::cvtColor(_src, _imghsv, cv::COLOR_BGR2HSV);

    // Get the color of Yellow from the HSV image and store it as a mask
    cv::inRange(_imghsv, cv::Scalar(nLowThresh[0], nLowThresh[1], nLowThresh[2]),
                cv::Scalar(nHighThresh[0], nHighThresh[1], nHighThresh[2]), mask1);

    // Get the color of Green from the HSV image and store it as a mask
    cv::inRange(_imghsv, cv::Scalar(nLowThresh[3], nLowThresh[4], nLowThresh[5]),
                cv::Scalar(nHighThresh[3], nHighThresh[4], nHighThresh[5]), mask2);

    // Get the color of Red from the HSV image and store it as a mask
    cv::inRange(_imghsv, cv::Scalar(nLowThresh[6], nLowThresh[7], nLowThresh[8]),
                cv::Scalar(nHighThresh[6], nHighThresh[7], nHighThresh[8]), mask3);

    // Bitwise OR the masks together (adding them) to the range
    _imgrange = mask1 | mask2 | mask3;

    cv::Mat element = cv::getStructuringElement(0, cv::Size(3, 3), cv::Point(-1, -1));
    cv::erode(_imgrange, _dst, element, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT);

    cv::dilate(_dst, _dst, element, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT);

    cv::dilate(_dst, _dst, element, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT);

    cv::erode(_dst, _dst, element, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT);
}

/* ===================================================================== */

// Color detect DUT wrapper 
void colordetect_dut_wrapper(cv::Mat& _src, cv::Mat& _dst, int rows, int cols, unsigned char* low_thresh, unsigned char* high_thresh, unsigned char* process_shape) {
    // IO streams
    stream_in_t stream_dut_in("stream_dut_in");
    stream_in_t stream_dut_out("stream_dut_out");
    stream_out_t stream_dst("stream_dst");

    // Mat
    xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, NPC1> _rgb2hsv(rows, cols);
    xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPC1> _imgHelper1(rows, cols);
    xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPC1> _imgHelper2(rows, cols);
    xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPC1> _imgHelper3(rows, cols);
    xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPC1> _imgHelper4(rows, cols);
    xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPC1> _imgOutput(rows, cols);

    // Convert Mat to Stream
    cvMat2AXIvideoxf<NPC1>(_src, stream_dut_in);

    // Execute DUT
    rgb2hsv(
        (stream_in_t &)stream_dut_in, 
        (stream_in_t &)stream_dut_out,
        rows, 
        cols
    );

    // Retrieve DUT output stream and convert to xf::cv::Mat object
    xf::cv::AXIvideo2xfMat<INPUT_PTR_WIDTH, IN_TYPE, HEIGHT, WIDTH, NPC1>(stream_dut_out, _rgb2hsv);

    // Execute remaining pipeline stages
    xf::cv::colorthresholding<IN_TYPE, OUT_TYPE, MAXCOLORS, HEIGHT, WIDTH, NPC1>(_rgb2hsv, _imgHelper1, low_thresh, high_thresh);
    xf::cv::erode<XF_BORDER_CONSTANT, OUT_TYPE, HEIGHT, WIDTH, XF_KERNEL_SHAPE, FILTER_SIZE, FILTER_SIZE, ITERATIONS, NPC1>(_imgHelper1, _imgHelper2, process_shape);
    xf::cv::dilate<XF_BORDER_CONSTANT, OUT_TYPE, HEIGHT, WIDTH, XF_KERNEL_SHAPE, FILTER_SIZE, FILTER_SIZE, ITERATIONS, NPC1>(_imgHelper2, _imgHelper3, process_shape);
    xf::cv::dilate<XF_BORDER_CONSTANT, OUT_TYPE, HEIGHT, WIDTH, XF_KERNEL_SHAPE, FILTER_SIZE, FILTER_SIZE, ITERATIONS, NPC1>(_imgHelper3, _imgHelper4, process_shape);
    xf::cv::erode<XF_BORDER_CONSTANT, OUT_TYPE, HEIGHT, WIDTH, XF_KERNEL_SHAPE, FILTER_SIZE, FILTER_SIZE, ITERATIONS, NPC1>(_imgHelper4, _imgOutput, process_shape);

    // Convert from xf::cv::Mat to cv::Mat
    xf::cv::xfMat2AXIvideo<OUTPUT_PTR_WIDTH, OUT_TYPE, HEIGHT, WIDTH, NPC1>(_imgOutput, stream_dst);
    AXIvideo2cvMatxf<NPC1, OUTPUT_PTR_WIDTH>(stream_dst, _dst);
}

/* ===================================================================== */

// Verification 
float colordetect_verif(cv::Mat& out_ref, cv::Mat& out_dut) {
    cv::Mat diff;
    int cnt = 0;

    cv::absdiff(out_ref, out_dut, diff);

    for (int i = 0; i < diff.rows; ++i) {
        for (int j = 0; j < diff.cols; ++j) {   
            uchar v = diff.at<uchar>(i, j);
            if (v > 0) cnt++;
        }
    }

    float err_per = 100.0 * (float)cnt / (diff.rows * diff.cols);

    std::cout << "INFO: Verification results:" << std::endl;
    std::cout << "\tPercentage of pixels above error threshold = " << err_per << "%" << std::endl;

    return err_per;
}

/* ===================================================================== */

// Write out image in header file
void write_mat_out(cv::Mat& img_mat, char* img_name, int ptr_width){
    char* img_array = (char*)malloc(img_mat.rows*img_mat.cols*sizeof(char));

    // Convert Mat to array
	for (int y = 0; y < img_mat.rows; y++){
		for (int x = 0; x < img_mat.cols; x++){
			img_array[y * img_mat.rows + x] = img_mat.at<char>(y, x);
		}
	}

    // Create output header files
    gen_Hfile(img_name, img_array, img_mat.rows, img_mat.cols, ptr_width);
}

/* ===================================================================== */

int main(int argc, char** argv) {
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <INPUT IMAGE PATH 1> <INPUT IMAGE PATH 2>\n", argv[0]);
        return EXIT_FAILURE;
    }

    cv::Mat in_img, img_rgba, out_img, ocv_ref, out_pipeline_ref;

    // Open input image
    in_img = cv::imread(argv[1], 1);
    if (!in_img.data) {
        fprintf(stderr, "ERROR: Could not open the input image.\n ");
        return -1;
    }

    // Allocate the memory for output images:
    out_img.create(in_img.rows, in_img.cols, CV_8UC1);
    ocv_ref.create(in_img.rows, in_img.cols, CV_8UC1);

    // Get processing kernel with desired shape - used in dilate and erode:
    cv::Mat element = cv::getStructuringElement(0, cv::Size(FILTER_SIZE, FILTER_SIZE), cv::Point(-1, -1));

    // Define parameters
    unsigned char* high_thresh = (unsigned char*)malloc(FILTER_SIZE * (FILTER_SIZE) * sizeof(unsigned char));
    unsigned char* low_thresh = (unsigned char*)malloc(FILTER_SIZE * (FILTER_SIZE) * sizeof(unsigned char));
    unsigned char* shape = (unsigned char*)malloc(FILTER_SIZE * (FILTER_SIZE) * sizeof(unsigned char));

    // Define the low and high thresholds
    // Want to grab 3 colors (Yellow, Green, Red) for the input image
    low_thresh[0] = 22; // Lower boundary for Yellow
    low_thresh[1] = 150;
    low_thresh[2] = 60;

    high_thresh[0] = 38; // Upper boundary for Yellow
    high_thresh[1] = 255;
    high_thresh[2] = 255;

    low_thresh[3] = 38; // Lower boundary for Green
    low_thresh[4] = 150;
    low_thresh[5] = 60;

    high_thresh[3] = 75; // Upper boundary for Green
    high_thresh[4] = 255;
    high_thresh[5] = 255;

    low_thresh[6] = 160; // Lower boundary for Red
    low_thresh[7] = 150;
    low_thresh[8] = 60;

    high_thresh[6] = 179; // Upper boundary for Red
    high_thresh[7] = 255;
    high_thresh[8] = 255;

    for (int i = 0; i < (FILTER_SIZE * FILTER_SIZE); i++) {
        shape[i] = element.data[i];
    }

    int rows = in_img.rows;
    int cols = in_img.cols;

    std::cout << "INFO: Thresholds loaded." << std::endl;

    // ------------------------------------------------------------------ //

    // Execute reference function
    colordetect_ref(in_img, ocv_ref, low_thresh, high_thresh);

    // Execute DUT wrapper
    colordetect_dut_wrapper(in_img, out_img, rows, cols, low_thresh, high_thresh, shape);

    // Results verification
    if (colordetect_verif(ocv_ref, out_img) > 0.0f) {
        fprintf(stderr, "ERROR: Test Failed.\n ");
        return EXIT_FAILURE;
    };

    // ------------------------------------------------------------------ //

    // Create output header files
    write_mat_out(in_img, "in_img_small", INPUT_PTR_WIDTH);
    write_mat_out(ocv_ref, "golden_out_img_small", OUTPUT_PTR_WIDTH);

    return 0;
}

/* ===================================================================== */