import os
import shutil
import sys

dstdir = sys.argv[1]

for i in range(2, len(sys.argv)):
  shutil.copy(sys.argv[i], dstdir)

