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

# Create a project
open_project -reset mmult_proj

# Source files position
set src_loc "src"

# Add design files
add_files $src_loc/mmult.cpp

# Add test bench 
add_files -tb $src_loc/mmult_tb.cpp

# Set the top-level function
set_top mmult_hw

# Create a solution
open_solution -reset solution1

# Define technology (Xilinx ZCU102) 
set_part  {xczu9eg-ffvb1156-2-e}

# Define clock rate (ns) - 6.66ns <-> 150MHz
create_clock -period 6.66

# Read Makefile arg and determine which steps to execute
csim_design

set mode [lindex $argv 3]
puts "$mode"

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
