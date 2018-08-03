#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import os
import subprocess


def create_asset(target_dir):
  """Create the asset."""
  env = {}
  env.update(os.environ)
  env['GOPATH'] = target_dir
  subprocess.check_call(
      ['go', 'get', '-u', '-t', 'go.skia.org/infra/...'],
      env=env)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
