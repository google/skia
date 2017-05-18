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


def create_asset(local_svgs_dir, target_dir):
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

  # Copy over the SVGs from local_svgs_dir (if any).
  if local_svgs_dir and os.path.exists(local_svgs_dir):
    for svg_filename in os.listdir(local_svgs_dir):
      shutil.copy(src=os.path.join(local_svgs_dir, svg_filename),
                  dst=os.path.join(target_dir, svg_filename))


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--local_svgs_dir', '-l', default='')
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.local_svgs_dir, args.target_dir)


if __name__ == '__main__':
  main()
