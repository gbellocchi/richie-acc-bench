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
#include "xf_fast_config.h"

int main(int argc, char** argv) {
    // IO data
    cv::Mat in_img, in_gray, out_img, out_img_ocv, out_hls;

    // Parameters
    uchar_t threshold = 20; // threshold
    unsigned short imgwidth = in_img.cols;
    unsigned short imgheight = in_img.rows;

    std::vector<cv::KeyPoint> keypoints;
    
    // Read golden data
    in_img = cv::imread(argv[1], -1); // reading in the color image (128x128.png)

    if (!in_img.data) {
        fprintf(stderr, "Failed to load the image ... %s\n ", argv[1]);
        return -1;
    }

    // Additional image processing
    cvtColor(in_img, in_gray, CV_BGR2GRAY);

    /* Create the hardware streams */
    // out_hls.create(in_gray.rows, in_gray.cols, CV_8U);

    // Golden model (OpenCV reference function)
    cv::FAST(in_gray, keypoints, threshold, NMS);

    // // IO streams
    hls::stream<ap_axiu<PTR_WIDTH,1,1,1>> stream_in;
    hls::stream<ap_axiu<PTR_WIDTH,1,1,1>> stream_out;

	// Convert Mat to Stream
	for (int y = 0; y < HEIGHT; y++){
		for (int x = 0; x < WIDTH; x++){
			ap_axiu<PTR_WIDTH,1,1,1> value = in_gray.at<ap_axiu<PTR_WIDTH,1,1,1>>(y, x); // depth_t(double(src.at<uchar>(y, x) + 0.5) / double(255.0));
			stream_in.write(value);
		}
	}

    // DUT call
    fast_corner_detect(stream_in, stream_out, threshold, imgheight, imgwidth);

    // // Convert Stream to Mat
	// for (int y = 0; y < HEIGHT; y++){
	// 	for (int x = 0; x < WIDTH; x++){
    //         ap_axiu<PTR_WIDTH,1,1,1> value = stream_out.read();
    //         uchar value2 = value.data;
    //         out_hls.at<uchar>(y, x) = value2;
	// 		// ap_axiu<PTR_WIDTH,1,1,1> value = stream_out.read();
    //         // out_hls.at<uchar>(y, x) = uchar(value);
	// 	}
	// }

    // Post-processing - Golden + HLS outputs
    std::vector<cv::Point> hls_points;
    std::vector<cv::Point> ocv_points;
    std::vector<cv::Point> common_points;
    std::vector<cv::Point> noncommon_points;

    FILE *fp, *fp1;
    fp = fopen("ocvpoints.txt", "w");
    fp1 = fopen("hlspoints.txt", "w");

    int nsize = keypoints.size();

    printf("ocvpoints:%d=\n", nsize);

    for (int i = 0; i < nsize; i++) {
        int x = keypoints[i].pt.x;
        int y = keypoints[i].pt.y;
        ocv_points.push_back(cv::Point(x, y));
        // fprintf(fp, "x = %d, y = %d\n", x, y);
    }
    // fclose(fp);

    out_img_ocv = in_img.clone();

    int ocv_x = 0, ocv_y = 0;

    for (int cnt1 = 0; cnt1 < keypoints.size(); cnt1++) {
        ocv_x = keypoints[cnt1].pt.x;
        ocv_y = keypoints[cnt1].pt.y;
        cv::circle(out_img_ocv, cv::Point(ocv_x, ocv_y), 5, cv::Scalar(0, 0, 255), 2, 8, 0);
    }
    cv::imwrite("output_ocv.png", out_img_ocv);

    out_img = in_gray.clone();

    for (int j = 0; j < HEIGHT; j++) {
        for (int i = 0; i < WIDTH; i++) {

            // Read from output stream
            ap_axiu<PTR_WIDTH,1,1,1> stream_tmp = stream_out.read();

            // Extract single data from stream
            ap_int<PTR_WIDTH> value = stream_tmp.data;
    //         unsigned char value = out_hls.at<unsigned char>(j, i);

            if (value != 0) {
                short int y, x;
                y = j;
                x = i;

                cv::Point tmp;
                tmp.x = i;
                tmp.y = j;

                hls_points.push_back(tmp);
                if (j > 0) cv::circle(out_img, cv::Point(x, y), 5, cv::Scalar(0, 0, 255, 255), 2, 8, 0);
            }
        }
    }

    int nsize1 = hls_points.size();

    int Nocv = ocv_points.size();
    int Nhls = hls_points.size();

    for (int r = 0; r < nsize1; r++) {
        int a, b;
        a = (int)hls_points[r].x;
        b = (int)hls_points[r].y;
        fprintf(fp1, "x = %d, y = %d\n", a, b);
    }
    fclose(fp1);

    for (int j = 0; j < Nocv; j++) {
        for (int k = 0; k < Nhls; k++) {
            if ((ocv_points[j].x == ((hls_points[k].x))) && (ocv_points[j].y == ((hls_points[k].y)))) {
                common_points.push_back(ocv_points[j]);
            }
        }
    }

    FILE* fpt3;
    fpt3 = fopen("common.txt", "w");

    for (unsigned int p = 0; p < common_points.size(); p++) {
        fprintf(fpt3, "x = %d, y = %d\n", common_points[p].x, common_points[p].y);
    }

    fclose(fpt3);

    cv::imwrite("output_hls.png", out_img);

    // Results verification
    float persuccess, perloss, pergain;

    int totalocv = ocv_points.size();
    int totalhls = hls_points.size();
    int ncommon = common_points.size();

    persuccess = (((float)ncommon / totalhls) * 100);
    perloss = (((float)(totalocv - ncommon) / totalocv) * 100);
    pergain = (((float)(totalhls - ncommon) / totalhls) * 100);

    std::cout << "INFO: Verification results:" << std::endl;
    std::cout << "\tCommon = " << ncommon << std::endl;
    std::cout << "\tSuccess = " << persuccess << std::endl;
    std::cout << "\tLoss = " << perloss << std::endl;
    std::cout << "\tGain = " << pergain << std::endl;

    if (persuccess < 80) {
        fprintf(stderr, "ERROR: Test Failed.\n ");
        return EXIT_FAILURE;
    }

    return 0;
}
