#!/usr/bin/env python
#
# Copyright 2019 The Flutter and Fuchsia Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
""" Generate a Fuchsia FAR Archive from an asset manifest.
"""

import argparse
import collections
import json
import os
import subprocess
import sys

def CreatePackageIDFile(dst_root, pkg_name, pkg_version):
  meta = os.path.join(dst_root, 'meta')
  if not os.path.isdir(meta):
    os.makedirs(meta)
  content = {}
  content['name'] = pkg_name
  content['version'] = pkg_version
  pkg = os.path.join(meta, 'package')
  print("CreatePackageID: opening: " + pkg)
  with open(pkg, 'w') as out_file:
    json.dump(content, out_file)


#
# Generates the manifest and returns the file.
#
def GenerateManifest(pkg_dir, pkg_name):
  full_paths = []
  for root, dirs, files in os.walk(pkg_dir):
    for f in files:
      common_prefix = os.path.commonprefix([root, pkg_dir])
      rel_path = os.path.relpath(os.path.join(root, f), common_prefix)
      from_pkg = os.path.abspath(os.path.join(pkg_dir, rel_path))
      full_paths.append('%s=%s' % (rel_path, from_pkg))
  parent_dir = os.path.abspath(os.path.join(pkg_dir, os.pardir))
  manifest_file_name = os.path.basename(pkg_dir) + '.manifest'
  manifest_path = os.path.join(parent_dir, manifest_file_name)
  with open(manifest_path, 'w') as f:
    for item in full_paths:
      f.write("%s\n" % item) 
  return manifest_path


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

  # if not os.path.exists(os.path.join(pkg_dir, 'meta', 'pkg')):
  #   CreatePackageIDFile(pkg_dir, args.pkg_name, args.pkg_version)
  # pkg_manifest = GenerateManifest(pkg_dir, pkg_name)

  pm_command_base = [
      args.pm_bin,
      '-o',
      pkg_dir,
  ]

  # Create the package ID file
  print("PM INIT\n")
  subprocess.check_call(pm_command_base + ['-n'] + [pkg_name] + ['init'])

  # Build the package
  print("PM BUILD\n")
  subprocess.check_call(pm_command_base + ['-m'] + [pkg_manifest] + ['build'])

  # Archive the package
  print("PM ARCHIVE\n")
  subprocess.check_call(pm_command_base + ['-m'] + [pkg_manifest] + ['-version'] + [pkg_version] + ['archive'])

  return 0


if __name__ == '__main__':
  sys.exit(main())
