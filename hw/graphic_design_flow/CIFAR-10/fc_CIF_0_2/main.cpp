//#include <assert.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>

#define bitwidth 32

//typedef ap_axiu<32,1,1,1> AXI_VAL;
struct AXI_VAL{
	ap_uint<bitwidth> data;
	bool last;
};

void FC_2(hls::stream<AXI_VAL> &in_stream_a, hls::stream<AXI_VAL> &out_stream);

int main (){

	hls::stream<AXI_VAL> in_stream_a;
	hls::stream<AXI_VAL> in_stream_b;
	hls::stream<AXI_VAL> out_stream;

	unsigned A_ROW;
	unsigned A_COL;
	unsigned B_ROW;
	unsigned B_COL;
	AXI_VAL valIn_a, valIn_b;

	unsigned int status = 1;
	const unsigned int batch_size = 1;
	const unsigned int Ker_DIM = 3;
	const unsigned int In_DIM = 8;
	const unsigned int In_CH = 1;
	const unsigned int Out_DIM = 8;
	const unsigned int Out_CH = 1;
	const unsigned int PadDim = 1;

	/////////////////////////////////Test for B/////////////////////////////

	valIn_a.data = status;
	in_stream_a << valIn_a;
	valIn_a.data = batch_size;
	in_stream_a << valIn_a;
	valIn_a.data = Ker_DIM;
	in_stream_a << valIn_a;
	valIn_a.data = In_CH;
	in_stream_a << valIn_a;
	valIn_a.data = In_DIM;
	in_stream_a << valIn_a;
	valIn_a.data = Out_CH;
	in_stream_a << valIn_a;
	valIn_a.data = Out_DIM;
	in_stream_a << valIn_a;
	valIn_a.data = PadDim;
	in_stream_a << valIn_a;

	int kernel [Ker_DIM*Ker_DIM] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

	for (int i = 0; i < Out_CH; i ++){
		for(int j =0; j < Ker_DIM*Ker_DIM; j++){
			valIn_a.data = kernel[j];
			in_stream_a << valIn_a;
		}
	}

	FC_2(in_stream_a, out_stream);

	AXI_VAL parOut;
	out_stream.read(parOut);printf("status is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("batch_size is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("Ker_DIM is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("In_CH is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("In_DIM is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("Out_CH is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("Out_DIM is %d \n", (int)parOut.data);
	out_stream.read(parOut);printf("PadDim is %d \n", (int)parOut.data);

	int counter = 0;
		ap_uint<bitwidth> sum;
		for (int j = 0; j < Ker_DIM*Ker_DIM*Out_CH; j ++){
			AXI_VAL valOut;
			out_stream.read(valOut);
			sum = valOut.data;

			printf("result is %d, last signal is %d \n", (unsigned)sum, (int)valOut.last);
			counter ++;
		}

	//AXI_VAL valOut;
	//out_stream.read(valOut);
	//int end_bit = valOut.data;
	//printf("result is %d, last signal is %d \n", (int)end_bit, (int)valOut.last);
	printf("%d results received \n", counter);

//	/////////////////////////////////Test for A/////////////////////////////
//	status = 0;
//
//	valIn_a.data = status;
//	in_stream_a << valIn_a;
//	valIn_a.data = batch_size;
//	in_stream_a << valIn_a;
//	valIn_a.data = Ker_DIM;
//	in_stream_a << valIn_a;
//	valIn_a.data = In_CH;
//	in_stream_a << valIn_a;
//	valIn_a.data = In_DIM;
//	in_stream_a << valIn_a;
//	valIn_a.data = Out_CH;
//	in_stream_a << valIn_a;
//	valIn_a.data = Out_DIM;
//	in_stream_a << valIn_a;
//	valIn_a.data = PadDim;
//	in_stream_a << valIn_a;
//
//	int input [Out_DIM*Out_DIM*Ker_DIM*Ker_DIM*batch_size] = {1,1,0,0,1,1,0,1,1,1,1,1,0,0,0,1,0,1,1,1,1,1,1,0,0,1,0,1,1,1,1,1,1,0,1,0,  1,1,0,0,1,1,0,1,1,1,1,1,0,0,0,1,0,1,1,1,1,1,1,0,0,1,0,1,1,1,1,1,1,0,1,0};
//	//	input = {1,1,1,1,0,0,1,1,0,1,1,0,0,1,1,0};
//
//	for (int i = 0; i < Out_DIM*Out_DIM*Ker_DIM*Ker_DIM*batch_size; i ++){
//		for(int j =0; j < In_CH; j++){
//			valIn_a.data = input[i];
//			in_stream_a << valIn_a;
//		}
//	}
//
//	FC_2(in_stream_a, out_stream);
//
//	out_stream.read(parOut);printf("status is %d \n", (int)parOut.data);
//	out_stream.read(parOut);printf("batch_size is %d \n", (int)parOut.data);
//	out_stream.read(parOut);printf("Ker_DIM is %d \n", (int)parOut.data);
//	out_stream.read(parOut);printf("In_CH is %d \n", (int)parOut.data);
//	out_stream.read(parOut);printf("In_DIM is %d \n", (int)parOut.data);
//	out_stream.read(parOut);printf("Out_CH is %d \n", (int)parOut.data);
//	out_stream.read(parOut);printf("Out_DIM is %d \n", (int)parOut.data);
//	out_stream.read(parOut);printf("PadDim is %d \n", (int)parOut.data);
//
//	int counter_A = 0;
//	ap_uint<bitwidth> sum_A;
//	for (int j = 0; j < Out_DIM*Out_DIM*Out_CH*batch_size; j ++){
//		AXI_VAL valOut;
//		out_stream.read(valOut);
//		sum_A = valOut.data;
//
//		printf("result is %d, last signal is %d \n", (int)sum_A, (int)valOut.last);
//		counter_A ++;
//	}
//
//	printf("%d results received \n", counter_A);

	return 0;

}
