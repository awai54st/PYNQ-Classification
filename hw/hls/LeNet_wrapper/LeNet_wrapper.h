#include <assert.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>

#include "../hw_library/config.h"

#include "../hw_library/stream_convolution_slideWindow.h"
#include "../hw_library/fixed_point_stream_convolution.h"
#include "../hw_library/pool.h"
#include "../hw_library/fully_connected.h"
//
//extern void SCIG(
//	hls::stream<AXI_VAL> & in,
//	hls::stream<AXI_VAL> & out,
//	const unsigned KerDim_curr,
//	const unsigned IFMCH_curr,
//	const unsigned IFMDim_curr,
//	const unsigned OFMCH_curr,
//	const unsigned OFMDim_curr,
//	const ap_uint<bitwidth> padValue
//);
//
//template<
//	unsigned A_COL_MAX,
//	unsigned A_ROW_MAX,
//	unsigned B_COL_MAX
//>
//extern void SMM(
//	hls::stream<AXI_VAL> &in_stream_a,
//	hls::stream<AXI_VAL> &out_stream,
//	const unsigned layer_id,
////	const unsigned A_COL_MAX,
////	const unsigned A_ROW_MAX,
////	const unsigned B_COL_MAX,
//	const unsigned output_rectify,
//	const int FACTOR
//);
//
//template<
//	unsigned pool_size,
//	unsigned In_CH_MAX,
//	unsigned IFMDim_MAX
//>
//extern void pool (
//	hls::stream<AXI_VAL> & in,
//	hls::stream<AXI_VAL> & out,
//	const unsigned layer_id,
////	const unsigned pool_size,
////	const unsigned In_CH_MAX,
////	const unsigned IFMDim_MAX,
//	const unsigned pool_mode, // 0 for max pooling, 1 for average pooling
//	const bool output_rectify
//);
//
//template<
//	unsigned A_COL_MAX,
//	unsigned A_ROW_MAX,
//	unsigned B_COL_MAX
//>
//extern void FC(
//	hls::stream<AXI_VAL> &in_stream_a,
//	hls::stream<AXI_VAL> &out_stream,
//	const unsigned layer_id,
////	const unsigned A_COL_MAX,
////	const unsigned A_ROW_MAX,
////	const unsigned B_COL_MAX,
//	const unsigned output_rectify,
//	const int FACTOR
//);
