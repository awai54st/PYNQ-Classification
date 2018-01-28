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

///////////////////////////////////////
// Parameters//
#define layer_id 2
#define pool_size 2
#define pool_square pool_size*pool_size
#define In_CH_MAX 32
#define IFMDim_MAX 16
#define pool_mode 1 // 0 for max pooling, 1 for average pooling
#define output_rectify 0
///////////////////////////////////////

#define INF 999999

struct AXI_VAL{
	ap_int<bitwidth> data;
	bool last;
};


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) > (y)) ? (y) : (x))


// II=1 sliding window generator, with common iteration space
//template<unsigned int ConvKernelDim, unsigned int IFMChannels,
//unsigned int IFMDim, unsigned int OFMDim, unsigned int InpWidth=1, unsigned int PadDim=0>
void maxPool_CIF_0_2(hls::stream<AXI_VAL> & in, hls::stream<AXI_VAL> & out) {

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
	static unsigned KerDim_curr = 0;
	static unsigned IFMCH_curr = 0;
	static unsigned IFMDim_curr = 0;
	static unsigned OFMCH_curr = 0;
	static unsigned OFMDim_curr = 0;
	static unsigned PadDim_curr = 0;
	if (status == layer_id) {
		KerDim_curr = ConvKernelDim;
		IFMCH_curr = OFMChannels;
		IFMDim_curr = OFMDim;
		PadDim_curr = PadDim;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////
	// need buffer space for a single maxpooled row of the image
	int buf[In_CH_MAX][IFMDim_MAX / pool_size];
#pragma HLS RESOURCE variable=buf core=RAM_2P_LUTRAM
	int acc[In_CH_MAX];
#pragma HLS RESOURCE variable=acc core=RAM_2P_LUTRAM
	for(unsigned int j = 0; j < In_CH_MAX; j++){
		for(unsigned int i = 0; i < IFMDim_MAX / pool_size; i++) {
#pragma HLS UNROLL
			if (pool_mode == 0) buf[j][i] = -INF;
			else if (pool_mode == 1) buf[j][i] = 0;
		}
	}
	for(unsigned int j = 0; j < In_CH_MAX; j++){
#pragma HLS UNROLL
		if (pool_mode == 0) acc[j] = -INF;
		else if (pool_mode == 1) acc[j] = 0;
	}


	if (status == 0){
		unsigned int pool_out_bound = IFMDim_curr / pool_size;
		for (unsigned int num_img = 0; num_img < batch_size; num_img++){
			for (unsigned int yp = 0; yp < pool_out_bound; yp++) {
				for (unsigned int ky = 0; ky < pool_size; ky++) {
					for (unsigned int xp = 0; xp < pool_out_bound; xp++) {

						for (unsigned int kx = 0; kx < pool_size; kx++) {
							for (unsigned int ch = 0; ch < IFMCH_curr; ch++){
#pragma HLS PIPELINE II=1
								//acc = acc | in.read();
								valIn = in.read();
								if (pool_mode == 0) acc[ch] = MAX(acc[ch],(int)valIn.data);
								else if (pool_mode == 1) acc[ch] = acc[ch] + valIn.data;
							}
						}
						// pool with old value in row buffer
						//buf[xp] |= acc;
						for (unsigned int ch = 0; ch < In_CH_MAX; ch++){
#pragma HLS PIPELINE II=1
							if (pool_mode == 0){
								buf[ch][xp] = MAX(buf[ch][xp], acc[ch]);
								acc[ch] = -INF;
							}
							else if (pool_mode == 1){
								buf[ch][xp] = buf[ch][xp] + acc[ch];
								acc[ch] = 0;
							}
						}
					}
				}

				for (unsigned int outpix = 0; outpix < pool_out_bound; outpix++) {
					for (unsigned int outch = 0; outch < IFMCH_curr; outch++){
#pragma HLS PIPELINE II=1
						if (pool_mode == 0) valOut.data = ((output_rectify)?(MAX(buf[outch][outpix], 0)) : (buf[outch][outpix]));
						else if (pool_mode == 1) valOut.data = ((output_rectify)?(MAX(buf[outch][outpix]/(pool_square), 0)) : (buf[outch][outpix]/(pool_square)));
						if (num_img==batch_size-1 && yp==pool_out_bound-1 && outpix==pool_out_bound-1 && outch==IFMCH_curr-1) valOut.last = 1;
						else valOut.last = 0;
						out.write(valOut);
						// get buffer ready for next use
						if (pool_mode == 0){
							buf[outch][outpix] = -INF;
							acc[outch] = -INF;
						}
						else if (pool_mode == 1){
							buf[outch][outpix] = 0;
							acc[outch] = 0;
						}
					}
				}
			}
		}
	}
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
