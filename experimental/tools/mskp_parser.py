#!/usr/bin/env python

# Copyright 2016 Google Inc.
# 
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Experimental Skia Multi-Picture Doc parser.

from __future__ import print_function

import fileinput
import sys
import struct

if len(sys.argv) < 2:
  sys.stderr.write('Usage:\n\tpython %s MSKP_FILE [OUTPUT_SKP]\n\n'
                   % sys.argv[0])
  exit(1)

mskp_src = sys.argv[1]
src = open(mskp_src, 'rb')

magic_constant = b'Skia Multi-Picture Doc\n\n'
magic = src.read(len(magic_constant))
if magic != magic_constant:
  sys.stderr.write('Not a mskp file: "%s"\n' % mskp_src)
  exit(2)

version, page_count = struct.unpack('II', src.read(8))[:2]
print('MSKP version: ', version)
print('page count: ', page_count)
if version > 2 or version < 1:
  #TODO(halcanary): Remove support for version 1.
  sys.stderr.write('unsupported mskp version\n')
  exit(3)
offsets = []
for page in range(page_count):
  print('page %3d\t' % page, end='')
  if version == 1:
    offset, size_x, size_y =struct.unpack('Qff', src.read(16))
    print('offset = %-7d\t' % offset, end='')
    offsets.append(offset)
  elif version == 2:
    size_x, size_y =struct.unpack('ff', src.read(8))
  print('size = (%r,%r)' % (size_x, size_y))

if len(sys.argv) >= 3:
  with open(sys.argv[2], 'wb') as o:
    if version == 2 or len(offsets) < 2:
      while True:
        file_buffer = src.read(8192)
        if 0 == len(file_buffer):
          break
        o.write(file_buffer)
    else:
      o.write(src.read(offsets[1] - offsets[0]))
