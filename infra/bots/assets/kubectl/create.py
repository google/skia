#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import requests
import subprocess


VERSION = 'v1.25.4'
DOWNLOAD_URL = 'https://dl.k8s.io/release/%s/bin/linux/amd64/kubectl' % VERSION
SHA256_URL = 'https://dl.k8s.io/%s/bin/linux/amd64/kubectl.sha256' % VERSION


def create_asset(target_dir):
  """Create the asset."""
  subprocess.check_call(['curl', '-LO', DOWNLOAD_URL], cwd=target_dir)
  sha256 = requests.get(SHA256_URL).text
  p = subprocess.Popen(
      ['sha256sum', '--check'],
      cwd=target_dir,
      stdout=subprocess.PIPE,
      stderr=subprocess.STDOUT,
      stdin=subprocess.PIPE)
  output = p.communicate(
      input=(sha256 + ' kubectl').encode('utf-8'))[0].decode('utf-8')
  if 'OK' not in output:
    raise ValueError('Got wrong checksum for kubectl: %s' % output)
  subprocess.check_call(['chmod', 'a+x', 'kubectl'], cwd=target_dir)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()

