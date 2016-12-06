#!/usr/bin/python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
import shutil
import sys

parser = argparse.ArgumentParser(description='copy shared libs')
parser.add_argument('-d', '--src_dir', help='dir that contains the lib')
parser.add_argument('-t', '--target_cpu', help='target cpu of the lib')
parser.add_argument('app_name')

args = parser.parse_args()

android_variant = ""

if args.target_cpu == "arm":
  android_variant = "armeabi-v7a"
elif args.target_cpu == "arm64":
  android_variant = "arm64-v8a"
elif args.target_cpu == "x86":
  android_variant = "x86"
elif args.target_cpu == "x64":
  android_variant = "x86_64"
elif args.target_cpu == "mipsel":
  android_variant = "mips"
elif args.target_cpu == "mips64el":
  android_variant = "mips64"
else:
  sys.exit("unknown target_cpu")

current_dir = os.path.dirname(__file__)
src_dir = os.path.join(current_dir, args.src_dir)
dst_dir = os.path.join(current_dir, "..", "platform_tools", "android", "apps",
                       args.app_name, "src", "main", "libs", android_variant)
lib_name = "lib" + args.app_name + ".so"

# clean out the dst directory
if os.path.exists(dst_dir):
  shutil.rmtree(dst_dir)
os.mkdir(dst_dir)

# copy the file
shutil.copyfile(os.path.join(src_dir, lib_name),
                os.path.join(dst_dir, lib_name))

# write tmp file so that GN has a known output to look for
tmp_file = os.path.join(src_dir, ".lastbuild_lib" + args.app_name)
with open(tmp_file, 'a'):
  os.utime(tmp_file, None)
