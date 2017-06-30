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
    print("Recompiling " + p + "...")
    try:
        subprocess.check_output([skslc, p, path + ".h"])
        subprocess.check_output([skslc, p, path + ".cpp"])
    except subprocess.CalledProcessError as err:
        print("### Error compiling " + p + ":")
        print(err.output)
        exit(1)
