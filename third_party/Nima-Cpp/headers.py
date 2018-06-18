#!/usr/bin/python2

import os.path
import shutil

DIR = os.path.dirname(os.path.realpath(__file__))

NIMA_CPP = DIR + '/../externals/Nima-Cpp'
NIMA_MATH_CPP = DIR + '/../externals/Nima-Cpp/Nima-Math-Cpp'

# Remove the existing directories.
shutil.rmtree(NIMA_CPP + '/nima', ignore_errors = True)
shutil.rmtree(NIMA_MATH_CPP + '/nima', ignore_errors = True)

# Copy files.
shutil.copytree(NIMA_CPP + '/Source', NIMA_CPP + '/nima')
shutil.copytree(NIMA_MATH_CPP + '/Source', NIMA_MATH_CPP + '/nima')
