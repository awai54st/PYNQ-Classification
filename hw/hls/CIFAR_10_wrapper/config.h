#include <hls_stream.h>
#include <ap_axi_sdata.h>

#define bitwidth 32
#define InpWidth 16
#define quant_scale 32768// 2^(wordwidth-1)

#define INF 999999

#ifndef AXI_VAL_DEF
typedef ap_int<bitwidth> AXI_VAL;
struct AXI_DMA_IO{
	ap_int<bitwidth> data;
	bool last;
};
#define AXI_VAL_DEF
#endif


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) > (y)) ? (y) : (x))
