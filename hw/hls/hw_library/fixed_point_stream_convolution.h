
template<
	unsigned A_COL_MAX,
	unsigned A_ROW_MAX,
	unsigned B_COL_MAX
>
void SMM(
	hls::stream<AXI_VAL> &in_stream_a, 
	hls::stream<AXI_VAL> &out_stream,
	const unsigned layer_id,
	const unsigned output_rectify,
	const int FACTOR
){

	const unsigned B_ROW_MAX = A_ROW_MAX;
	static ap_int<InpWidth> A[A_COL_MAX][A_ROW_MAX], B[B_COL_MAX][B_ROW_MAX];
#pragma HLS RESOURCE variable=A core=RAM_S2P_LUTRAM
#pragma HLS RESOURCE variable=B core=RAM_S2P_BRAM
#pragma HLS array_partition variable=A block factor=FACTOR dim=2
#pragma HLS array_partition variable=B block factor=FACTOR dim=2
	AXI_VAL valIn_a, valOut;

	// first data showing mode.
	// 0 - CNN forward propagation
	// 1 - weight loading
	valIn_a = in_stream_a.read();
	unsigned status = (unsigned)valIn_a;
	out_stream.write(valIn_a);

	valIn_a = in_stream_a.read();
	unsigned batch_size = (unsigned)valIn_a;
	out_stream.write(valIn_a);

	valIn_a = in_stream_a.read();
	unsigned ConvKernelDim = (unsigned)valIn_a;
	out_stream.write(valIn_a);

	valIn_a = in_stream_a.read();
	unsigned IFMChannels = (unsigned)valIn_a;
	out_stream.write(valIn_a);

	valIn_a = in_stream_a.read();
	unsigned IFMDim = (unsigned)valIn_a;
	out_stream.write(valIn_a);

	valIn_a = in_stream_a.read();
	unsigned OFMChannels = (unsigned)valIn_a;
	out_stream.write(valIn_a);

	valIn_a = in_stream_a.read();
	unsigned OFMDim = (unsigned)valIn_a;
	out_stream.write(valIn_a);

	valIn_a = in_stream_a.read();
	unsigned PadDim = (unsigned)valIn_a;
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
					B[i][j] = valIn_a;
					valOut = valIn_a;
					out_stream.write(valOut);
				}
				else {
					B[i][j] = 0;
				}
			}

	}
	// convolution
	else if (status == 0){
		for (int num_imag=0; num_imag < batch_size; num_imag++){

			A_COL = OFMDim_current*OFMDim_current;
			A_ROW = B_ROW;

			A_COL_ITER = A_COL;
			for (int iter=0; iter<A_COL_ITER; iter++){

				for(int i=0; i<A_COL_MAX; i++)
					for(int j=0; j<A_ROW_MAX; j++)
					{
#pragma HLS PIPELINE II=1
						if (j < A_ROW) {
							valIn_a = in_stream_a.read();
							A[i][j] = valIn_a;
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
					{
						ap_int<bitwidth> sum = 0;
						L3:for(int ic = 0; ic < B_ROW_MAX/FACTOR; ++ic){
#pragma HLS PIPELINE II=1
							L4:for(int id = 0; id < FACTOR; ++id){
								sum += A[ia][id*B_ROW_MAX/FACTOR+ic] * B[ib][id*B_ROW_MAX/FACTOR+ic];
							}
						}
						int output_data = sum/quant_scale;
						valOut = (output_rectify)?(MAX(0, output_data)) : (output_data);
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
#pragma HLS PIPELINE II=1
			valIn_a = in_stream_a.read();
			out_stream.write(valIn_a);
		}
	}
}
