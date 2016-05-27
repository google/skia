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


INFRA_BOTS_DIR = os.path.realpath(os.path.dirname(os.path.abspath(__file__)))
ISOLATE_FILE_NAME = 'android_sdk.isolate'
REPO_SKIA = 'https://skia.googlesource.com/skia.git'
SDK_DIR_NAME = 'android-sdk'


def get_isolate_binary():
  """Find or, if necessary, obtain the isolate binary."""
  # Try to find isolate locally.
  platform = 'linux64'
  if sys.platform == 'win32':
    platform = 'win64'
  elif sys.platform == 'darwin':
    platform = 'mac64'
  repo_isolate = os.path.join(INFRA_BOTS_DIR,
                              'tools', 'luci-go', platform)
  path = os.pathsep.join((repo_isolate, os.environ['PATH']))
  try:
    output = subprocess.check_output(
        ['which', 'isolate'],
        env={'PATH':path}).rstrip()
    print 'Found isolate binary: %s' % output
    return output
  except subprocess.CalledProcessError:
    pass

  # Download isolate from GS.
  print 'Unable to find isolate binary; attempting to download...'
  try:
    subprocess.check_call(
        ['download_from_google_storage',
         '--bucket', 'chromium-luci',
         '-d', repo_isolate])
  except OSError as e:
    raise Exception('Failed to download isolate binary. '
                    'Is depot_tools in PATH?  Error: %s' % e)
  except subprocess.CalledProcessError as e:
    raise Exception('Failed to download isolate binary. '
                    'Are you authenticated to Google Storage?  Error: %s' % e)

  output = subprocess.check_output(
      ['which', 'isolate'],
      env={'PATH':path}).rstrip()
  return output


def check_isolate_auth(isolate):
  """Ensure that we're authenticated to the isolate server."""
  not_logged_in = 'Not logged in'
  try:
    output = subprocess.check_output([isolate, 'whoami'])
    
  except subprocess.CalledProcessError:
    output = not_logged_in
  if output == not_logged_in:
    raise Exception('Not authenticated to isolate server. You probably need to '
                    'run:\n$ %s login' % isolate)


def isolate_android_sdk(android_sdk_root):
  """Isolate the Android SDK and return the isolated hash."""
  repo_isolate_file = os.path.join(INFRA_BOTS_DIR, ISOLATE_FILE_NAME)
  with utils.tmp_dir():
    # Copy the SDK dir contents into a directory with a known name.
    sdk_dir = os.path.join(os.getcwd(), SDK_DIR_NAME)
    shutil.copytree(android_sdk_root, sdk_dir)
    isolate_file = os.path.join(os.getcwd(), ISOLATE_FILE_NAME)
    shutil.copyfile(repo_isolate_file, isolate_file)

    # Isolate the SDK.
    isolate = get_isolate_binary()
    check_isolate_auth(isolate)
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
  skia_path = os.path.abspath(os.path.join(INFRA_BOTS_DIR,
                                           os.pardir, os.pardir))

  with utils.print_timings():
    isolated_hash = isolate_android_sdk(args.android_sdk_root)
    update_sdk_file(skia_path, isolated_hash)


if __name__ == '__main__':
  main()
