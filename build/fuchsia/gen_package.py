#!/usr/bin/env python
#
# Copyright 2013 The Flutter Authors. All rights reserved.
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

def CreateMetaPackage(dst_root, far_name):
  meta = os.path.join(dst_root, 'meta')
  if not os.path.isdir(meta):
    os.makedirs(meta)
  content = {}
  content['name'] = far_name
  content['version'] = '0'
  package = os.path.join(meta, 'package')
  with open(package, 'w') as out_file:
    json.dump(content, out_file)


# Generates the manifest and returns the file.
def GenerateManifest(package_dir):
  full_paths = []
  for root, dirs, files in os.walk(package_dir):
    for f in files:
      common_prefix = os.path.commonprefix([root, package_dir])
      rel_path = os.path.relpath(os.path.join(root, f), common_prefix)
      from_package = os.path.abspath(os.path.join(package_dir, rel_path))
      full_paths.append('%s=%s' % (rel_path, from_package))
  parent_dir = os.path.abspath(os.path.join(package_dir, os.pardir))
  manifest_file_name = os.path.basename(package_dir) + '.manifest'
  manifest_path = os.path.join(parent_dir, manifest_file_name)
  with open(manifest_path, 'w') as f:
    for item in full_paths:
      f.write("%s\n" % item)
  return manifest_path


def CreateFarPackage(pm_bin, package_dir, dst_dir):
  manifest_path = GenerateManifest(package_dir)

  pm_command_base = [
      pm_bin, '-m', manifest_path, '-o', dst_dir
  ]

  # Build the package
  subprocess.check_call(pm_command_base + ['build'])

  # Archive the package
  subprocess.check_call(pm_command_base + ['archive'])

  return 0


def main():
  parser = argparse.ArgumentParser()

  parser.add_argument('--pm-bin', dest='pm_bin', action='store', required=True)
  parser.add_argument(
      '--package-dir', dest='package_dir', action='store', required=True)
  parser.add_argument(
      '--manifest-file', dest='manifest_file', action='store', required=False)
  parser.add_argument(
      '--far-name', dest='far_name', action='store', required=False)

  args = parser.parse_args()

  assert os.path.exists(args.pm_bin)
  assert os.path.exists(args.package_dir)

  pkg_dir = args.package_dir
  if not os.path.exists(os.path.join(pkg_dir, 'meta', 'package')):
    CreateMetaPackage(pkg_dir, args.far_name)

  manifest_file = None
  if args.manifest_file is not None:
    assert os.path.exists(args.manifest_file)
    manifest_file = args.manifest_file
  else:
    manifest_file = GenerateManifest(args.package_dir)

  pm_command_base = [
      args.pm_bin,
      '-o',
      os.path.abspath(os.path.join(pkg_dir, os.pardir)),
      '-m',
      manifest_file,
  ]

  # Build the package
  subprocess.check_call(pm_command_base + ['build'])

  # Archive the package
  subprocess.check_call(pm_command_base + ['archive'])

  return 0


if __name__ == '__main__':
  sys.exit(main())
