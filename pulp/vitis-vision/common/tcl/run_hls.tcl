#
# Copyright 2019 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

source settings.tcl

set PROJ "${PRJ_NAME}"
set SOLN "${SOL_NAME}"

if {![info exists CLKP]} {
  set CLKP 3.3
}

open_project -reset $PROJ

# Notes:
# >> std changed from c++0x to c++14 to silence warnings
add_files "${CPP_TOP_PATH}" -cflags "-I${VITIS_LIB_INCLUDE} -I ${CUR_DIR}/build -I ./ -D__SDSVHLS__ -std=c++14" -csimflags "-I${VITIS_LIB_INCLUDE} -I ${CUR_DIR}/build -I ./ -D__SDSVHLS__ -std=c++14"
add_files -tb "${CPP_TB_PATH}" -cflags "-I${OPENCV_INCLUDE} -I${VITIS_LIB_INCLUDE} -I ${CUR_DIR}/build -I ./ -I ${GENHFILE_DIR} -D__SDSVHLS__ -std=c++14" -csimflags "-I${VITIS_LIB_INCLUDE} -I ${CUR_DIR}/build -I ./ -I ${GENHFILE_DIR} -D__SDSVHLS__ -std=c++14"
set_top ${ACC_NAME}

open_solution -reset $SOLN

set_part $XPART
create_clock -period $CLKP

# C simulation
if {$CSIM} {
  csim_design -ldflags "-L ${OPENCV_LIB} -lopencv_imgcodecs -lopencv_imgproc -lopencv_core -lopencv_highgui -lopencv_flann -lopencv_features2d" -argv " ${XF_PROJ_ROOT}/data/128x128.png "
}

# HLS synthesis
if {$CSYNTH} {
  csynth_design
}

# Post-synthesis co-simulation
if {($COSIM) && !($COSIM_GUI)} {
  cosim_design -ldflags "-L ${OPENCV_LIB} -lopencv_imgcodecs -lopencv_imgproc -lopencv_core -lopencv_highgui -lopencv_flann -lopencv_features2d" -argv " ${XF_PROJ_ROOT}/data/128x128.png "
}

# Post-synthesis co-simulation (with Vivado GUI)
if {($COSIM) && ($COSIM_GUI)} {
  cosim_design -tool xsim -trace_level all -wave_debug -ldflags "-L ${OPENCV_LIB} -lopencv_imgcodecs -lopencv_imgproc -lopencv_core -lopencv_highgui -lopencv_flann -lopencv_features2d" -argv " ${XF_PROJ_ROOT}/data/128x128.png "
}

# Vivado synthesis
if {$VIVADO_SYN} {
  export_design -flow syn -rtl verilog
}

# Vivado implementation
if {$VIVADO_IMPL} {
  export_design -flow impl -rtl verilog
}

exit