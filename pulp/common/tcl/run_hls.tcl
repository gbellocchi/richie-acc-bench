#
# Copyright 2020 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Read HLS root
set hls_root [lindex $argv 2]

# Read project name
set project_name [lindex $argv 3]

# Read accelerator IP name
set accel_name [lindex $argv 4]

# Read mode
set mode [lindex $argv 5]

# Create a project
open_project -reset $project_name\_proj

# Set source directory
set src_loc $hls_root\/src

# Add design files
add_files $src_loc/$project_name\.cpp

# Add test bench 
add_files -tb $src_loc/$project_name\_tb.cpp

# Set the top-level function
set_top $accel_name

# Create a solution
open_solution -reset solution1

# Define technology (Xilinx ZCU102) 
set_part  {xczu9eg-ffvb1156-2-e}

# Define clock rate (ns) - 6.66ns <-> 150MHz
create_clock -period 6.66

# Determine which steps to execute
csim_design

if {$mode == 1} {
	puts "Run Synthesis and Exit"
	csynth_design
	
} elseif {$mode == 2} {
	puts "Run Synthesis, RTL Simulation and Exit"
	csynth_design
	cosim_design

} elseif {$mode == 3} { 
	puts "Run Synthesis, RTL Simulation, RTL implementation and Exit"
	csynth_design
	cosim_design
	export_design -rtl verilog -flow impl
} else {
	puts "Default: Exit after setup"
	csynth_design
	export_design -rtl verilog -format ip_catalog
}

exit
