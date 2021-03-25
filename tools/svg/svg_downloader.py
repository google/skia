#!/usr/bin/env python
# Copyright (c) 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Downloads SVGs into a specified directory."""


from __future__ import print_function
import optparse
import os
import urllib


PARENT_DIR = os.path.dirname(os.path.realpath(__file__))


def download_files(input_file, output_dir, prefix, keep_common_prefix):
  with open(input_file, 'r') as f:
    lines = f.readlines()

    if keep_common_prefix:
      common_prefix = os.path.commonprefix(lines)

    for url in lines:
      file_url = url.strip()

      if keep_common_prefix:
        rel_file = file_url.replace(common_prefix, '')
        dest_dir = os.path.join(output_dir, os.path.dirname(rel_file))
      else:
        dest_dir = output_dir

      dest_file = os.path.join(dest_dir, prefix + os.path.basename(file_url))
      if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)

      print('Downloading %s to %s' % (file_url, dest_file))
      urllib.urlretrieve(file_url, dest_file)


if '__main__' == __name__:
  option_parser = optparse.OptionParser()
  option_parser.add_option(
      '-i', '--input_file',
      help='Path to the text file containing URLs. Each line should contain a '
           'single URL.',
      default=os.path.join(PARENT_DIR, 'svgs.txt'))
  option_parser.add_option(
      '-o', '--output_dir',
      help='The output dir where downloaded SVGs and images will be stored in.')
  option_parser.add_option(
      '-p', '--prefix',
      help='The prefix which downloaded files will begin with.',
      default='')
  option_parser.add_option(
      '-k', '--keep_common_prefix',
      help='Preserve everything in the URL after the common prefix as directory '
           'hierarchy.',
      action='store_true', default=False)
  options, unused_args = option_parser.parse_args()

  if not options.output_dir:
    raise Exception('Must specify --output_dir')

  download_files(options.input_file, options.output_dir,
                 options.prefix, options.keep_common_prefix)
