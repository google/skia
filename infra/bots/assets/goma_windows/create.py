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
import tempfile
import urllib2

GOMA_DOWNLOADURL_URL = (
    "https://clients5.google.com/cxx-compiler-service/download/downloadurl")
GOMA_FILENAME = "goma-win64.zip"

def create_asset(target_dir):
  """Create the asset."""
  if not os.path.isdir(target_dir):
    os.makedirs(target_dir)
  current_ver_url = urllib2.urlopen(GOMA_DOWNLOADURL_URL).read().strip()
  goma_url = current_ver_url + "/" + GOMA_FILENAME
  temp_filename = tempfile.gettempdir() + "/" + GOMA_FILENAME
  with open(temp_filename, 'wb') as f:
    f.write(urllib2.urlopen(goma_url).read())
  subprocess.check_call(["unzip", "-d", target_dir, temp_filename])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
