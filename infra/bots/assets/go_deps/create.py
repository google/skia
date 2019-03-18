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


def create_asset(target_dir):
  """Create the asset."""
  print 'Syncing Go checkouts...'
  env = {}
  env.update(os.environ)
  env['GOPATH'] = target_dir
  subprocess.check_call(
      ['go', 'get', '-u', '-t', 'go.skia.org/infra/...'],
      env=env)
  skia_log = subprocess.check_output(
      ['git', 'log', '-n1'],
      cwd=os.path.join(target_dir, 'src', 'go.skia.org', 'infra'))
  print 'Got go.skia.org/infra at:\n%s' % skia_log

  # There's a broken symlink which causes a lot of problems. Delete it.
  bad_symlink = os.path.join(
      target_dir, 'src', 'go.chromium.org', 'luci', 'machine-db', 'appengine',
      'frontend', 'bower_components')
  os.remove(bad_symlink)

  # Install additional dependencies via the install_go_deps.sh script.
  script = os.path.join(
      target_dir, 'src', 'go.skia.org', 'infra', 'scripts',
      'install_go_deps.sh')
  subprocess.check_call(script, env=env)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
