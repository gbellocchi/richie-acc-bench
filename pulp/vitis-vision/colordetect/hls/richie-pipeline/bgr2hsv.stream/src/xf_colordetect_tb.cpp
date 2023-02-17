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

// OpenCV reference function:
void colordetect_ref(cv::Mat& _src, cv::Mat& _dst, unsigned char* nLowThresh, unsigned char* nHighThresh) {
    // Temporary matrices for processing
    cv::Mat mask1, mask2, mask3, _imgrange, _imghsv;

    cv::imwrite("a_img_in.png", _src);

    // Convert the input to the HSV colorspace. Using BGR here since it is the default of OpenCV.
    // Using RGB yields different results, requiring a change of the threshold ranges
    cv::cvtColor(_src, _imghsv, cv::COLOR_BGR2HSV);
    cv::imwrite("b_img_bgr2hsv.png", _imghsv);

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
    cv::imwrite("c_img_threshold.png", _imgrange);

    cv::Mat element = cv::getStructuringElement(0, cv::Size(3, 3), cv::Point(-1, -1));
    cv::erode(_imgrange, _dst, element, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT);
    cv::imwrite("d_img_erode.png", _dst);

    cv::dilate(_dst, _dst, element, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT);
    cv::imwrite("e_img_dilate.png", _dst);

    cv::dilate(_dst, _dst, element, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT);
    cv::imwrite("f_img_dilate.png", _dst);

    cv::erode(_dst, _dst, element, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT);
    cv::imwrite("g_img_erode.png", _dst);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <INPUT IMAGE PATH 1> <INPUT IMAGE PATH 2>\n", argv[0]);
        return EXIT_FAILURE;
    }

    cv::Mat in_img, img_rgba, out_img, ocv_ref, out_pipeline_ref, diff;

    // Open input image:
    in_img = cv::imread(argv[1], 1);
    if (!in_img.data) {
        fprintf(stderr, "ERROR: Could not open the input image.\n ");
        return -1;
    }

    // Allocate the memory for output images:
    out_img.create(in_img.rows, in_img.cols, CV_8UC1);
    ocv_ref.create(in_img.rows, in_img.cols, CV_8UC1);

    // Convert input from BGR to RGBA:
    //   cv::cvtColor(in_img, img_rgba, CV_BGR2RGBA);

    // Get processing kernel with desired shape - used in dilate and erode:

    cv::Mat element = cv::getStructuringElement(0, cv::Size(FILTER_SIZE, FILTER_SIZE), cv::Point(-1, -1));

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
        // printf("\n\t SHAPE[i] = %d\n", shape[i]);
    }

    int rows = in_img.rows;
    int cols = in_img.cols;

    std::cout << "INFO: Thresholds loaded." << std::endl;

    // Reference function:
    colordetect_ref(in_img, ocv_ref, low_thresh, high_thresh);

    // IO streams
    stream_in_t stream_in("stream_in");
    stream_in_t stream_out("stream_out");

    // Convert Mat to Stream
    cvMat2AXIvideoxf<NPC1>(in_img, stream_in);

    // DUT
    color_detect(
        (stream_in_t &)stream_in, 
        (stream_in_t &)stream_out,
        rows, 
        cols
    );

    // Convert Stream to Mat
    AXIvideo2cvMatxf<NPC1, INPUT_PTR_WIDTH>(stream_out, out_img);

    // Open partial pipeline result:
    out_pipeline_ref = cv::imread("b_img_bgr2hsv.png", 1);
    if (!out_pipeline_ref.data) {
        fprintf(stderr, "ERROR: Could not open the input image.\n ");
        return -1;
    }

    // Results verification:
    int cnt = 0;
    cv::absdiff(out_pipeline_ref, in_img, diff);

    for (int i = 0; i < diff.rows; ++i) {
        for (int j = 0; j < diff.cols; ++j) {
            uchar v = diff.at<uchar>(i, j);
            if (v > 0) cnt++;
        }
    }

    float err_per = 100.0 * (float)cnt / (diff.rows * diff.cols);

    std::cout << "INFO: Verification results:" << std::endl;
    std::cout << "\tPercentage of pixels above error threshold = " << err_per << "%" << std::endl;

    if (err_per > 0.0f) {
        fprintf(stderr, "ERROR: Test Failed.\n ");
        return EXIT_FAILURE;
    }

    // ------------------------------------------------------------------ //

    // Write down images
    cv::imwrite("golden_out_img.png", ocv_ref);
    cv::imwrite("output.png", out_img);
    cv::imwrite("diff.png", diff);

    // Convert input Mat img to array
    char* in_img_array = (char*)malloc(HEIGHT*WIDTH*sizeof(char));
	for (int y = 0; y < HEIGHT; y++){
		for (int x = 0; x < WIDTH; x++){
			in_img_array[y * HEIGHT + x] = in_img.at<char>(y, x);
		}
	}

    // Convert golden Mat img to array
    char* golden_img_array = (char*)malloc(HEIGHT*WIDTH*sizeof(char));
	for (int y = 0; y < HEIGHT; y++){
		for (int x = 0; x < WIDTH; x++){
			golden_img_array[y * HEIGHT + x] = ocv_ref.at<char>(y, x);
		}
	}

    // Create output header files
    gen_Hfile("in_img_small", in_img_array, HEIGHT, WIDTH, INPUT_PTR_WIDTH);
    gen_Hfile("golden_out_img_small", golden_img_array, HEIGHT, WIDTH, OUTPUT_PTR_WIDTH);

    return 0;
}
