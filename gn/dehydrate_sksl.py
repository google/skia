#!/usr/bin/env python
#
# Copyright 2020 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

skslc = sys.argv[1]
targetDir = sys.argv[2]
includes = sys.argv[3:]

for inc in includes:
    try:
        noExt, _ = os.path.splitext(inc)
        head, tail = os.path.split(noExt)
        if not os.path.isdir(targetDir):
            os.mkdir(targetDir)
        target = os.path.join(targetDir, tail)
        subprocess.check_output([
            skslc, inc, target + ".dehydrated.sksl"]).decode('utf-8')
    except subprocess.CalledProcessError as err:
        print("### Error compiling " + inc + ":")
        print(err.output)
        exit(1)
