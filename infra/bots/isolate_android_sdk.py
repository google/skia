#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Isolate a locally-managed Android SDK."""


import argparse
import os
import shlex
import shutil
import subprocess
import sys
import utils


ISOLATE_FILE_NAME = 'android_sdk.isolate'
REPO_SKIA = 'https://skia.googlesource.com/skia.git'
SDK_DIR_NAME = 'android-sdk'


def isolate_android_sdk(android_sdk_root):
  """Isolate the Android SDK and return the isolated hash."""
  repo_isolate_file = os.path.join(os.path.dirname(os.path.realpath(__file__)),
                                   ISOLATE_FILE_NAME)
  with utils.tmp_dir():
    # Copy the SDK dir contents into a directory with a known name.
    sdk_dir = os.path.join(os.getcwd(), SDK_DIR_NAME)
    shutil.copytree(android_sdk_root, sdk_dir)
    isolate_file = os.path.join(os.getcwd(), ISOLATE_FILE_NAME)
    shutil.copyfile(repo_isolate_file, isolate_file)

    # Isolate the SDK.
    isolate = 'isolate'  # TODO(borenet): Don't assume this is in PATH.
    android_sdk_relpath = os.path.relpath(
        sdk_dir, os.path.dirname(isolate_file))
    isolate_cmd = [isolate, 'archive', '--quiet',
        '--isolate-server', 'https://isolateserver.appspot.com',
        '-i', isolate_file,
        '-s', 'android_sdk.isolated',
        '--extra-variable', 'ANDROID_SDK_DIR=%s' % android_sdk_relpath]
    isolate_out = subprocess.check_output(isolate_cmd).rstrip()
    return shlex.split(isolate_out)[0]



def update_sdk_file(skia_path, isolated_hash):
  """Edit the android_sdk_hash file, upload a CL."""
  with utils.chdir(skia_path):
    with utils.git_branch():
      hash_file = os.path.join('infra', 'bots', 'android_sdk_hash')
      with open(hash_file, 'w') as f:
        f.write(isolated_hash)
      subprocess.check_call([utils.GIT, 'add', hash_file])
      subprocess.check_call([utils.GIT, 'commit', '-m', 'Update Android SDK'])
      subprocess.check_call([utils.GIT, 'cl', 'upload', '--bypass-hooks'])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--android_sdk_root', required=True)
  args = parser.parse_args()
  skia_path = os.path.join(os.path.dirname(os.path.realpath(__file__)),
                           os.pardir, os.pardir)

  with utils.print_timings():
    isolated_hash = isolate_android_sdk(args.android_sdk_root)
    update_sdk_file(skia_path, isolated_hash)


if __name__ == '__main__':
  main()
