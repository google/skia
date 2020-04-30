#!/usr/bin/env python
# Copyright 2019 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys
import subprocess

'''
Look for the first match in the format
    C:\\Program Files (x86)\\Microsoft Visual Studio\\${RELEASE}\\${VERSION}\\VC
'''
def find_msvc():
  if sys.platform.startswith('win'):
    default_dir = r'C:\Program Files (x86)\Microsoft Visual Studio'
    for release in ['2019', '2017']:
      for version in ['Enterprise', 'Professional', 'Community', 'BuildTools', 'Preview']:
        path = os.path.join(default_dir, release, version, 'VC')
        if os.path.isdir(path):
          return path

    # Fall back to vswhere.exe to determine non-standard installation paths
    # Fixed location, https://github.com/Microsoft/vswhere/wiki/Installing
    vswhere = os.path.join(os.getenv('ProgramFiles(x86)'),
              'Microsoft Visual Studio', 'Installer', 'vswhere.exe')
    command = (vswhere + ' -prerelease -legacy -products * -sort -utf8 '
              '-property installationPath')
    paths = subprocess.check_output(command).decode('utf-8').splitlines()
    if paths:
      return paths[0] + '\\VC'

  return None

if __name__ == '__main__':
  result = find_msvc()
  if result:
    sys.stdout.write(result + '\n')
