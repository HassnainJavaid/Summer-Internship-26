from setuptools import setup, Extension
from Cython.Build import cythonize
import os

ext = Extension(
    name="tokenizer_wrapper",
    sources=["src/python/tokenizer_wrapper.pyx"],
    include_dirs=["include"],
    libraries=["tokenizer"],
    library_dirs=["build/lib"],
    extra_compile_args=["-O3"],
    runtime_library_dirs=["$ORIGIN/../lib"]
)

setup(
    ext_modules=cythonize([ext], compiler_directives={'language_level': "3"})
)