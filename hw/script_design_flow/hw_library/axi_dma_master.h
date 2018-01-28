
void AXI_DMA_MASTER(
	hls::stream<AXI_VAL> &in_stream,
	hls::stream<AXI_DMA_IO> &out_stream
){

	AXI_DMA_IO valOut;
	AXI_VAL valIn;

	valIn = in_stream.read();
	unsigned status = (unsigned)valIn;
	valOut.data = valIn; valOut.last = 0;
	out_stream.write(valOut);

	valIn = in_stream.read();
	unsigned batch_size = (unsigned)valIn;
	valOut.data = valIn; valOut.last = 0;
	out_stream.write(valOut);

	valIn = in_stream.read();
	unsigned ConvKernelDim = (unsigned)valIn;
	valOut.data = valIn; valOut.last = 0;
	out_stream.write(valOut);

	valIn = in_stream.read();
	unsigned IFMChannels = (unsigned)valIn;
	valOut.data = valIn; valOut.last = 0;
	out_stream.write(valOut);

	valIn = in_stream.read();
	unsigned IFMDim = (unsigned)valIn;
	valOut.data = valIn; valOut.last = 0;
	out_stream.write(valOut);

	valIn = in_stream.read();
	unsigned OFMChannels = (unsigned)valIn;
	valOut.data = valIn; valOut.last = 0;
	out_stream.write(valOut);

	valIn = in_stream.read();
	unsigned OFMDim = (unsigned)valIn;
	valOut.data = valIn; valOut.last = 0;
	out_stream.write(valOut);

	valIn = in_stream.read();
	unsigned PadDim = (unsigned)valIn;
	valOut.data = valIn; valOut.last = 0;
	out_stream.write(valOut);

	if (status == 0) {// execution
		unsigned int OFM_size_0 = OFMChannels * OFMDim;
		unsigned int OFM_size_1 = OFM_size_0 * OFMDim;
		unsigned int OFM_bound = OFM_size_1 * batch_size;
#pragma HLS RESOURCE variable=OFM_size_0 core=Mul_LUT
#pragma HLS RESOURCE variable=OFM_size_1 core=Mul_LUT
#pragma HLS RESOURCE variable=OFM_bound core=Mul_LUT

		for(unsigned int i = 0; i < OFM_bound; i++){
#pragma HLS PIPELINE II=1
			valOut.data = in_stream.read();
			if (i==OFM_bound-1) valOut.last = 1;
			else valOut.last = 0;
			out_stream.write(valOut);
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
			valOut.data = in_stream.read();
			if (i==KER_bound-1) valOut.last = 1;
			else valOut.last = 0;
			out_stream.write(valOut);
		}
	}



}
