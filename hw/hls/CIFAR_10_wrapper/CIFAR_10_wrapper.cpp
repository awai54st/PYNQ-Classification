#include"CIFAR_10_wrapper.h"

void cifar_10(
	hls::stream<AXI_DMA_IO> &in_stream,
	hls::stream<AXI_DMA_IO> &out_stream
){
#pragma HLS DATAFLOW
#pragma HLS INTERFACE axis port=in_stream
#pragma HLS INTERFACE axis port=out_stream
#pragma HLS INTERFACE ap_ctrl_none port=return

	// Defining Interconnections
	hls::stream<AXI_VAL> connect_0, connect_1, connect_2, connect_3, connect_4, connect_5, connect_6, connect_7, connect_8, connect_9, connect_10, connect_11;
#pragma HLS STREAM variable=connect_11 depth=50
#pragma HLS STREAM variable=connect_10 depth=50
#pragma HLS STREAM variable=connect_9 depth=50
//#pragma HLS STREAM variable=connect_8 depth=50
#pragma HLS STREAM variable=connect_7 depth=50
#pragma HLS STREAM variable=connect_6 depth=50
//#pragma HLS STREAM variable=connect_5 depth=50
#pragma HLS STREAM variable=connect_4 depth=50
#pragma HLS STREAM variable=connect_3 depth=50
//#pragma HLS STREAM variable=connect_2 depth=50
#pragma HLS STREAM variable=connect_1 depth=50
#pragma HLS STREAM variable=connect_0 depth=50

//	void SCIG<KerDim_curr, IFMCH_curr, IFMDim_curr, OFMCH_curr, OFMDim_curr, PadDim_curr = 0>(in, out)
//	void SMM<A_COL_MAX, A_ROW_MAX, B_COL_MAX>(in_stream_a, out_stream, layer_id, output_rectify = 1, FACTOR)
//	void pool<pool_size, In_CH_MAX, IFMDim_MAX> (in, out, layer_id, pool_mode, // 0 for max pooling, 1 for average pooling, const bool output_rectify = 1)

	AXI_DMA_SLAVE(in_stream, connect_0);
	SCIG<5, 3, 32, 32, 32, 2>(connect_0, connect_1);
	SMM<1, 75, 32>(connect_1, connect_2, 1, 0, 25);
	pool<2, 32, 32>(connect_2, connect_3, 1, 0, 1);
	SCIG<5, 32, 16, 32, 16, 2>(connect_3, connect_4);
	SMM<1, 800, 32>(connect_4, connect_5, 2, 1, 25);
	pool<2, 32, 16>(connect_5, connect_6, 2, 1, 0);
	SCIG<5, 32, 8, 64, 8, 2>(connect_6, connect_7);
	SMM<1, 800, 64>(connect_7, connect_8, 3, 1, 25);
	pool<2, 64, 8>(connect_8, connect_9, 3, 1, 0);
	FC<1, 1024, 64>(connect_9, connect_10, 4, 0, 32);
	FC<1, 64, 10>(connect_10, connect_11, 5, 0, 16);
	AXI_DMA_MASTER(connect_11, out_stream);

}

