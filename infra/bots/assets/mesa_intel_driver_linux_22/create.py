#!/usr/bin/env python
#
# Copyright 2022 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the mesa driver."""


import argparse
import subprocess

MESA_VERSION = '22.1.3'

def create_asset(target_dir):
  """Create the asset."""
  cmd = [
    'docker', 'build', '-t', 'mesa-driver-builder:latest',
    './mesa_intel_driver_linux_22/mesa-driver-builder',
  ]
  print('Building container', cmd)
  subprocess.check_output(cmd)

  cmd = [
    'docker', 'run', '--volume', '%s:/OUT' % target_dir,
    '--env', 'MESA_VERSION=%s' % MESA_VERSION,
    'mesa-driver-builder'
  ]
  print('Running container', cmd)
  subprocess.check_output(cmd)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
