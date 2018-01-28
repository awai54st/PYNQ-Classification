/******************************************************************************
 *  Copyright (c) 2016, Xilinx, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2.  Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *  3.  Neither the name of the copyright holder nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION). HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 *  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
/******************************************************************************
 *
 *
 * @file slidingwindow.h
 *
 * Library of templated HLS functions for BNN deployment.
 * This file describe the input generator used in the convolutional layer to
 * output the inputfeature map to perform the matrix-vector reduction of a
 * convolution
 *
 *
 *****************************************************************************/

#include <hls_stream.h>
#include <ap_axi_sdata.h>

#define bitwidth 32
#define InpWidth 16

#define inElem_MAX 10500

struct AXI_VAL{
	ap_int<bitwidth> data;
	bool last;
};


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) > (y)) ? (y) : (x))


// II=1 sliding window generator, with common iteration space
//template<unsigned int ConvKernelDim, unsigned int IFMChannels,
//unsigned int IFMDim, unsigned int OFMDim, unsigned int InpWidth=1, unsigned int PadDim=0>
void SCIG_CIF_0_2(hls::stream<AXI_VAL> & in, hls::stream<AXI_VAL> & out, ap_uint<bitwidth> padValue = 0) {

#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE ap_ctrl_none port=return

	AXI_VAL valIn, valOut;

	// first two data as row_size and col_size
	valIn = in.read();
	unsigned status = (unsigned)valIn.data;
	out.write(valIn);

	valIn = in.read();
	unsigned batch_size = (unsigned)valIn.data;
	out.write(valIn);

	valIn = in.read();
	unsigned ConvKernelDim = (unsigned)valIn.data;
	out.write(valIn);

	valIn = in.read();
	unsigned IFMChannels = (unsigned)valIn.data;
	out.write(valIn);

	valIn = in.read();
	unsigned IFMDim = (unsigned)valIn.data;
	out.write(valIn);

	valIn = in.read();
	unsigned OFMChannels = (unsigned)valIn.data;
	out.write(valIn);

	valIn = in.read();
	unsigned OFMDim = (unsigned)valIn.data;
	out.write(valIn);

	valIn = in.read();
	unsigned PadDim = (unsigned)valIn.data;
	out.write(valIn);

	// Registers to store parameters for the current layer

	const unsigned KerDim_curr = 5;
	const unsigned IFMCH_curr = 32;
	const unsigned IFMDim_curr = 16;
	const unsigned OFMCH_curr = 32;
	const unsigned OFMDim_curr = 16;
	const unsigned PadDim_curr = 2;

//	static unsigned KerDim_curr = 0;
//	static unsigned IFMCH_curr = 0;
//	static unsigned IFMDim_curr = 0;
//	static unsigned OFMCH_curr = 0;
//	static unsigned OFMDim_curr = 0;
//	static unsigned PadDim_curr = 0;
//	if (status == 2) {
//		KerDim_curr = ConvKernelDim;
//		IFMCH_curr = IFMChannels;
//		IFMDim_curr = IFMDim;
//		OFMCH_curr = OFMChannels;
//		OFMDim_curr = OFMDim;
//		PadDim_curr = PadDim;
//	}

	if (status == 0){

		//	constexpr unsigned int IFMChanChunk = IFMChannels*InpWidth;
		unsigned int IFMPadDim = IFMDim_curr + 2*PadDim_curr;
		unsigned int IFMPadDimSqrt = IFMPadDim*IFMPadDim;
	#pragma HLS RESOURCE variable=IFMPadDimSqrt core=Mul_LUT
		int IFMLoopBound = IFMDim_curr + PadDim_curr;
		ap_int<InpWidth> inputBuf[inElem_MAX];
	#pragma HLS_RESOURCE variable inputBuf core=RAM_S2P_BRAM

		unsigned int additional_lines = IFMPadDimSqrt/(OFMDim_curr * KerDim_curr * KerDim_curr);
		unsigned int Initial_lines =  ((IFMPadDim) < ((OFMDim_curr * KerDim_curr * KerDim_curr)) ? (KerDim_curr+1) : (KerDim_curr + additional_lines - IFMDim_curr));
		unsigned int Initial_buffer = MIN(Initial_lines * (IFMPadDim),IFMPadDim * IFMPadDim-1);
		unsigned int baseIter = Initial_buffer
				+ (OFMDim_curr * OFMDim_curr * KerDim_curr * KerDim_curr);


		unsigned int inp = 0, oy = 0, ox = 0, ky = 0, kx = 0;
		int inp_i = -PadDim_curr, inp_j = -PadDim_curr;


		//for (unsigned int num_imag = 0; num_imag < batch_size; num_imag++) {
		unsigned int baseIterBound = baseIter*batch_size;
#pragma HLS RESOURCE variable=baseIterBound core=Mul_LUT
		for (unsigned int i = 0; i < baseIter*batch_size; i++) {
#pragma HLS PIPELINE II=1
			if (inp < IFMPadDimSqrt) {
				ap_uint<InpWidth> inElem [256];
#pragma HLS RESOURCE variable=inElem core=RAM_S2P_LUTRAM
				if ((inp_i < 0) || (inp_j < 0) || (inp_i >= IFMDim_curr) || (inp_j >= IFMDim_curr)) {
					//				inElem = padValue;
					for(unsigned int inp_ch=0; inp_ch<IFMCH_curr; inp_ch++){
//#pragma HLS PIPELINE II=1
						inElem[inp_ch] = padValue;
					}
				}
				else {
					//				inElem = in.read();
					for(unsigned int inp_ch=0; inp_ch<IFMCH_curr; inp_ch++){
//#pragma HLS PIPELINE II=1
						AXI_VAL inElem_tmp = in.read();
						inElem[inp_ch] = inElem_tmp.data;
					}
				}
				//			inputBuf[inp] = inElem;
				for(unsigned int inp_ch=0; inp_ch<IFMCH_curr; inp_ch++){
//#pragma HLS PIPELINE II=1
					inputBuf[inp*IFMCH_curr+inp_ch] = inElem[inp_ch];
				}
				inp++;
				inp_j++;
				if(inp_j == IFMLoopBound) {
					inp_j = -PadDim_curr;
					inp_i++;
					if(inp_i == IFMLoopBound) {
						inp_i = -PadDim_curr;
					}
				}
			}
			if (inp > Initial_buffer)
			{
				unsigned int input_base = oy * IFMPadDim + ox;
				unsigned int input_ind = input_base + ky * IFMPadDim + kx;
				//			ap_uint<InpWidth> inElem = inputBuf[input_ind];
				//			out.write(inElem);
				AXI_VAL inElem;
				for(unsigned int inp_ch=0; inp_ch<IFMCH_curr; inp_ch++){
//#pragma HLS PIPELINE II=1
					inElem.data = inputBuf[input_ind*IFMCH_curr+inp_ch];
					out.write(inElem);
				}
				kx++;
				if (kx == KerDim_curr) {
					kx = 0;
					ky++;
					if (ky == KerDim_curr) {
						ky = 0;
						ox++;
						if (ox == OFMDim_curr) {
							ox = 0;
							oy++;
							if (oy == OFMDim_curr) {
								oy = 0;
								inp = 0;
							}
						}
					}
				}
			}
		}
		//}
	}
//	else if (status == 1) {
//		KerDim_curr = ConvKernelDim;
//		IFMCH_curr = IFMChannels;
//		IFMDim_curr = IFMDim;
//		OFMCH_curr = OFMChannels;
//		OFMDim_curr = OFMDim;
//		PadDim_curr = PadDim;
//		for(unsigned int i = 0; i < OFMChannels*ConvKernelDim*ConvKernelDim*IFMChannels; i++){
//			valIn = in.read();
//			out.write(valIn);
//		}
//	}
	else{
		unsigned int KER_size_0 = OFMChannels*ConvKernelDim;
		unsigned int KER_size_1 = KER_size_0*ConvKernelDim;
		unsigned int KER_bound = KER_size_1*IFMChannels;
#pragma HLS RESOURCE variable=KER_size_0 core=Mul_LUT
#pragma HLS RESOURCE variable=KER_size_1 core=Mul_LUT
#pragma HLS RESOURCE variable=KER_bound core=Mul_LUT
		for(unsigned int i = 0; i < KER_bound; i++){
#pragma HLS PIPELINE II=1
			valIn = in.read();
			out.write(valIn);
		}
	}
}
