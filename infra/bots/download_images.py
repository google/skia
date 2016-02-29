#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import common
import os
import sys


def main():
  if len(sys.argv) != 1:
    print >> sys.stderr, 'Usage: download_images.py'
    sys.exit(1)
  skia_dir = os.path.abspath(os.path.join(
      os.path.dirname(os.path.realpath(__file__)),
      os.pardir, os.pardir))
  dst_dir = os.path.join(skia_dir, os.pardir, 'images')
  tmp_dir = os.path.join(skia_dir, os.pardir, 'tmp')
  common.download_dir(skia_dir, tmp_dir, common.VERSION_FILE_SK_IMAGE,
                      common.GS_SUBDIR_TMPL_SK_IMAGE, dst_dir)


if __name__ == '__main__':
  main()
