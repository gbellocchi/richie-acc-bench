#!/usr/bin/env bash

my_home_dir=$(pwd)

# Create install directory
mkdir -p $my_home_dir/install
cd $my_home_dir/install

# Remove old versions
sudo apt purge -y libopencv* python-opencv
sudo apt autoremove -y

sudo apt update -y
sudo apt upgrade -y

# Build pre-requisites
sudo apt install -y build-essential cmake pkg-config git
sudo apt install -y libjpeg-dev libtiff5-dev libpng-dev
sudo apt install -y libavcodec-dev libavformat-dev libswscale-dev libxvidcore-dev libx264-dev libxine2-dev
sudo apt install -y libv4l-dev v4l-utils
sudo apt install -y libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev 
sudo apt install -y libgtk2.0-dev
sudo apt install -y mesa-utils libgl1-mesa-dri libgtkgl2.0-dev libgtkglext1-dev
sudo apt install -y libatlas-base-dev gfortran libeigen3-dev
sudo apt install -y python2.7-dev python3-dev python-numpy python3-numpy

# Download OpenCV
wget -O opencv.zip https://github.com/opencv/opencv/archive/3.3.1.zip
unzip opencv.zip && mv opencv-3.3.1 opencv
wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/3.3.1.zip
unzip opencv_contrib.zip && mv opencv_contrib-3.3.1 opencv_contrib

# Create OpenCV build directory
mkdir -p $my_home_dir/install/opencv/build
cd $my_home_dir/install/opencv/build

# Build OpenCV
cmake -D CMAKE_BUILD_TYPE=RELEASE \
-D CMAKE_INSTALL_PREFIX=/usr/local \
-D WITH_TBB=OFF \
-D WITH_IPP=OFF \
-D WITH_1394=OFF \
-D BUILD_WITH_DEBUG_INFO=OFF \
-D BUILD_DOCS=OFF \
-D INSTALL_C_EXAMPLES=ON \
-D INSTALL_PYTHON_EXAMPLES=ON \
-D BUILD_EXAMPLES=OFF \
-D BUILD_TESTS=OFF \
-D BUILD_PERF_TESTS=OFF \
-D WITH_QT=OFF \
-D WITH_GTK=ON \
-D WITH_OPENGL=ON \
-D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules \
-D WITH_V4L=ON  \
-D WITH_FFMPEG=ON \
-D WITH_XINE=ON \
-D BUILD_NEW_PYTHON_SUPPORT=ON \
-D OPENCV_GENERATE_PKGCONFIG=ON \
-D WITH_CUDA=OFF \
../

# Install OpenCV
make -j4
sudo make install
sudo sh -c 'echo '/usr/local/lib' > /etc/ld.so.conf.d/opencv.conf'
sudo ldconfig

# Back home
cd $my_home_dir