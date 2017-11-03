#include"CIFAR_10_wrapper.h"
#include "../hw_library/config.h"

void cifar_10(
	hls::stream<AXI_VAL> &in_stream,
	hls::stream<AXI_VAL> &out_stream
){
#pragma HLS DATAFLOW
#pragma HLS INTERFACE axis port=in_stream
#pragma HLS INTERFACE axis port=out_stream
#pragma HLS INTERFACE ap_ctrl_none port=return

	// Defining Interconnections
	hls::stream<AXI_VAL> connect_0, connect_1, connect_2, connect_3, connect_4, connect_5, connect_6, connect_7, connect_8, connect_9;

//	void SCIG<KerDim_curr, IFMCH_curr, IFMDim_curr, OFMCH_curr, OFMDim_curr>(in, out, PadDim_curr = 0)
//	void SMM<A_COL_MAX, A_ROW_MAX, B_COL_MAX>(in_stream_a, out_stream, layer_id, output_rectify = 1, FACTOR)
//	void pool<pool_size, In_CH_MAX, IFMDim_MAX> (in, out, layer_id, pool_mode, // 0 for max pooling, 1 for average pooling, const bool output_rectify = 1)
	SCIG<5, 3, 32, 32, 32, 2>(in_stream, connect_0);
	SMM<1, 75, 32>(connect_0, connect_1, 1, 0, 25);
	pool<2, 32, 32>(connect_1, connect_2, 1, 0, 1);
	SCIG<5, 32, 16, 32, 16, 2>(connect_2, connect_3);
	SMM<1, 800, 32>(connect_3, connect_4, 2, 1, 25);
	pool<2, 32, 16>(connect_4, connect_5, 2, 1, 0);
	SCIG<5, 32, 8, 64, 8, 2>(connect_5, connect_6);
	SMM<1, 800, 64>(connect_6, connect_7, 3, 1, 25);
	pool<2, 64, 8>(connect_7, connect_8, 3, 1, 0);
	FC<1, 1024, 64>(connect_8, connect_9, 4, 0, 32);
	FC<1, 64, 10>(connect_9, out_stream, 5, 0, 16);

}

