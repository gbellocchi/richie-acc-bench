#!/usr/bin/env bash

check_env_var()
{
    variable_name=$1
    variable_value=$2

    if [[ -z "$variable_value" ]]; then
      echo "[ERROR] <$variable_name> is not defined yet. Please correct this!"
    else
      echo "<$variable_name> defined at '$variable_value'"
    fi
}

# Check on the definition of a Vitis Vision workspace
check_env_var workspace_vitis_vision $workspace_vitis_vision

# Check on the definition of an install path
check_env_var install_path $install_path

# Create install directory
mkdir -p $install_path
cd $install_path

if [ $UNIMORE -eq 1 ]; then
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
fi

# Download OpenCV
wget -O opencv.zip https://github.com/opencv/opencv/archive/3.3.1.zip
unzip opencv.zip && mv opencv-3.3.1 opencv
wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/3.3.1.zip
unzip opencv_contrib.zip && mv opencv_contrib-3.3.1 opencv_contrib

# Create OpenCV build directory
mkdir -p $install_path/opencv/build
cd $install_path/opencv/build

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
cd $workspace_vitis_vision