#include <assert.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>

#include "config.h"

#include "../hw_library/axi_dma_master.h"
#include "../hw_library/axi_dma_slave.h"
#include "../hw_library/stream_convolution_slideWindow.h"
#include "../hw_library/fixed_point_stream_convolution.h"
#include "../hw_library/pool.h"
#include "../hw_library/fully_connected.h"
