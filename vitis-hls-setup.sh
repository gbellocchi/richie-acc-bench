# Common paths
wks_tools=$HOME/workspace_tools

# Environment
source /var/pkg/ritchie/Xilinx/Vitis/2022.2/settings64.sh
# source < part-to-XRT-installation-directory >/setup.sh

export DEVICE=/var/pkg/ritchie/Xilinx/Vitis/2022.2/base_platforms/xilinx_zcu102_base_202220_1/xilinx_zcu102_base_202220_1.xpfm
export XPART=xczu9eg-ffvb1156-2-e

export CSIM=0
export CSYNTH=1
export COSIM=0

export OPENCV_INCLUDE=$wks_tools/opencv/opencv/build/include
export OPENCV_LIB=$wks_tools/opencv/opencv/build/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OPENCV_LIB

export XF_PROJ_ROOT=$wks_tools/vitis/PYNQ_Composable_Pipeline/Vitis_Libraries/vision
export VITIS_LIB_INCLUDE=$wks_tools/vitis/PYNQ_Composable_Pipeline/Vitis_Libraries/vision/L1/include

export TOOL_VERSION=2022.2 
export PLATFORM_REPO_PATHS=/var/pkg/ritchie/Xilinx/Vitis/2022.2/base_platforms/xilinx_zcu102_base_202220_1

# Direct run of Vitis Vision Pipelines (ZCU102)
# make run CSIM=$CSIM CSYNTH=$CSYNTH COSIM=$COSIM XPART=$XPART TOOL_VERSION=$TOOL_VERSION PLATFORM_REPO_PATHS=$PLATFORM_REPO_PATHS