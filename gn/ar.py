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
sys.exit(subprocess.call([ar, "rcs", output, "@" + rspfile]))
