# Design name.
set design_name [lindex $argv 0]
puts "System-level integration of $design_name accelerator IP using standard Xilinx methodology."

# Vivado project location.
set vivado_dir [lindex $argv 1]
set prj_dir "$vivado_dir/$design_name"
puts "Vivado project is going to be located in $prj_dir\."

# Accelerator IP repository.
set ip_repo [lindex $argv 2]
puts "IP repository is located in $ip_repo\."

# Hardware design file location.
set hw_design_dir [lindex $argv 3]
puts "Hardware design files are going to be located in $hw_design_dir\."

# Create project.
create_project $design_name $prj_dir\/ -part xczu9eg-ffvb1156-2-e
set_property board_part xilinx.com:zcu102:part0:3.3 [current_project]

# Create block hw_design.
create_bd_design "$design_name\_bd"
update_compile_order -fileset sources_1

# Update IP repository.
set_property  ip_repo_paths $ip_repo [current_project]
update_ip_catalog

# Zynq UltraScale+ Processor System.
create_bd_cell -type ip -vlnv xilinx.com:ip:zynq_ultra_ps_e:3.3 zynq_ultra_ps_e_0
set_property -dict [list \
    CONFIG.PSU__USE__M_AXI_GP2 {0} \
    CONFIG.PSU__USE__S_AXI_GP2 {1} \
    CONFIG.PSU__SAXIGP2__DATA_WIDTH {32} \
    CONFIG.PSU__USE__S_AXI_GP3 {1} \
    CONFIG.PSU__SAXIGP3__DATA_WIDTH {32} \
    CONFIG.PSU__USE__S_AXI_GP4 {1} \
    CONFIG.PSU__SAXIGP4__DATA_WIDTH {32} \
    CONFIG.PSU__USE__IRQ0 {1} \
] [get_bd_cells zynq_ultra_ps_e_0]

# HLS-generated accelerator.
create_bd_cell -type ip -vlnv xilinx.com:hls:mmult_hw:1.0 mmult_hw_0

# Connect system IPs.
connect_bd_net [get_bd_pins zynq_ultra_ps_e_0/pl_clk0] [get_bd_pins mmult_hw_0/ap_clk]
connect_bd_net [get_bd_pins zynq_ultra_ps_e_0/pl_resetn0] [get_bd_pins mmult_hw_0/ap_rst_n]
connect_bd_net [get_bd_pins zynq_ultra_ps_e_0/pl_ps_irq0] [get_bd_pins mmult_hw_0/interrupt]
connect_bd_intf_net [get_bd_intf_pins mmult_hw_0/m_axi_port_in1] [get_bd_intf_pins zynq_ultra_ps_e_0/S_AXI_HP0_FPD]
connect_bd_intf_net [get_bd_intf_pins mmult_hw_0/m_axi_port_in2] [get_bd_intf_pins zynq_ultra_ps_e_0/S_AXI_HP1_FPD]
connect_bd_intf_net [get_bd_intf_pins zynq_ultra_ps_e_0/S_AXI_HP2_FPD] [get_bd_intf_pins mmult_hw_0/m_axi_port_out]
connect_bd_net [get_bd_pins zynq_ultra_ps_e_0/saxihp0_fpd_aclk] [get_bd_pins zynq_ultra_ps_e_0/saxihp1_fpd_aclk]
connect_bd_net [get_bd_pins zynq_ultra_ps_e_0/saxihp1_fpd_aclk] [get_bd_pins zynq_ultra_ps_e_0/saxihp2_fpd_aclk]
connect_bd_net [get_bd_pins zynq_ultra_ps_e_0/saxihp2_fpd_aclk] [get_bd_pins zynq_ultra_ps_e_0/pl_clk0]
apply_bd_automation -rule xilinx.com:bd_rule:zynq_ultra_ps_e -config {apply_board_preset "1" }  [get_bd_cells zynq_ultra_ps_e_0]

apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config { Clk_master {Auto} Clk_slave {/zynq_ultra_ps_e_0/pl_clk0 (99 MHz)} Clk_xbar {Auto} Master {/zynq_ultra_ps_e_0/M_AXI_HPM0_FPD} Slave {/mmult_hw_0/s_axi_control} ddr_seg {Auto} intc_ip {New AXI Interconnect} master_apm {0}}  [get_bd_intf_pins mmult_hw_0/s_axi_control]
apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config { Clk_master {Auto} Clk_slave {/zynq_ultra_ps_e_0/pl_clk0 (99 MHz)} Clk_xbar {/zynq_ultra_ps_e_0/pl_clk0 (99 MHz)} Master {/zynq_ultra_ps_e_0/M_AXI_HPM1_FPD} Slave {/mmult_hw_0/s_axi_control} ddr_seg {Auto} intc_ip {/ps8_0_axi_periph} master_apm {0}}  [get_bd_intf_pins zynq_ultra_ps_e_0/M_AXI_HPM1_FPD]

# Set FPGA hw_design frequency.
set_property -dict [list \
    CONFIG.PSU__CRL_APB__PL0_REF_CTRL__FREQMHZ {150} \
] [get_bd_cells zynq_ultra_ps_e_0]

# Address map.
## in1.
exclude_bd_addr_seg [get_bd_addr_segs mmult_hw_0/Data_m_axi_port_in1/SEG_zynq_ultra_ps_e_0_HP0_PCIE_LOW]
exclude_bd_addr_seg [get_bd_addr_segs mmult_hw_0/Data_m_axi_port_in1/SEG_zynq_ultra_ps_e_0_HP0_QSPI]
## in2.
exclude_bd_addr_seg [get_bd_addr_segs mmult_hw_0/Data_m_axi_port_in2/SEG_zynq_ultra_ps_e_0_HP1_PCIE_LOW]
exclude_bd_addr_seg [get_bd_addr_segs mmult_hw_0/Data_m_axi_port_in2/SEG_zynq_ultra_ps_e_0_HP1_QSPI]
## out.
exclude_bd_addr_seg [get_bd_addr_segs mmult_hw_0/Data_m_axi_port_out/SEG_zynq_ultra_ps_e_0_HP2_PCIE_LOW]
exclude_bd_addr_seg [get_bd_addr_segs mmult_hw_0/Data_m_axi_port_out/SEG_zynq_ultra_ps_e_0_HP2_QSPI]

# Validate and save top-bevel block design.
save_bd_design
validate_bd_design

# Create hardware wrapper.
close_bd_design [get_bd_designs $design_name\_bd]
make_wrapper -files [get_files $prj_dir/$design_name.srcs/sources_1/bd/$design_name\_bd/$design_name\_bd.bd] -top
add_files -norecurse $prj_dir/$design_name.srcs/sources_1/bd/$design_name\_bd/hdl/$design_name\_bd_wrapper.v

# Setup synthesis and implementation
set_property name synth_150MHz [get_runs synth_1]
set_property name impl_150MHz [get_runs impl_1]

# Synthesize
launch_runs synth_150MHz -host {larain8 32} -remote_cmd {ssh -q -o BatchMode=yes}
wait_on_run synth_150MHz

# Implement
launch_runs impl_150MHz -jobs 12
wait_on_run impl_150MHz

# Check timing constraints.
open_run impl_150MHz
set timingrep [report_timing_summary -no_header -no_detailed_paths -return_string]
if {! [string match -nocase {*timing constraints are met*} $timingrep]} {
  send_msg_id {USER 1-1} ERROR {Timing constraints were not met.}
  return -code error
}

# Generate Bitstream
launch_runs impl_150MHz -to_step write_bitstream -jobs 12
wait_on_run impl_150MHz

# Export Hardware Definition file.
write_hwdef -force -file $hw_design_dir/$design_name.hdf

# Export bitstream file.
file copy -force $prj_dir/$design_name.runs/impl_150MHz/$design_name\_bd_wrapper.bit $hw_design_dir/$design_name.bit

# Export Xilinx Support Archive (XSA) file.
write_hw_platform -fixed -force -include_bit -file $hw_design_dir/$design_name.xsa
