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


URL = 'https://github.com/bazelbuild/bazelisk/releases/download/v1.27.0/bazelisk-windows-amd64.exe'
SHA256 = 'd4b5e1cea61fcdb0bed60f8868c2e37684221b65feae898d1124482cd39ec89e'

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

