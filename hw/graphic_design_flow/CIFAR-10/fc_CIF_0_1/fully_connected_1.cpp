#include <assert.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>

#define bitwidth 32
#define wordwidth 16
#define quant_scale 32768// 2^(wordwidth-1)

//typedef ap_axiu<32,1,1,1> AXI_VAL;
struct AXI_VAL{
	ap_int<bitwidth> data;
	bool last;
};
//#define A_COL_ITER 32
/////////////////////////////
// Paramteres //
#define A_COL_MAX 1
#define A_ROW_MAX 1024

#define B_COL_MAX 64
#define B_ROW_MAX A_ROW_MAX

#define C_COL_MAX A_COL_MAX
#define C_ROW_MAX B_ROW_MAX

#define layer_id 4
#define output_rectify 0
/////////////////////////////

#define inElem_MAX 57600

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) > (y)) ? (y) : (x))

void FC_CIF_0_1(hls::stream<AXI_VAL> &in_stream_a, hls::stream<AXI_VAL> &out_stream)
{

#pragma HLS INTERFACE axis port=in_stream_a
	//#pragma HLS INTERFACE axis port=in_stream_b
#pragma HLS INTERFACE axis port=out_stream
#pragma HLS INTERFACE ap_ctrl_none port=return

	static ap_int<wordwidth> A[A_COL_MAX][A_ROW_MAX], B[B_COL_MAX][B_ROW_MAX], C[C_COL_MAX][C_ROW_MAX];
	int const FACTOR = 32;
#pragma HLS array_partition variable=A block factor=FACTOR dim=2
#pragma HLS array_partition variable=B block factor=FACTOR dim=2
	AXI_VAL valIn_a, valOut;

	// first data showing mode.
	// 0 - CNN forward propagation
	// 1 - weight loading
	valIn_a = in_stream_a.read();
	unsigned status = (unsigned)valIn_a.data;
	out_stream.write(valIn_a);

	valIn_a = in_stream_a.read();
	unsigned batch_size = (unsigned)valIn_a.data;
	out_stream.write(valIn_a);

	valIn_a = in_stream_a.read();
	unsigned ConvKernelDim = (unsigned)valIn_a.data;
	out_stream.write(valIn_a);

	valIn_a = in_stream_a.read();
	unsigned IFMChannels = (unsigned)valIn_a.data;
	out_stream.write(valIn_a);

	valIn_a = in_stream_a.read();
	unsigned IFMDim = (unsigned)valIn_a.data;
	out_stream.write(valIn_a);

	valIn_a = in_stream_a.read();
	unsigned OFMChannels = (unsigned)valIn_a.data;
	out_stream.write(valIn_a);

	valIn_a = in_stream_a.read();
	unsigned OFMDim = (unsigned)valIn_a.data;
	out_stream.write(valIn_a);

	valIn_a = in_stream_a.read();
	unsigned PadDim = (unsigned)valIn_a.data;
	out_stream.write(valIn_a);

	// then, first two data as row_size and col_size
	static unsigned A_COL = A_COL_MAX;
	static unsigned A_ROW = A_ROW_MAX;
	static unsigned B_COL = B_COL_MAX;
	static unsigned B_ROW = B_ROW_MAX;
	static unsigned C_COL = A_COL_MAX;
	static unsigned C_ROW = B_ROW_MAX;
	int A_COL_ITER;


	// stream in the 2 input matrices
	static unsigned OFMDim_current;

	// store weight for current layer
	if (status == layer_id) {

		B_COL = OFMChannels;
		B_ROW = ConvKernelDim*ConvKernelDim*IFMChannels;
		OFMDim_current = OFMDim;

		for (int i=0; i<B_COL_MAX; i++)
			for (int j=0; j<B_ROW_MAX; j++)
			{
#pragma HLS PIPELINE II=1
				if (j < B_ROW && i < B_COL) {
					valIn_a = in_stream_a.read();
					B[i][j] = valIn_a.data;
					valOut.data = valIn_a.data;
					if (i==B_COL-1 && j==B_ROW-1)valOut.last = 1;
					else valOut.last = 0;
					out_stream.write(valOut);
				}
				else {
					B[i][j] = 0;
				}
				//				valOut.data = sum;
				//				if (i==B_COL_MAX-1 && j==B_ROW_MAX-1) valOut.last = 1;
				//				else valOut.last = 0;
				//				out_stream.write(valOut);
			}

	}
	// convolution
	else if (status == 0){
		for (int num_imag=0; num_imag < batch_size; num_imag++){
//#pragma HLS LOOP_TRIPCOUNT min=600 max=600 avg=600

			A_COL = OFMDim_current*OFMDim_current;
			//A_ROW = ConvKernelDim*ConvKernelDim*IFMChannels;
			A_ROW = B_ROW;

			A_COL_ITER = A_COL/A_COL_MAX;
			for (int iter=0; iter<A_COL_ITER; iter++){
//#pragma HLS LOOP_TRIPCOUNT min=1 max=1 avg=1

				for(int i=0; i<A_COL_MAX; i++)
					for(int j=0; j<A_ROW_MAX; j++)
					{
#pragma HLS PIPELINE II=1
						if (j < A_ROW) {
							valIn_a = in_stream_a.read();
							A[i][j] = valIn_a.data;
						}
						else {
							A[i][j] = 0;
						}
					}

				////////////////////////////////////////////////////////
				// matrix multiplication of a A*B matrix
				L1:for (int ia = 0; ia < A_COL_MAX; ++ia)
				{
					L2:for (int ib = 0; ib < B_COL; ++ib)
//#pragma HLS LOOP_TRIPCOUNT min=500 max=500 avg=500
					{
						ap_int<bitwidth> sum = 0;
						L3:for(int ic = 0; ic < B_ROW_MAX/FACTOR; ++ic){
#pragma HLS PIPELINE II=1
							L4:for(int id = 0; id < FACTOR; ++id){
								sum += A[ia][id*B_ROW_MAX/FACTOR+ic] * B[ib][id*B_ROW_MAX/FACTOR+ic];
							}
						}
						int output_data = sum/quant_scale;
						valOut.data = (output_rectify)?(MAX(0, output_data)) : (output_data);
						if (ia+iter*A_COL_MAX==A_COL-1 && ib==B_COL-1 && num_imag==batch_size-1) valOut.last = 1;
						else valOut.last = 0;
						out_stream.write(valOut);
					}
				}

			}

		}

	}
	// pass filters for other layers
	else{
		unsigned int KER_size_0 = OFMChannels*ConvKernelDim;
		unsigned int KER_size_1 = KER_size_0*ConvKernelDim;
		unsigned int KER_bound = KER_size_1*IFMChannels;
#pragma HLS RESOURCE variable=KER_size_0 core=Mul_LUT
#pragma HLS RESOURCE variable=KER_size_1 core=Mul_LUT
#pragma HLS RESOURCE variable=KER_bound core=Mul_LUT
		for(unsigned int i = 0; i < KER_bound; i++){
//#pragma HLS LOOP_TRIPCOUNT min=100 max=100 avg=100
#pragma HLS PIPELINE II=1
			valIn_a = in_stream_a.read();
			out_stream.write(valIn_a);
		}
	}
}
