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
processors = sys.argv[2:]
for p in processors:
    path, _ = os.path.splitext(p)
    subprocess.check_call([skslc, p, path + ".h"])
    subprocess.check_call([skslc, p, path + ".cpp"])
