import cython
cimport numpy as np
import numpy as np
ctypedef fused DTYPE_t:
    np.float32_t
    np.float64_t

def im2col_lasagne_cython(np.ndarray[DTYPE_t, ndim=4] x, int field_height,
                  int field_width, int padding, int stride):

    cdef int p = padding
    cdef np.ndarray[DTYPE_t, ndim=4] x_padded = np.pad(x,
            ((0, 0), (0, 0), (p, p), (p, p)), mode='constant')
    cdef int N = x_padded.shape[0]
    cdef int C = x_padded.shape[1]
    cdef int H = x_padded.shape[2]
    cdef int W = x_padded.shape[3]

    cdef int HH = (H - field_height) / stride + 1
    cdef int WW = (W - field_width) / stride + 1

    cdef np.ndarray[DTYPE_t, ndim=2] cols = np.zeros(
            (C * field_height * field_width, N * HH * WW),
            dtype=x.dtype)

    # Moving the inner loop to a C function with no bounds checking works, but does
    # not seem to help performance in any measurable way.

    cdef int c, ii, jj, row, yy, xx, i, col

    for c in range(C):
        for yy in range(HH):
            for xx in range(WW):
                for ii in range(field_height):
                    for jj in range(field_width):
                        row = c * field_width * field_height + ii * field_height + jj
                        for i in range(N):
                            col = yy * WW * N + xx * N + i
                            cols[row, col] = x_padded[i, c, stride * yy + ii, stride * xx + jj]
    return cols
