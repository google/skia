#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Download an updated VS toolchain"""


import argparse
import common
import json
import os
import shlex
import shutil
import subprocess
import sys
import utils

import win_toolchain_utils


# By default the toolchain includes a bunch of unnecessary stuff with long path
# names. Trim out directories with these names.
IGNORE_LIST = [
  'WindowsMobile',
  'App Certification Kit',
  'Debuggers',
  'Extension SDKs',
  'winrt',
  'DesignTime',
  'AccChecker',
]

REPO_CHROME = 'https://chromium.googlesource.com/chromium/src.git'


def filter_toolchain_files(dirname, files):
  """Callback for shutil.copytree. Return lists of files to skip."""
  split = dirname.split(os.path.sep)
  for ign in IGNORE_LIST:
    if ign in split:
       print 'Ignoring dir %s' % dirname
       return files
  return []


def get_toolchain_dir(toolchain_dir_output):
  """Find the toolchain directory."""
  prefix = 'vs_path = '
  for line in toolchain_dir_output.splitlines():
    if line.startswith(prefix):
      return line[len(prefix):].strip('"')
  raise Exception('Unable to find toolchain dir in output:\n%s' % (
                  toolchain_dir_output))


def gen_toolchain(chrome_path, msvs_version, target_dir):
  """Update the VS toolchain and copy it to the target_dir."""
  with utils.chdir(os.path.join(chrome_path, 'src')):
    subprocess.check_call([utils.GCLIENT, 'sync'])
    depot_tools = subprocess.check_output([
        'python', os.path.join('build', 'find_depot_tools.py')]).rstrip()
    with utils.git_branch():
      vs_toolchain_py = os.path.join('build', 'vs_toolchain.py')
      env = os.environ.copy()
      env['GYP_MSVS_VERSION'] = msvs_version
      subprocess.check_call(['python', vs_toolchain_py, 'update'], env=env)
      output = subprocess.check_output(['python', vs_toolchain_py,
                                        'get_toolchain_dir'], env=env).rstrip()
      src_dir = get_toolchain_dir(output)
      # Mock out absolute paths in win_toolchain.json.
      win_toolchain_utils.abstract(os.path.join('build', 'win_toolchain.json'),
                                   os.path.dirname(depot_tools))

      # Copy the toolchain files to the target_dir.
      build = os.path.join(os.getcwd(), 'build')
      dst_build = os.path.join(target_dir, 'src', 'build')
      os.makedirs(dst_build)
      for f in ('find_depot_tools.py', 'vs_toolchain.py', 'win_toolchain.json'):
        shutil.copyfile(os.path.join(build, f), os.path.join(dst_build, f))

      shutil.copytree(os.path.join(os.getcwd(), 'tools', 'gyp', 'pylib'),
                      os.path.join(target_dir, 'src', 'tools', 'gyp', 'pylib'))

      dst_depot_tools = os.path.join(target_dir, 'depot_tools')
      os.makedirs(dst_depot_tools)
      for f in ('gclient.py', 'breakpad.py'):
        shutil.copyfile(os.path.join(depot_tools, f),
                        os.path.join(dst_depot_tools, f))
      toolchain_dst = os.path.join(
          target_dir, 'depot_tools', os.path.relpath(src_dir, depot_tools))
      shutil.copytree(src_dir, toolchain_dst, ignore=filter_toolchain_files)


def create_asset(target_dir, msvs_version, chrome_path=None):
  """Create the asset."""
  if not os.path.isdir(target_dir):
    os.makedirs(target_dir)
  with utils.tmp_dir() as tmp_dir:
    if not chrome_path:
      print ('Syncing Chrome from scratch. If you already have a checkout, '
             'specify --chrome_path to save time.')
      chrome_path = os.path.join(tmp_dir.name, 'src')
    if not os.path.isdir(chrome_path):
      subprocess.check_call([utils.GCLIENT, 'config', REPO_CHROME, '--managed'])
      subprocess.check_call([utils.GCLIENT, 'sync'])

    gen_toolchain(chrome_path, msvs_version, target_dir)

def main():
  if sys.platform != 'win32':
    print >> sys.stderr, 'This script only runs on Windows.'
    sys.exit(1)

  parser = argparse.ArgumentParser()
  parser.add_argument('--msvs_version', required=True)
  parser.add_argument('--chrome_path')
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  target_dir = os.path.abspath(args.target_dir)
  create_asset(target_dir, args.msvs_version, args.chrome_path)


if __name__ == '__main__':
  main()
