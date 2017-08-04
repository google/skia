#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import common
import subprocess
import os
import shutil


SVG_TOOLS = os.path.join(common.INFRA_BOTS_DIR, os.pardir, os.pardir, 'tools',
                         'svg')
SVG_GS_BUCKET = 'gs://skia-svgs'


def create_asset(target_dir):
  """Create the asset."""
  target_dir = os.path.realpath(target_dir)

  if not os.path.exists(target_dir):
    os.makedirs(target_dir)

  # Download the SVGs specified in tools/svg/svgs.txt
  download_svgs_cmd = [
    'python', os.path.join(SVG_TOOLS, 'svg_downloader.py'),
    '--output_dir', target_dir,
    '--svgs_file', os.path.join(SVG_TOOLS, 'svgs.txt'),
  ]
  subprocess.check_call(download_svgs_cmd)

  # Download the SVGs specified in tools/svg/svgs_parse_only.txt with a prefix.
  download_svgs_parse_only_cmd = [
    'python', os.path.join(SVG_TOOLS, 'svg_downloader.py'),
    '--output_dir', target_dir,
    '--svgs_file', os.path.join(SVG_TOOLS, 'svgs_parse_only.txt'),
    '--prefix', 'svgparse_',
  ]
  subprocess.check_call(download_svgs_parse_only_cmd)

  # Download SVGs from Google storage.
  # The Google storage bucket will either contain private SVGs or SVGs which we
  # cannot download over the internet using svg_downloader.py.
  for skbug in ['skbug4713', 'skbug6918']:
    subprocess.check_call([
        'gsutil', '-m', 'cp', os.path.join(SVG_GS_BUCKET, skbug, '*'),
        target_dir
    ])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
