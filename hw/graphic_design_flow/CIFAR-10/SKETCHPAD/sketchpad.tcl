
################################################################
# This is a generated script based on design: sketchpad
#
# Though there are limitations about the generated script,
# the main purpose of this utility is to make learning
# IP Integrator Tcl commands easier.
################################################################

create_project sketchpad sketchpad -part xc7z020clg400-1

set_property ip_repo_paths  .. [current_project]
update_ip_catalog

namespace eval _tcl {
proc get_script_folder {} {
   set script_path [file normalize [info script]]
   set script_folder [file dirname $script_path]
   return $script_folder
}
}
variable script_folder
set script_folder [_tcl::get_script_folder]

################################################################
# START
################################################################

# To test this script, run the following commands from Vivado Tcl console:
# source base_script.tcl

# If there is no project opened, this script will create a
# project, but make sure you do not have an existing project
# <./myproj/project_1.xpr> in the current working folder.


# CHANGE DESIGN NAME HERE
set design_name sketchpad

# This script was generated for a remote BD. To create a non-remote design,
# change the variable <run_remote_bd_flow> to <0>.

set run_remote_bd_flow 1
if { $run_remote_bd_flow == 1 } {
  set str_bd_folder base_project
  set str_bd_filepath ${str_bd_folder}/${design_name}/${design_name}.bd

  # Check if remote design exists on disk
  if { [file exists $str_bd_filepath ] == 1 } {
     catch {common::send_msg_id "BD_TCL-110" "ERROR" "The remote BD file path <$str_bd_filepath> already exists!"}
     common::send_msg_id "BD_TCL-008" "INFO" "To create a non-remote BD, change the variable <run_remote_bd_flow> to <0>."
     common::send_msg_id "BD_TCL-009" "INFO" "Also make sure there is no design <$design_name> existing in your current project."

     return 1
  }

  # Check if design exists in memory
  set list_existing_designs [get_bd_designs -quiet $design_name]
  if { $list_existing_designs ne "" } {
     catch {common::send_msg_id "BD_TCL-111" "ERROR" "The design <$design_name> already exists in this project! Will not create the remote BD <$design_name> at the folder <$str_bd_folder>."}

     common::send_msg_id "BD_TCL-010" "INFO" "To create a non-remote BD, change the variable <run_remote_bd_flow> to <0> or please set a different value to variable <design_name>."

     return 1
  }

  # Check if design exists on disk within project
  set list_existing_designs [get_files */${design_name}.bd]
  if { $list_existing_designs ne "" } {
     catch {common::send_msg_id "BD_TCL-112" "ERROR" "The design <$design_name> already exists in this project at location:
    $list_existing_designs"}
     catch {common::send_msg_id "BD_TCL-113" "ERROR" "Will not create the remote BD <$design_name> at the folder <$str_bd_folder>."}

     common::send_msg_id "BD_TCL-011" "INFO" "To create a non-remote BD, change the variable <run_remote_bd_flow> to <0> or please set a different value to variable <design_name>."

     return 1
  }

  # Now can create the remote BD
  create_bd_design -dir $str_bd_folder $design_name
} else {

  # Create regular design
  if { [catch {create_bd_design $design_name} errmsg] } {
     common::send_msg_id "BD_TCL-012" "INFO" "Please set a different value to variable <design_name>."

     return 1
  }
}

current_bd_design $design_name

##################################################################
# DESIGN PROCs
##################################################################



# Procedure to create entire design; Provide argument to make
# procedure reusable. If parentCell is "", will use root.
proc create_root_design { parentCell } {

  variable script_folder

  if { $parentCell eq "" } {
     set parentCell [get_bd_cells /]
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_msg_id "BD_TCL-100" "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_msg_id "BD_TCL-101" "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj


  # Create interface ports
  set axis_in [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 axis_in ]
  set_property -dict [ list \
CONFIG.HAS_TKEEP {0} \
CONFIG.HAS_TLAST {0} \
CONFIG.HAS_TREADY {1} \
CONFIG.HAS_TSTRB {0} \
CONFIG.LAYERED_METADATA {undef} \
CONFIG.TDATA_NUM_BYTES {1} \
CONFIG.TDEST_WIDTH {0} \
CONFIG.TID_WIDTH {0} \
CONFIG.TUSER_WIDTH {0} \
 ] $axis_in
  set axis_out [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 axis_out ]

  # Create ports
  set clk_in [ create_bd_port -dir I -type clk clk_in ]
  set_property -dict [ list \
CONFIG.FREQ_HZ {100000000} \
 ] $clk_in
  set rsn_in [ create_bd_port -dir I -type rst rsn_in ]

  # Create instance: FC_CIF_0_1_0, and set properties
  set FC_CIF_0_1_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:FC_CIF_0_1:1.0 FC_CIF_0_1_0 ]

  # Create instance: FC_CIF_0_2_0, and set properties
  set FC_CIF_0_2_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:FC_CIF_0_2:1.0 FC_CIF_0_2_0 ]

  # Create instance: SCIG_CIF_0_1_0, and set properties
  set SCIG_CIF_0_1_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:SCIG_CIF_0_1:1.0 SCIG_CIF_0_1_0 ]

  # Create instance: SCIG_CIF_0_2_0, and set properties
  set SCIG_CIF_0_2_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:SCIG_CIF_0_2:1.0 SCIG_CIF_0_2_0 ]

  # Create instance: SCIG_CIF_0_3_0, and set properties
  set SCIG_CIF_0_3_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:SCIG_CIF_0_3:1.0 SCIG_CIF_0_3_0 ]

  # Create instance: SMM_CIF_0_1_0, and set properties
  set SMM_CIF_0_1_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:SMM_CIF_0_1:1.0 SMM_CIF_0_1_0 ]

  # Create instance: SMM_CIF_0_2_0, and set properties
  set SMM_CIF_0_2_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:SMM_CIF_0_2:1.0 SMM_CIF_0_2_0 ]

  # Create instance: SMM_CIF_0_3_0, and set properties
  set SMM_CIF_0_3_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:SMM_CIF_0_3:1.0 SMM_CIF_0_3_0 ]

  # Create instance: axis_register_slice_0, and set properties
  set axis_register_slice_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_register_slice:1.1 axis_register_slice_0 ]

  # Create instance: axis_register_slice_1, and set properties
  set axis_register_slice_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_register_slice:1.1 axis_register_slice_1 ]

  # Create instance: axis_register_slice_2, and set properties
  set axis_register_slice_2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_register_slice:1.1 axis_register_slice_2 ]

  # Create instance: axis_register_slice_3, and set properties
  set axis_register_slice_3 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_register_slice:1.1 axis_register_slice_3 ]

  # Create instance: axis_register_slice_4, and set properties
  set axis_register_slice_4 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_register_slice:1.1 axis_register_slice_4 ]

  # Create instance: axis_register_slice_5, and set properties
  set axis_register_slice_5 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_register_slice:1.1 axis_register_slice_5 ]

  # Create instance: axis_register_slice_6, and set properties
  set axis_register_slice_6 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_register_slice:1.1 axis_register_slice_6 ]

  # Create instance: axis_register_slice_7, and set properties
  set axis_register_slice_7 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_register_slice:1.1 axis_register_slice_7 ]

  # Create instance: axis_register_slice_8, and set properties
  set axis_register_slice_8 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_register_slice:1.1 axis_register_slice_8 ]

  # Create instance: axis_register_slice_9, and set properties
  set axis_register_slice_9 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_register_slice:1.1 axis_register_slice_9 ]

  # Create instance: maxPool_CIF_0_1_0, and set properties
  set maxPool_CIF_0_1_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:maxPool_CIF_0_1:1.0 maxPool_CIF_0_1_0 ]

  # Create instance: maxPool_CIF_0_2_0, and set properties
  set maxPool_CIF_0_2_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:maxPool_CIF_0_2:1.0 maxPool_CIF_0_2_0 ]

  # Create instance: maxPool_CIF_0_3_0, and set properties
  set maxPool_CIF_0_3_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:maxPool_CIF_0_3:1.0 maxPool_CIF_0_3_0 ]

  # Create interface connections
  connect_bd_intf_net -intf_net FC_CIF_0_1_0_out_stream [get_bd_intf_pins FC_CIF_0_1_0/out_stream] [get_bd_intf_pins axis_register_slice_9/S_AXIS]
  connect_bd_intf_net -intf_net FC_CIF_0_2_0_out_stream [get_bd_intf_ports axis_out] [get_bd_intf_pins FC_CIF_0_2_0/out_stream]
  connect_bd_intf_net -intf_net SCIG_CIF_0_1_0_out_r [get_bd_intf_pins SCIG_CIF_0_1_0/out_r] [get_bd_intf_pins axis_register_slice_0/S_AXIS]
  connect_bd_intf_net -intf_net SCIG_CIF_0_2_0_out_r [get_bd_intf_pins SCIG_CIF_0_2_0/out_r] [get_bd_intf_pins axis_register_slice_3/S_AXIS]
  connect_bd_intf_net -intf_net SCIG_CIF_0_3_0_out_r [get_bd_intf_pins SCIG_CIF_0_3_0/out_r] [get_bd_intf_pins axis_register_slice_6/S_AXIS]
  connect_bd_intf_net -intf_net SMM_CIF_0_1_0_out_stream [get_bd_intf_pins SMM_CIF_0_1_0/out_stream] [get_bd_intf_pins axis_register_slice_1/S_AXIS]
  connect_bd_intf_net -intf_net SMM_CIF_0_2_0_out_stream [get_bd_intf_pins SMM_CIF_0_2_0/out_stream] [get_bd_intf_pins axis_register_slice_4/S_AXIS]
  connect_bd_intf_net -intf_net SMM_CIF_0_3_0_out_stream [get_bd_intf_pins SMM_CIF_0_3_0/out_stream] [get_bd_intf_pins axis_register_slice_7/S_AXIS]
  connect_bd_intf_net -intf_net axis_in_1 [get_bd_intf_ports axis_in] [get_bd_intf_pins SCIG_CIF_0_1_0/in_r]
  connect_bd_intf_net -intf_net axis_register_slice_0_M_AXIS [get_bd_intf_pins SMM_CIF_0_1_0/in_stream_a] [get_bd_intf_pins axis_register_slice_0/M_AXIS]
  connect_bd_intf_net -intf_net axis_register_slice_1_M_AXIS [get_bd_intf_pins axis_register_slice_1/M_AXIS] [get_bd_intf_pins maxPool_CIF_0_1_0/in_r]
  connect_bd_intf_net -intf_net axis_register_slice_2_M_AXIS [get_bd_intf_pins SCIG_CIF_0_2_0/in_r] [get_bd_intf_pins axis_register_slice_2/M_AXIS]
  connect_bd_intf_net -intf_net axis_register_slice_3_M_AXIS [get_bd_intf_pins SMM_CIF_0_2_0/in_stream_a] [get_bd_intf_pins axis_register_slice_3/M_AXIS]
  connect_bd_intf_net -intf_net axis_register_slice_4_M_AXIS [get_bd_intf_pins axis_register_slice_4/M_AXIS] [get_bd_intf_pins maxPool_CIF_0_2_0/in_r]
  connect_bd_intf_net -intf_net axis_register_slice_5_M_AXIS [get_bd_intf_pins SCIG_CIF_0_3_0/in_r] [get_bd_intf_pins axis_register_slice_5/M_AXIS]
  connect_bd_intf_net -intf_net axis_register_slice_6_M_AXIS [get_bd_intf_pins SMM_CIF_0_3_0/in_stream_a] [get_bd_intf_pins axis_register_slice_6/M_AXIS]
  connect_bd_intf_net -intf_net axis_register_slice_7_M_AXIS [get_bd_intf_pins axis_register_slice_7/M_AXIS] [get_bd_intf_pins maxPool_CIF_0_3_0/in_r]
  connect_bd_intf_net -intf_net axis_register_slice_8_M_AXIS [get_bd_intf_pins FC_CIF_0_1_0/in_stream_a] [get_bd_intf_pins axis_register_slice_8/M_AXIS]
  connect_bd_intf_net -intf_net axis_register_slice_9_M_AXIS [get_bd_intf_pins FC_CIF_0_2_0/in_stream_a] [get_bd_intf_pins axis_register_slice_9/M_AXIS]
  connect_bd_intf_net -intf_net maxPool_CIF_0_1_0_out_r [get_bd_intf_pins axis_register_slice_2/S_AXIS] [get_bd_intf_pins maxPool_CIF_0_1_0/out_r]
  connect_bd_intf_net -intf_net maxPool_CIF_0_2_0_out_r [get_bd_intf_pins axis_register_slice_5/S_AXIS] [get_bd_intf_pins maxPool_CIF_0_2_0/out_r]
  connect_bd_intf_net -intf_net maxPool_CIF_0_3_0_out_r [get_bd_intf_pins axis_register_slice_8/S_AXIS] [get_bd_intf_pins maxPool_CIF_0_3_0/out_r]

  # Create port connections
  connect_bd_net -net clk_in_1 [get_bd_ports clk_in] [get_bd_pins FC_CIF_0_1_0/ap_clk] [get_bd_pins FC_CIF_0_2_0/ap_clk] [get_bd_pins SCIG_CIF_0_1_0/ap_clk] [get_bd_pins SCIG_CIF_0_2_0/ap_clk] [get_bd_pins SCIG_CIF_0_3_0/ap_clk] [get_bd_pins SMM_CIF_0_1_0/ap_clk] [get_bd_pins SMM_CIF_0_2_0/ap_clk] [get_bd_pins SMM_CIF_0_3_0/ap_clk] [get_bd_pins axis_register_slice_0/aclk] [get_bd_pins axis_register_slice_1/aclk] [get_bd_pins axis_register_slice_2/aclk] [get_bd_pins axis_register_slice_3/aclk] [get_bd_pins axis_register_slice_4/aclk] [get_bd_pins axis_register_slice_5/aclk] [get_bd_pins axis_register_slice_6/aclk] [get_bd_pins axis_register_slice_7/aclk] [get_bd_pins axis_register_slice_8/aclk] [get_bd_pins axis_register_slice_9/aclk] [get_bd_pins maxPool_CIF_0_1_0/ap_clk] [get_bd_pins maxPool_CIF_0_2_0/ap_clk] [get_bd_pins maxPool_CIF_0_3_0/ap_clk]
  connect_bd_net -net rsn_in_1 [get_bd_ports rsn_in] [get_bd_pins FC_CIF_0_1_0/ap_rst_n] [get_bd_pins FC_CIF_0_2_0/ap_rst_n] [get_bd_pins SCIG_CIF_0_1_0/ap_rst_n] [get_bd_pins SCIG_CIF_0_2_0/ap_rst_n] [get_bd_pins SCIG_CIF_0_3_0/ap_rst_n] [get_bd_pins SMM_CIF_0_1_0/ap_rst_n] [get_bd_pins SMM_CIF_0_2_0/ap_rst_n] [get_bd_pins SMM_CIF_0_3_0/ap_rst_n] [get_bd_pins axis_register_slice_0/aresetn] [get_bd_pins axis_register_slice_1/aresetn] [get_bd_pins axis_register_slice_2/aresetn] [get_bd_pins axis_register_slice_3/aresetn] [get_bd_pins axis_register_slice_4/aresetn] [get_bd_pins axis_register_slice_5/aresetn] [get_bd_pins axis_register_slice_6/aresetn] [get_bd_pins axis_register_slice_7/aresetn] [get_bd_pins axis_register_slice_8/aresetn] [get_bd_pins axis_register_slice_9/aresetn] [get_bd_pins maxPool_CIF_0_1_0/ap_rst_n] [get_bd_pins maxPool_CIF_0_2_0/ap_rst_n] [get_bd_pins maxPool_CIF_0_3_0/ap_rst_n]

  # Create address segments

  # Perform GUI Layout
  regenerate_bd_layout -layout_string {
   guistr: "# # String gsaved with Nlview 6.5.12  2016-01-29 bk=1.3547 VDI=39 GEI=35 GUI=JA:1.6
#  -string -flagsOSRD
preplace port rsn_in -pg 1 -y 90 -defaultsOSRD
preplace port clk_in -pg 1 -y 70 -defaultsOSRD
preplace port axis_out -pg 1 -y 290 -defaultsOSRD
preplace port axis_in -pg 1 -y 50 -defaultsOSRD
preplace inst axis_register_slice_7 -pg 1 -lvl 16 -y 270 -defaultsOSRD
preplace inst maxPool_CIF_0_2_0 -pg 1 -lvl 11 -y 240 -defaultsOSRD
preplace inst axis_register_slice_8 -pg 1 -lvl 18 -y 270 -defaultsOSRD
preplace inst maxPool_CIF_0_3_0 -pg 1 -lvl 17 -y 270 -defaultsOSRD
preplace inst axis_register_slice_9 -pg 1 -lvl 20 -y 270 -defaultsOSRD
preplace inst SCIG_CIF_0_1_0 -pg 1 -lvl 1 -y 80 -defaultsOSRD
preplace inst FC_CIF_0_2_0 -pg 1 -lvl 21 -y 290 -defaultsOSRD
preplace inst FC_CIF_0_1_0 -pg 1 -lvl 19 -y 270 -defaultsOSRD
preplace inst SMM_CIF_0_2_0 -pg 1 -lvl 9 -y 200 -defaultsOSRD
preplace inst SCIG_CIF_0_2_0 -pg 1 -lvl 7 -y 160 -defaultsOSRD
preplace inst axis_register_slice_0 -pg 1 -lvl 2 -y 100 -defaultsOSRD
preplace inst axis_register_slice_1 -pg 1 -lvl 4 -y 140 -defaultsOSRD
preplace inst SMM_CIF_0_1_0 -pg 1 -lvl 3 -y 120 -defaultsOSRD
preplace inst axis_register_slice_2 -pg 1 -lvl 6 -y 170 -defaultsOSRD
preplace inst maxPool_CIF_0_1_0 -pg 1 -lvl 5 -y 160 -defaultsOSRD
preplace inst axis_register_slice_3 -pg 1 -lvl 8 -y 180 -defaultsOSRD
preplace inst SMM_CIF_0_3_0 -pg 1 -lvl 15 -y 270 -defaultsOSRD
preplace inst axis_register_slice_4 -pg 1 -lvl 10 -y 220 -defaultsOSRD
preplace inst axis_register_slice_5 -pg 1 -lvl 12 -y 240 -defaultsOSRD
preplace inst axis_register_slice_6 -pg 1 -lvl 14 -y 250 -defaultsOSRD
preplace inst SCIG_CIF_0_3_0 -pg 1 -lvl 13 -y 230 -defaultsOSRD
preplace netloc axis_register_slice_4_M_AXIS 1 10 1 N
preplace netloc axis_register_slice_6_M_AXIS 1 14 1 N
preplace netloc maxPool_CIF_0_3_0_out_r 1 17 1 4420
preplace netloc SCIG_CIF_0_1_0_out_r 1 1 1 N
preplace netloc maxPool_CIF_0_2_0_out_r 1 11 1 2860
preplace netloc axis_register_slice_3_M_AXIS 1 8 1 N
preplace netloc axis_register_slice_9_M_AXIS 1 20 1 N
preplace netloc FC_CIF_0_1_0_out_stream 1 19 1 4980
preplace netloc SMM_CIF_0_1_0_out_stream 1 3 1 N
preplace netloc SCIG_CIF_0_2_0_out_r 1 7 1 N
preplace netloc SMM_CIF_0_3_0_out_stream 1 15 1 3940
preplace netloc SMM_CIF_0_2_0_out_stream 1 9 1 N
preplace netloc clk_in_1 1 0 21 30 160 340 170 540 190 870 210 1080 230 1340 240 1550 240 1870 250 2070 270 2400 290 2610 310 2870 310 3090 310 3410 320 3610 340 3950 340 4170 340 4430 340 4650 340 4990 340 NJ
preplace netloc SCIG_CIF_0_3_0_out_r 1 13 1 N
preplace netloc axis_register_slice_2_M_AXIS 1 6 1 1540
preplace netloc axis_register_slice_5_M_AXIS 1 12 1 3080
preplace netloc axis_register_slice_8_M_AXIS 1 18 1 4640
preplace netloc rsn_in_1 1 0 21 20 170 330 180 550 200 880 220 1090 240 1330 250 1560 250 1860 260 2080 280 2410 300 2620 320 2880 320 3100 320 3400 330 3620 350 3960 350 4180 350 4440 350 4660 350 5000 350 NJ
preplace netloc axis_register_slice_7_M_AXIS 1 16 1 4160
preplace netloc maxPool_CIF_0_1_0_out_r 1 5 1 1340
preplace netloc axis_register_slice_1_M_AXIS 1 4 1 N
preplace netloc axis_register_slice_0_M_AXIS 1 2 1 N
preplace netloc axis_in_1 1 0 1 NJ
preplace netloc FC_CIF_0_2_0_out_stream 1 21 1 N
levelinfo -pg 1 0 180 440 710 980 1210 1440 1710 1970 2240 2510 2740 2980 3250 3510 3780 4060 4300 4540 4820 5100 5370 5550 -top 0 -bot 360
",
}

  # Restore current instance
  current_bd_instance $oldCurInst

  save_bd_design
}
# End of create_root_design()

##################################################################
# MAIN FLOW
##################################################################

create_root_design ""

ipx::package_project -root_dir /home/awai54st/Documents/PYNQ-Classification/hw/graphic_design_flow/CIFAR-10/SKETCHPAD -vendor user.org -library user -taxonomy /UserIP -module sketchpad


