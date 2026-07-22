from setuptools import setup, Extension
from Cython.Build import cythonize
import os

ext = Extension(
    name="tokenizer_wrapper",
    sources=["src/tokenizer_wrapper.pyx"],
    include_dirs=["include"],
    libraries=["tokenizer"],
    library_dirs=["."],  # Directs linker to find libtokenizer.so / dll in root
    extra_compile_args=["-O3"]
)

setup(
    ext_modules=cythonize([ext], compiler_directives={'language_level': "3"})
)