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
output = sys.argv[2]
includes = sys.argv[3:]

for inc in includes:
    print("Recompiling " + inc + "...")
    try:
        noExt, _ = os.path.splitext(inc)
        head, tail = os.path.split(noExt)
        if head.endswith("generated"):
            targetDir = head
        else:
            targetDir = os.path.join(head, "generated")
        if not os.path.isdir(targetDir):
            os.mkdir(targetDir)
        target = os.path.join(targetDir, tail)
        print(skslc + " " + inc + " " + target + ".include.cpp")
        subprocess.check_output([skslc, inc, target + ".include.cpp"])
    except subprocess.CalledProcessError as err:
        print("### Error compiling " + inc + ":")
        print(err.output)
        exit(1)
open(output, 'w')
