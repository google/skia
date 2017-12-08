#!/usr/bin/env python2
# Copyright 2017 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

def get_resources(rdir):
  for root, _, files in os.walk(rdir):
    for filepath in files:
      fullpath = os.path.join(root, filepath)
      if os.path.isfile(fullpath):
        yield os.path.relpath(fullpath, rdir)

def main(resource_dir, array_name, filename):
  with open(filename, 'w') as o:
    o.write('//generated file\n#include "BinaryAsset.h"\n\n');
    names = []
    for n in sorted(get_resources(resource_dir)):
      o.write('static const unsigned char x%d[] = {\n' % len(names))
      with open(os.path.join(resource_dir, n), 'rb')  as f:
        while True:
          buf = f.read(20)
          if len(buf) == 0:
            break
          o.write(''.join('%d,' % ord(x) for x in buf) + '\n')
      o.write('};\n')
      names.append(n)
    o.write('\nBinaryAsset %s[] = {\n' % array_name)
    for i, n in enumerate(names):
      o.write('    {"%s", x%d, sizeof(x%d)},\n' % (n, i, i))
    o.write('    {nullptr, nullptr, 0}\n};\n')

if __name__ == '__main__':
  if len(sys.argv) < 4:
    msg = 'usage:\n  %s SOURCE_DIRECTORY ARRAY_IDENTIFIER OUTPUT_PATH.cpp\n\n'
    sys.stderr.write(msg % sys.argv[0])
    exit(1)
  main(*sys.argv[1:4])

