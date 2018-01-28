############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2016 Xilinx, Inc. All Rights Reserved.
############################################################
open_project -reset SCIG_CIF_0_2
set_top SCIG_CIF_0_2
add_files stream_convolution_slideWindow.cpp
add_files -tb main.cpp
open_solution -reset "solution1"
set_part {xc7z020clg484-1} -tool vivado
create_clock -period 10 -name default
#source "./SCIG_CIF_0_2/solution1/directives.tcl"
#csim_design
csynth_design
#cosim_design
export_design -format ip_catalog
exit
