#!/usr/bin/python

# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Copy a file.
"""

import argparse
import os
import shutil

if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument('src', help='File to copy.')
  parser.add_argument('dst', help='Location to copy to.')
  args = parser.parse_args()

  src = os.path.abspath(os.path.join(os.getcwd(), args.src))
  dst = os.path.abspath(os.path.join(os.getcwd(), args.dst))

  print 'Copying from %s to %s' % (src, dst)

  src_dir = os.path.dirname(src)
  if not os.path.exists(src_dir):
    raise AssertionError('src directory %s does not exist!' % src_dir)

  if not os.path.exists(src):
    raise AssertionError('file to copy %s does not exist' % src)

  dst_dir = os.path.dirname(dst)
  if not os.path.exists(dst_dir):
    print 'dst directory %s does not exist! creating it!' % dst_dir
    os.makedirs(dst_dir)

  shutil.copyfile(src, dst)
