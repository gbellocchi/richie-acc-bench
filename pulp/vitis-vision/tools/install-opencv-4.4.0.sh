#!/bin/bash

if [[ -z "$WKS_VITIS" ]]; then
  echo "Source the Vitis setup first."
  exit 1
fi

# Create install directory
mkdir -p $WKS_INSTALL
cd $WKS_INSTALL

# if [ $UNIMORE -eq 1 ]; then
    # # Remove old versions
    # sudo apt purge -y libopencv* python-opencv
    # sudo apt autoremove -y

    # sudo apt update -y
    # sudo apt upgrade -y

    # # Build pre-requisites
    # sudo apt install -y build-essential cmake pkg-config git
    # sudo apt install -y libjpeg-dev libtiff5-dev libpng-dev
    # sudo apt install -y libavcodec-dev libavformat-dev libswscale-dev libxvidcore-dev libx264-dev libxine2-dev
    # sudo apt install -y libv4l-dev v4l-utils
    # sudo apt install -y libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
    # sudo apt install -y libgtk2.0-dev
    # sudo apt install -y mesa-utils libgl1-mesa-dri libgtkgl2.0-dev libgtkglext1-dev
    # sudo apt install -y libatlas-base-dev gfortran libeigen3-dev
    # sudo apt install -y python2.7-dev python3-dev python-numpy python3-numpy
# fi

# # Download OpenCV
# wget -O opencv.zip https://github.com/opencv/opencv/archive/refs/tags/4.4.0.zip
# unzip opencv.zip && mv opencv-4.4.0 opencv
# wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/refs/tags/4.4.0.zip
# unzip opencv_contrib.zip && mv opencv_contrib-4.4.0 opencv_contrib

# Create OpenCV build directory
mkdir -p $WKS_INSTALL/opencv/build
cd $WKS_INSTALL/opencv/build

export LIBRARY_PATH=/usr/lib/x86_64-linux-gnu/:$LIBRARY_PATH

# Build OpenCV
if [ $UNIMORE -eq 1 ]; then
  cmake -D CMAKE_BUILD_TYPE=RELEASE \
  -D CMAKE_INSTALL_PREFIX=/usr/local \
  -D WITH_V4L=ON \
  -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules \
  -D BUILD_TESTS=OFF \
  -D BUILD_ZLIB=ON \
  -D BUILD_JPEG=ON \
  -D WITH_JPEG=ON \
  -D WITH_PNG=ON \
  -D BUILD_EXAMPLES=OFF \
  -D INSTALL_C_EXAMPLES=OFF \
  -D INSTALL_PYTHON_EXAMPLES=OFF \
  -D WITH_OPENEXR=OFF \
  -D BUILD_OPENEXR=OFF \
  -D CMAKE_CXX_COMPILER=/var/pkg/ritchie/Xilinx/Vitis_HLS/2022.2/tps/lnx64/gcc-6.2.0/bin/g++ \
  ../
fi

# Install OpenCV
make -j32
make install

# Back home
cd $workspace_vitis_vision

# export OPENCV_INCLUDE=<output path to installed opencv>/include/opencv4
# export OPENCV_LIB=<output path to installed opencv>/lib
# export LD_LIBRARY_PATH=<output path to installed opencv>/lib:$LD_LIBRARY_PATH
