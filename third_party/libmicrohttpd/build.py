# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# this script will configure and build microhttpd in a temp directory and then
# copy the static library generated to a destination folder
import argparse
import os
from subprocess import call
import shutil
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument("--src", help="microhttpd src directory")
parser.add_argument("--out", help="build directory")
parser.add_argument("--dst", help="output for final build products")
args = parser.parse_args()

out_dir = args.out
cwd = os.getcwd()
try:
  os.makedirs(out_dir)
except OSError as e:
  pass

os.chdir(out_dir)
call([os.path.join(cwd, args.src, "configure"),
      "--disable-doc",
      "--disable-examples",
      "--enable-https=no",
      "--disable-curl",
      "--enable-spdy=no",
      "--enable-shared=no"])
call(["make", "--silent"])
call(["cp",
      "src/microhttpd/.libs/libmicrohttpd.a",
      os.path.join(cwd, args.dst)])
