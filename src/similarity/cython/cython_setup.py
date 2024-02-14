from setuptools import setup
from Cython.Build import cythonize

setup(
    name='main',
    ext_modules=cythonize("bench.pyx", language_level=3),
    zip_safe=False,
)

# python cython_setup.py build_ext --inplace
# python import main
