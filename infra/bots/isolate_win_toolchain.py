#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import argparse
import json
import os
import shlex
import shutil
import subprocess
import sys
import utils


REPO_CHROME = 'https://chromium.googlesource.com/chromium/src.git'
REPO_SKIA = 'https://skia.googlesource.com/skia.git'


def get_toolchain_dir(toolchain_dir_output):
  """Find the toolchain directory."""
  prefix = 'vs_path = '
  for line in toolchain_dir_output.splitlines():
    if line.startswith(prefix):
      return line[len(prefix):].strip('"')
  raise Exception('Unable to find toolchain dir in output:\n%s' % (
                  toolchain_dir_output))


def gen_toolchain(chrome_path, msvs_version, isolate_file):
  """Update the VS toolchain, isolate it, and return the isolated hash."""
  with utils.chdir(chrome_path):
    subprocess.check_call([utils.GCLIENT, 'sync'])
    with utils.git_branch():
      vs_toolchain_py = os.path.join('build', 'vs_toolchain.py')
      env = os.environ.copy()
      env['GYP_MSVS_VERSION'] = msvs_version
      subprocess.check_call(['python', vs_toolchain_py, 'update'], env=env)
      output = subprocess.check_output(['python', vs_toolchain_py,
                                        'get_toolchain_dir'], env=env).rstrip()
      src_dir = get_toolchain_dir(output)

    # Isolate the toolchain. Assumes we're running on Windows, since the above
    # would fail otherwise.
    rel_path = os.path.relpath(src_dir, os.path.dirname(isolate_file))
    isolate = os.path.join(
        os.curdir, 'tools', 'luci-go', 'win64', 'isolate.exe')
    isolate_cmd = [isolate, 'archive', '--quiet',
        '--isolate-server', 'https://isolateserver.appspot.com',
        '-i', isolate_file,
        '-s', 'win_toolchain_%s.isolated' % msvs_version,
        '--extra-variable', 'WIN_TOOLCHAIN_DIR=%s' % rel_path]
    isolate_out = subprocess.check_output(isolate_cmd).rstrip()
    return shlex.split(isolate_out)[0]


def update_toolchain_file(skia_path, msvs_version, isolated_hash):
  """Edit the win_toolchain_hash file, upload a CL."""
  with utils.chdir(skia_path):
    with utils.git_branch():
      hash_file = os.path.join('infra', 'bots', 'win_toolchain_hash.json')
      with open(hash_file) as f:
        hashes = json.load(f)
      hashes[msvs_version] = isolated_hash
      with open(hash_file, 'w') as f:
        json.dump(hashes, f, indent=4, sort_keys=True)
      subprocess.check_call([utils.GIT, 'add', hash_file])
      subprocess.check_call([utils.GIT, 'commit', '-m', 'Update Win toolchain'])
      subprocess.check_call([utils.GIT, 'cl', 'upload', '--bypass-hooks'])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--msvs_version', required=True)
  parser.add_argument('--chrome_path')
  parser.add_argument('--skia_path')
  args = parser.parse_args()

  isolate_file = os.path.join(os.path.dirname(os.path.realpath(__file__)),
                              'win_toolchain.isolate')

  with utils.print_timings():
    with utils.tmp_dir() as tmp_dir:
      chrome_path = args.chrome_path
      if not chrome_path:
        print ('Syncing Chrome from scratch. If you already have a checkout, '
               'specify --chrome_path to save time.')
        chrome_path = os.path.join(tmp_dir.name, 'src')
      if not os.path.isdir(chrome_path):
        utils.git_clone(REPO_CHROME, chrome_path)

      skia_path = args.skia_path
      if not skia_path:
        print ('Syncing Skia from scratch. If you already have a checkout, '
               'specify --chrome_path to save time.')
        skia_path = os.path.join(tmp_dir.name, 'skia')
      if not os.path.isdir(skia_path):
        utils.git_clone(REPO_SKIA, skia_path)

      isolated_hash = gen_toolchain(chrome_path, args.msvs_version,
                                    isolate_file)
      update_toolchain_file(skia_path, args.msvs_version, isolated_hash)


if __name__ == '__main__':
  main()
