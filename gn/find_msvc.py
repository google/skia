#!/usr/bin/env python
# Copyright 2019 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

'''
Look for the first match in the format
    C:\Program Files (x86)\Microsoft Visual Studio\${RELEASE}\${VERSION}\VC
'''
def find_msvc():
  default_dir = r'C:\Program Files (x86)\Microsoft Visual Studio'
  for release in ['2019', '2017']:
    for version in ['Enterprise', 'Professional', 'Community', 'BuildTools' ]:
      path = os.path.join(default_dir, release, version, 'VC')
      if os.path.isdir(path):
        return path

if __name__ == '__main__':
  path = find_msvc()
  if path:
    sys.stdout.write(path + '\n')
