#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

# Equivalent to: rm -f $2 && $1 rcs $2 @$3

ar, output, rspfile = sys.argv[1:]

if os.path.exists(output):
  os.remove(output)

if sys.platform != 'darwin':
  sys.exit(subprocess.call([ar, "rcs", output, "@" + rspfile]))

# Mac ar doesn't support @rspfile syntax.
objects = open(rspfile).read().split()
# It also spams stderr with warnings about objects having no symbols.
pipe = subprocess.Popen([ar, "rcs", output] + objects, stderr=subprocess.PIPE)
_, err = pipe.communicate()
for line in err.splitlines():
  if 'has no symbols' not in line:
    sys.stderr.write(line + '\n')
sys.exit(pipe.returncode)
