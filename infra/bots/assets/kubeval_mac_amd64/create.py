#!/usr/bin/env python3
#
# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import os
import subprocess
import tempfile

URL = 'https://github.com/instrumenta/kubeval/releases/download/v0.16.1/kubeval-darwin-amd64.tar.gz'
SHA256 = 'c79a91f2e6638463881a8189e0628ebd583a5e2912e6f411897e3cea567125e7'

BINARY = URL.split('/')[-1]


def create_asset(target_dir):
  """Create the asset."""
  tmp = tempfile.mkdtemp()
  target_file = os.path.join(tmp, 'kubeval.tar.gz')
  subprocess.call(['wget', '--quiet', '--output-document', target_file, URL])
  output = subprocess.check_output(['sha256sum', target_file], encoding='utf-8')
  actual_hash = output.split(' ')[0]
  if actual_hash != SHA256:
    raise Exception('SHA256 does not match (%s != %s)' % (actual_hash, SHA256))
  subprocess.check_call(
      ['tar', 'xf', os.path.join(tmp, 'kubeval.tar.gz'), 'kubeval'],
      cwd=target_dir)
  subprocess.check_call(['rm', '-rf', tmp])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()

