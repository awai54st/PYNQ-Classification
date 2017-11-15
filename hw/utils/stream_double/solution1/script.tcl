############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2016 Xilinx, Inc. All Rights Reserved.
############################################################
open_project stream_double
set_top stream_double
add_files stream_double/stream_double.cpp
open_solution "solution1"
set_part {xc7z020clg484-1} -tool vivado
create_clock -period 10 -name default
#source "./stream_double/solution1/directives.tcl"
#csim_design
csynth_design
#cosim_design
export_design -format ip_catalog -description "Doubles the values in a stream" -vendor "Xilinx" -display_name "Stream Double"
