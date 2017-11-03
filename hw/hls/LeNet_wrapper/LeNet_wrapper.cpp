#include"LeNet_wrapper.h"
#include "../hw_library/config.h"

void lenet(
	hls::stream<AXI_VAL> &in_stream,
	hls::stream<AXI_VAL> &out_stream
){
#pragma HLS DATAFLOW
#pragma HLS INTERFACE axis port=in_stream
#pragma HLS INTERFACE axis port=out_stream
#pragma HLS INTERFACE ap_ctrl_none port=return

	// Defining Interconnections
	hls::stream<AXI_VAL> connect_0, connect_1, connect_2, connect_3, connect_4, connect_5, connect_6, connect_7;

//	void SCIG<KerDim_curr, IFMCH_curr, IFMDim_curr, OFMCH_curr, OFMDim_curr>(in, out, PadDim_curr = 0)
	SCIG<5, 1, 28, 20, 24, 0>(in_stream, connect_0);
//	void SMM<A_COL_MAX, A_ROW_MAX, B_COL_MAX>(in_stream_a, out_stream, layer_id, output_rectify = 1, FACTOR)
	SMM<1, 25, 20>(connect_0, connect_1, 1, 1, 25);
//	void pool<pool_size, In_CH_MAX, IFMDim_MAX> (in, out, layer_id, pool_mode, // 0 for max pooling, 1 for average pooling
//		const bool output_rectify = 1)
	pool<2, 20, 24>(connect_1, connect_2, 1, 0, 0);
	SCIG<5, 20, 12, 50, 8, 0>(connect_2, connect_3);
	SMM<1, 500, 50>(connect_3, connect_4, 2, 1, 25);
	pool<2, 50, 8>(connect_4, connect_5, 2, 0, 0);
	FC<1, 800, 500>(connect_5, connect_6, 3, 1, 25);
	FC<1, 500, 10>(connect_6, out_stream, 4, 1, 25);

}

