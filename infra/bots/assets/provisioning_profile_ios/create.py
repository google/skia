#!/usr/bin/env python
#
# Copyright 2026 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


from __future__ import print_function
import argparse
import os
import shutil
import subprocess
import sys
import tempfile


orig_pkg = 'chrome_internal/ios/provisioning_profiles/org_chromium'
provision_file = 'Upstream_Com_Testing_Provisioning_Profile.mobileprovision'


def create_asset(target_dir):
  with tempfile.TemporaryDirectory() as tempdir:
    subprocess.check_call(['cipd', 'init', '--force', '.'], cwd=tempdir)
    subprocess.check_call(['cipd', 'install', orig_pkg, 'latest'], cwd=tempdir)
    shutil.copy2(os.path.join(tempdir, provision_file), target_dir)

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
