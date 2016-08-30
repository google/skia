#!/usr/bin/env python
# Copyright (c) 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Downloads SVGs into a specified directory."""


import optparse
import os
import sys
import urllib


PARENT_DIR = os.path.dirname(os.path.realpath(__file__))


def downloadSVGs(svgs_file, output_dir):
  with open(svgs_file, 'r') as f:
    for url in f.xreadlines():
      svg_url = url.strip()
      dest_file = os.path.join(output_dir, os.path.basename(svg_url))
      print 'Downloading %s' % svg_url
      urllib.urlretrieve(svg_url, dest_file)


if '__main__' == __name__:
  option_parser = optparse.OptionParser()
  option_parser.add_option(
      '-s', '--svgs_file',
      help='Path to the text file containing SVGs. Each line should contain a '
           'single URL.',
      default=os.path.join(PARENT_DIR, 'svgs.txt'))
  option_parser.add_option(
      '-o', '--output_dir',
      help='The output dir where downloaded SVGs will be stored in.')
  options, unused_args = option_parser.parse_args()

  if not options.output_dir:
    raise Exception('Must specify --output_dir')
  sys.exit(downloadSVGs(options.svgs_file, options.output_dir))
