#!/bin/bash

THIS_DIR=$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")

# Vitis setup
WKS_VITIS=$THIS_DIR
WKS_INSTALL=$WKS_VITIS/install
tools_dir=$WKS_VITIS/tools

if [ $UNIMORE = 1 ]; then

    echo "- Vitis UNIMORE setup -"

    # Xilinx tools
    export TOOL_VERSION=2022.2 # Important for Vitis HLS in this case
    export VIVADO="vivado"
    export VIVADO_HLS="vivado_hls"
    export VITIS_HLS="vitis_hls"
    export PETALINUX_VER=""

    # Xilinx setup
    # --->  Source Vitis settings: settings64.sh
    export DEVICE= # Add path to: /Vitis/2022.2/base_platforms/xilinx_zcu102_base_202220_1/xilinx_zcu102_base_202220_1.xpfm
    export PLATFORM_REPO_PATHS= # Add path to: /Vitis/2022.2/base_platforms/xilinx_zcu102_base_202220_1

elif [ $IIS = 1 ]; then

    echo "- Vitis ETHZ setup -"

    # Xilinx tools
    export TOOL_VERSION=2022.2 # Important for Vitis HLS in this case
    export VIVADO="vitis-2019.2 vivado"
    export VIVADO_HLS="vitis-2019.2 vivado_hls"
    export VITIS_HLS="vitis-2022.2 vitis_hls"
    export PETALINUX_VER="vitis-2019.2"

    # Xilinx setup
    # ...TO-DO...

fi

# FPGA part
export XPART=xczu9eg-ffvb1156-2-e # ZU9EG

# OpenCV
export OPENCV_INCLUDE=$WKS_INSTALL/opencv/build/include
export OPENCV_LIB=$WKS_INSTALL/opencv/build/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OPENCV_LIB

# # Vitis Libraries
# export XF_PROJ_ROOT=$WKS_VITIS/Vitis_Libraries/vision
# export VITIS_LIB_INCLUDE=$WKS_VITIS/Vitis_Libraries/vision/L1/include

# # OpenCV
# export OPENCV_INCLUDE= # Add path to OpenCV include directory: /opencv/opencv/build/include
# export OPENCV_LIB= # Add path to OpenCV library directory: /opencv/opencv/build/lib
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OPENCV_LIB

# Vitis Libraries
export XF_PROJ_ROOT= # Add path to Vitis libraries root: /PYNQ_Composable_Pipeline/Vitis_Libraries/vision
export VITIS_LIB_INCLUDE= # Add path to Vitis libraries include directory: /PYNQ_Composable_Pipeline/Vitis_Libraries/vision/L1/include

# Environment check
source $THIS_DIR/check_environment.sh
must_be_nonempty WKS_VITIS $WKS_VITIS
