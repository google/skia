#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import os
import urllib2


GOMA_CTL_URL = (
    'https://clients5.google.com/cxx-compiler-service/download/goma_ctl.py')

def create_asset(target_dir):
  """Create the asset."""
  if not os.path.isdir(target_dir):
    os.makedirs(target_dir)
  filename = os.path.join(target_dir, 'goma_ctl.py')
  with open(filename, 'wb') as f:
    f.write(urllib2.urlopen(GOMA_CTL_URL).read())


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
