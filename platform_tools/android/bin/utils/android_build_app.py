#!/usr/bin/python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
import shutil
import subprocess
import sys

parser = argparse.ArgumentParser(description='builds skia android apps')
parser.add_argument('-o', '--output_dir', help='dir to install the package')
parser.add_argument('-t', '--target_cpu', help='target_cpu of the app')
parser.add_argument('app_name')

args = parser.parse_args()

android_variant = ""
android_buildtype = "debug"

if args.target_cpu == "arm":
  android_variant = "arm"
elif args.target_cpu == "arm64":
  android_variant = "arm64"
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

# build the apk using gradle
try:
    subprocess.check_call(['./apps/gradlew',
      ':viewer:assemble' + android_variant + android_buildtype,
      '-papps/' + args.app_name,
      '-PsuppressNativeBuild',
      '--daemon'], cwd=os.path.join(os.path.dirname(__file__), "../.."))
except subprocess.CalledProcessError as error:
  print error
  sys.exit("gradle build failed")

# copy apk back into the main out directory
current_dir = os.path.dirname(__file__)
apk_src = os.path.join(current_dir, "..", "..", "apps", args.app_name, "build",
                       "outputs", "apk", args.app_name + android_variant +
                       android_buildtype + ".apk")
apk_dst = os.path.join(current_dir, args.output_dir, args.app_name + ".apk")
shutil.copyfile(apk_src, apk_dst)
