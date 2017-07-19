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
  ]
  subprocess.check_call(download_svgs_cmd)

  # Download SVGs from Google storage. These are material design SVGs from
  # skbug.com/5757.
  # There was no easy way to create URLs from them to specify in
  # tools/svg/svgs.txt which is why they had been stored in Google storage.
  subprocess.check_call([
      'gsutil', '-m', 'cp', os.path.join(SVG_GS_BUCKET, 'skbug5757', '*'),
      target_dir
  ])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
