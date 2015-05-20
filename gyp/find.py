# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
find.py is a poor-man's emulation of `find $1 -name=$2` on Unix.

Call python find.py <directory> <glob> to list all files matching glob under
directory (recursively).  E.g.
   $ python find.py ../tests/ '*.cpp'
will print all .cpp files under ../tests/.
'''

import fnmatch
import os
import sys

for d, kids, files in os.walk(sys.argv[1]):
  files.sort()
  for f in files:
    if fnmatch.fnmatch(f, sys.argv[2]):
      print os.path.join(d, f).replace('\\', '/')  # Gyp wants Unix paths.
