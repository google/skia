#!/usr/bin/env python
# Copyright 2019 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

'''
Look for the first match in the format
    C:\\Program Files (x86)\\Windows Kits\\10
'''
def find_winsdk():
  if sys.platform.startswith('win'):
    default_dir = r'C:\Program Files (x86)\Windows Kits\10'
    if os.path.isdir(default_dir):
      return default_dir
  return None

if __name__ == '__main__':
  result = find_winsdk()
  if result:
    sys.stdout.write(result + '\n')
