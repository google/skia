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
fetchClangFormat = sys.argv[3]
processors = sys.argv[4:]

exeSuffix = '.exe' if sys.platform.startswith('win') else '';

for p in processors:
    try:
        if not os.path.isfile(clangFormat + exeSuffix):
            subprocess.check_call([sys.executable, fetchClangFormat]);

        noExt, _ = os.path.splitext(p)
        head, tail = os.path.split(noExt)
        targetDir = os.path.join(head, "generated")
        if not os.path.isdir(targetDir):
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
