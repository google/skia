#!/usr/bin/env python

# Copyright 2019 Google LLC. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
 Builds a Fuchsia FAR archive.
"""

import argparse
import os
import subprocess
import sys

def main():
  parser = argparse.ArgumentParser()

  parser.add_argument('--pm-bin', dest='pm_bin', action='store', required=True)
  parser.add_argument(
      '--pkg-dir', dest='pkg_dir', action='store', required=True)
  parser.add_argument(
      '--pkg-name', dest='pkg_name', action='store', required=True)
  parser.add_argument(
      '--pkg-version', dest='pkg_version', action='store', required=True)
  parser.add_argument(
      '--pkg-manifest', dest='pkg_manifest', action='store', required=True)

  args = parser.parse_args()

  assert os.path.exists(args.pm_bin)
  assert os.path.exists(args.pkg_dir)

  pkg_dir = args.pkg_dir
  pkg_name = args.pkg_name
  pkg_manifest = args.pkg_manifest
  pkg_version = args.pkg_version

  pm_command_base = [
      args.pm_bin,
      '-o',
      pkg_dir,
  ]

  # Create the package ID file.
  subprocess.check_call(pm_command_base + ['-n'] + [pkg_name] + ['init'])

  # Build the package.
  subprocess.check_call(pm_command_base + ['-m'] + [pkg_manifest] + ['build'])

  # Archive the package.
  subprocess.check_call(pm_command_base + ['-m'] + [pkg_manifest] + ['-version'] + [pkg_version] + ['archive'])

  return 0


if __name__ == '__main__':
  sys.exit(main())
