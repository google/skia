#!/usr/bin/env python3
#
# Copyright 2022 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import os
import subprocess


URL = 'https://github.com/bazelbuild/bazelisk/releases/download/v1.14.0/bazelisk-windows-amd64.exe'
SHA256 = '89fdae9b4e05004ab52a7296583c7da5028d76c1b185e93f3309ecf5cf965ba9'

BINARY = URL.split('/')[-1]


def create_asset(target_dir):
  """Create the asset."""
  target_file = os.path.join(target_dir, 'bazelisk.exe')
  subprocess.call(['wget', '--quiet', '--output-document', target_file, URL])
  output = subprocess.check_output(['sha256sum', target_file], encoding='utf-8')
  actual_hash = output.split(' ')[0]
  if actual_hash != SHA256:
    raise Exception('SHA256 does not match (%s != %s)' % (actual_hash, SHA256))
  subprocess.call(['chmod', 'ugo+x', target_file])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()

