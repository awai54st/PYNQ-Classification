
void AXI_DMA_SLAVE(
	hls::stream<AXI_DMA_IO> &in_stream,
	hls::stream<AXI_VAL> &out_stream
){

	AXI_DMA_IO valIn;

	valIn = in_stream.read();
	unsigned status = (unsigned)valIn.data;
	out_stream.write(valIn.data);

	valIn = in_stream.read();
	unsigned batch_size = (unsigned)valIn.data;
	out_stream.write(valIn.data);

	valIn = in_stream.read();
	unsigned ConvKernelDim = (unsigned)valIn.data;
	out_stream.write(valIn.data);

	valIn = in_stream.read();
	unsigned IFMChannels = (unsigned)valIn.data;
	out_stream.write(valIn.data);

	valIn = in_stream.read();
	unsigned IFMDim = (unsigned)valIn.data;
	out_stream.write(valIn.data);

	valIn = in_stream.read();
	unsigned OFMChannels = (unsigned)valIn.data;
	out_stream.write(valIn.data);

	valIn = in_stream.read();
	unsigned OFMDim = (unsigned)valIn.data;
	out_stream.write(valIn.data);

	valIn = in_stream.read();
	unsigned PadDim = (unsigned)valIn.data;
	out_stream.write(valIn.data);

	if (status == 0) {// execution
		unsigned int IFM_size_0 = IFMChannels * IFMDim;
		unsigned int IFM_size_1 = IFM_size_0 * IFMDim;
		unsigned int IFM_bound = IFM_size_1 * batch_size;
#pragma HLS RESOURCE variable=IFM_size_0 core=Mul_LUT
#pragma HLS RESOURCE variable=IFM_size_1 core=Mul_LUT
#pragma HLS RESOURCE variable=IFM_bound core=Mul_LUT

		for(unsigned int i = 0; i < IFM_bound; i++){
#pragma HLS PIPELINE II=1
			valIn = in_stream.read();
			out_stream.write(valIn.data);
		}
	}
	else{// weight loading
		unsigned int KER_size_0 = OFMChannels*ConvKernelDim;
		unsigned int KER_size_1 = KER_size_0*ConvKernelDim;
		unsigned int KER_bound = KER_size_1*IFMChannels;
#pragma HLS RESOURCE variable=KER_size_0 core=Mul_LUT
#pragma HLS RESOURCE variable=KER_size_1 core=Mul_LUT
#pragma HLS RESOURCE variable=KER_bound core=Mul_LUT

		for(unsigned int i = 0; i < KER_bound; i++){
#pragma HLS PIPELINE II=1
			valIn = in_stream.read();
			out_stream.write(valIn.data);
		}
	}
}
