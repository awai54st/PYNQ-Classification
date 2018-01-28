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





// II=1 sliding window generator, with common iteration space
//template<unsigned int ConvKernelDim, unsigned int IFMChannels,
//unsigned int IFMDim, unsigned int OFMDim, unsigned int InpWidth=1, unsigned int PadDim=0>

template<
	unsigned pool_size,
	unsigned In_CH_MAX,
	unsigned IFMDim_MAX
>
void pool (
	hls::stream<AXI_VAL> & in, 
	hls::stream<AXI_VAL> & out,
	const unsigned layer_id,
	const unsigned pool_mode, // 0 for max pooling, 1 for average pooling
	const bool output_rectify
) {

	const unsigned pool_square = pool_size * pool_size;
	AXI_VAL valIn, valOut;

	// first two data as row_size and col_size
	valIn = in.read();
	unsigned status = (unsigned)valIn;
	out.write(valIn);

	valIn = in.read();
	unsigned batch_size = (unsigned)valIn;
	out.write(valIn);

	valIn = in.read();
	unsigned ConvKernelDim = (unsigned)valIn;
	out.write(valIn);

	valIn = in.read();
	unsigned IFMChannels = (unsigned)valIn;
	out.write(valIn);

	valIn = in.read();
	unsigned IFMDim = (unsigned)valIn;
	out.write(valIn);

	valIn = in.read();
	unsigned OFMChannels = (unsigned)valIn;
	out.write(valIn);

	valIn = in.read();
	unsigned OFMDim = (unsigned)valIn;
	out.write(valIn);

	valIn = in.read();
	unsigned PadDim = (unsigned)valIn;
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
								valIn = in.read();
								if (pool_mode == 0) acc[ch] = MAX(acc[ch],(int)valIn);
								else if (pool_mode == 1) acc[ch] = acc[ch] + valIn;
							}
						}
						// pool with old value in row buffer
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
						if (pool_mode == 0) valOut = ((output_rectify)?(MAX(buf[outch][outpix], 0)) : (buf[outch][outpix]));
						else if (pool_mode == 1) valOut = ((output_rectify)?(MAX(buf[outch][outpix]/(pool_square), 0)) : (buf[outch][outpix]/(pool_square)));
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
