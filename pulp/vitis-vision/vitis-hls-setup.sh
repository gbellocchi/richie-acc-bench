# =====================================================================
# Project:      Richie accelerator benchmarks
# Title:        Vitis Vision setup script
#
# $Date:        23.02.2023
# =====================================================================
#
# Copyright (C) 2023 University of Modena and Reggio Emilia.
#
# Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
#
# =====================================================================

# Paths
workspace_vitis_vision=$(dirname "$(realpath ${BASH_SOURCE:-$0})")
vitis_path=$workspace_vitis_vision/Vitis_Libraries
install_path=$workspace_vitis_vision/install
tools_dir=$workspace_vitis_vision/tools

if [ $UNIMORE -eq 1 ]; then

    echo "- Vitis UNIMORE setup -"

    # Xilinx tools
    export TOOL_VERSION=2022.2 # Important for Vitis HLS in this case
    export VIVADO="vivado"
    export VIVADO_HLS="vivado_hls"
    export VITIS_HLS="vitis_hls"
    export PETALINUX_VER="" 

    # Xilinx setup
    source /var/pkg/ritchie/Xilinx/Vitis/2022.2/settings64.sh
    export DEVICE=/var/pkg/ritchie/Xilinx/Vitis/2022.2/base_platforms/xilinx_zcu102_base_202220_1/xilinx_zcu102_base_202220_1.xpfm
    export PLATFORM_REPO_PATHS=/var/pkg/ritchie/Xilinx/Vitis/2022.2/base_platforms/xilinx_zcu102_base_202220_1

elif [ $IIS -eq 1 ]; then

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

export XPART=xczu9eg-ffvb1156-2-e

# OpenCV
export OPENCV_INCLUDE=$install_path/opencv/build/include
export OPENCV_LIB=$install_path/opencv/build/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OPENCV_LIB

# Vitis Libraries
export XF_PROJ_ROOT=$vitis_path/vision
export VITIS_LIB_INCLUDE=$vitis_path/vision/L1/include