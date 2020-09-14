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
inputs = sys.argv[2:]

for input in inputs:
    try:
        noExt, ext = os.path.splitext(input)
        head, tail = os.path.split(noExt)
        targetDir = os.path.join(head, "golden")
        if not os.path.isdir(targetDir):
            os.mkdir(targetDir)
        target = os.path.join(targetDir, tail)
        if ext == ".fp":
            subprocess.check_output([skslc, input, target + ".h"],
                                    stderr=subprocess.STDOUT)
            subprocess.check_output([skslc, input, target + ".cpp"],
                                    stderr=subprocess.STDOUT)
        elif ext == ".sksl":
            subprocess.check_output([skslc, input, target + ".glsl"],
                                    stderr=subprocess.STDOUT)
        else:
            print("### Unrecognized file type for " + input + ", skipped")

    except subprocess.CalledProcessError as err:
        print("### Error compiling " + input + ":")
        print(err.output)
        exit(1)
