#!/usr/bin/env python

# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import os
import subprocess
import sys

sys.path.insert(0, os.path.join('common', 'py', 'utils'))
import gs_utils


def get_uploaded_hashes(gs_bucket, gs_dir):
  _, files = gs_utils.GSUtils().list_bucket_contents(gs_bucket, subdir=gs_dir)
  hashes = []
  for line in files:
    hashes.append(os.path.basename(line).split('.')[0])
  return hashes


def main(gs_bucket, gs_dir, output_file):
  hashes = get_uploaded_hashes(gs_bucket, gs_dir)
  with open(output_file, 'w') as f:
    for h in hashes:
      f.write(h + '\n')


if __name__ == '__main__':
  if len(sys.argv) != 4:
    print >> sys.stderr , ('Usage: %s <gs_bucket> <gs_subdir> <output_file>' %
                           sys.argv[0])
    sys.exit(1)
  main(*sys.argv[1:])
