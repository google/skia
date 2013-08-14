import os
import shutil
import sys

def copyfile(src, dst):
  fsrc = None
  fdst = None
  try:
    fsrc = open(src, 'rb')
    fdst = open(dst, 'wb')
    shutil.copyfileobj(fsrc, fdst)
  finally:
    if fdst:
      fdst.close()
    if fsrc:
      fsrc.close()

dstdir = sys.argv[1]

if not os.path.exists(dstdir):
  os.makedirs(dstdir)

for i in range(2, len(sys.argv)):
  copyfile(sys.argv[i], os.path.join(dstdir, os.path.basename(sys.argv[i])))

