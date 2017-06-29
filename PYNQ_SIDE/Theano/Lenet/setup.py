from distutils.core import setup, Extension
import numpy
from Cython.Distutils import build_ext

setup(
    cmdclass={'build_ext': build_ext},
    ext_modules=[Extension("im2col_lasagne_cython",
                 sources=["_im2col_lasagne_cython.pyx"],
                 include_dirs=[numpy.get_include()])],
)