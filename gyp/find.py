#!/usr/bin/env python
#
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
find.py is a poor-man's emulation of `find -name=$1 $2` on Unix.

Call python find.py <glob> <directory>... to list all files matching glob under
directory (recursively).  E.g.
   $ python find.py '*.cpp' ../tests/ ../bench/
will print all .cpp files under ../tests/ and ../bench/.
'''

import fnmatch
import os
import sys

for directory in sys.argv[2:]:
  for d, kids, files in os.walk(directory):
    files.sort()
    for f in files:
      if fnmatch.fnmatch(f, sys.argv[1]):
        print os.path.join(d, f).replace('\\', '/')  # Gyp wants Unix paths.
