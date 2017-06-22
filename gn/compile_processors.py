#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

skslc = sys.argv[1]
dst = sys.argv[2]
processors = sys.argv[3:]
for p in processors:
    path, _ = os.path.splitext(p)
    filename = os.path.split(path)[1]
    subprocess.check_call([skslc, p, os.path.join(dst, filename + ".h")])
    subprocess.check_call([skslc, p, os.path.join(dst, filename + ".cpp")])
