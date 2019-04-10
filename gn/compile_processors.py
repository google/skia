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
clangFormat = sys.argv[2]
processors = sys.argv[3:]
for p in processors:
    print("Recompiling " + p + "...")
    try:
        noExt, _ = os.path.splitext(p)
        head, tail = os.path.split(noExt)
        targetDir = os.path.join(head, "generated")
        if not os.path.exists(targetDir):
            os.mkdir(targetDir)
        target = os.path.join(targetDir, tail)
        subprocess.check_output([skslc, p, target + ".h"])
        subprocess.check_call(clangFormat + " --sort-includes=false -i \"" +
                              target + ".h\"", shell=True)
        subprocess.check_output([skslc, p, target + ".cpp"])
        subprocess.check_call(clangFormat + " --sort-includes=false -i \"" +
                              target + ".cpp\"", shell=True)
    except subprocess.CalledProcessError as err:
        print("### Error compiling " + p + ":")
        print(err.output)
        exit(1)
