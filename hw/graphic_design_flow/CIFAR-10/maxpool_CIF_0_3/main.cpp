//#include <assert.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>

#define bitwidth 32

#define IFMChannels 3
#define InpWidth 8

//typedef ap_axiu<32,1,1,1> AXI_VAL;
struct AXI_VAL{
	ap_int<IFMChannels*InpWidth> data;
	bool last;
};

//template<unsigned int ConvKernelDim, unsigned int IFMChannels,
//		unsigned int IFMDim, unsigned int OFMDim, unsigned int InpWidth, unsigned int PadDim>
void maxPool_CIF_0_3(hls::stream<AXI_VAL> & in, hls::stream<AXI_VAL> & out);


int main (){

	hls::stream<AXI_VAL> in_stream;
	hls::stream<AXI_VAL> out_stream;

	AXI_VAL valIn;

	unsigned int status;
	const unsigned int batch_size = 1;
	const unsigned int Ker_DIM = 3;
	const unsigned int In_DIM = 8;
	const unsigned int In_CH = 1;
	const unsigned int Out_DIM = 8;
	const unsigned int Out_CH = 1;
	const unsigned int PadDim = 1;

	/////////////////////////////////Test for B/////////////////////////////////
	status = 3;

	valIn.data = status;
	in_stream << valIn;
	valIn.data = batch_size;
	in_stream << valIn;
	valIn.data = Ker_DIM;
	in_stream << valIn;
	valIn.data = In_CH;
	in_stream << valIn;
	valIn.data = In_DIM;
	in_stream << valIn;
	valIn.data = Out_CH;
	in_stream << valIn;
	valIn.data = Out_DIM;
	in_stream << valIn;
	valIn.data = PadDim;
	in_stream << valIn;

	int kernel [Ker_DIM*Ker_DIM*Out_CH] = {0, 10, 20, 30, 40, 50, 60, 70, 80};
	//	input = {1,1,1,1,0,0,1,1,0,1,1,0,0,1,1,0};

	for (int i = 0; i < Out_CH*In_CH; i ++){
		for(int j =0; j < Ker_DIM*Ker_DIM; j++){
			valIn.data = kernel[j];
			in_stream << valIn;
		}
	}

	maxPool_CIF_0_3(in_stream, out_stream);

	AXI_VAL parOut;
	out_stream.read(parOut);printf("status is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("batch_size is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("Ker_DIM is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("In_CH is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("In_DIM is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("Out_CH is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("Out_DIM is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("PadDim is %d \n", (int)parOut.data);

	int counter_B = 0;
	ap_int<bitwidth> sum_B;
	for (int j = 0; j < Ker_DIM*Ker_DIM*Out_CH*In_CH; j ++){
		AXI_VAL valOut;
		out_stream.read(valOut);
		sum_B = valOut.data;

		printf("result is %d, last signal is %d \n", (int)sum_B, (int)valOut.last);
		counter_B ++;
	}

	printf("%d results received \n", counter_B);

	/////////////////////////////////Test for A/////////////////////////////
	status = 0;

	valIn.data = status;
	in_stream << valIn;
	valIn.data = batch_size;
	in_stream << valIn;
	valIn.data = Ker_DIM;
	in_stream << valIn;
	valIn.data = In_CH;
	in_stream << valIn;
	valIn.data = In_DIM;
	in_stream << valIn;
	valIn.data = Out_CH;
	in_stream << valIn;
	valIn.data = Out_DIM;
	in_stream << valIn;
	valIn.data = PadDim;
	in_stream << valIn;

	int input [In_DIM*In_DIM*batch_size] = {-1200, -1000, 600, 0, 0, 0, 0, 0,  -600, -1200, 1000, 600, 0, 0, 0, 0,   200, 600, 1200, 1000, 600, 0, 0, 0,  0, 200, 600, 1200, 1000, 600, 0, 0,   0, 0, 200, 600, 1200, 1000, 600, 0,  0, 0, 0, 200, 600, 1200, 1000, 600,   0, 0, 0, 0, 200, 600, 1200, 1000,  0, 0, 0, 0, 0, 200, 600, 1200};
	//	input = {1,1,1,1,0,0,1,1,0,1,1,0,0,1,1,0};

	for (int i = 0; i < In_DIM*In_DIM*batch_size; i ++){
		for(int j =0; j < In_CH; j++){
			valIn.data = input[i];
			in_stream << valIn;
		}
	}

	maxPool_CIF_0_3(in_stream, out_stream);

	out_stream.read(parOut);printf("status is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("batch_size is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("Ker_DIM is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("In_CH is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("In_DIM is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("Out_CH is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("Out_DIM is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("PadDim is %d \n", (int)parOut.data);

	int counter = 0;
	ap_int<bitwidth> sum;
	for (int j = 0; j < Out_DIM*Out_DIM*Out_CH*batch_size/4; j ++){
		AXI_VAL valOut;
		out_stream.read(valOut);
		sum = valOut.data;

		printf("result is %d, last signal is %d \n", (int)sum, (int)valOut.last);
		counter ++;
	}

	printf("%d results received \n", counter);

	return 0;

}
