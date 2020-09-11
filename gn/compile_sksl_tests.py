#!/usr/bin/env python
#
# Copyright 2020 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

def compile(skslc, input, target, extension):
    target += extension
    try:
        subprocess.check_output([skslc, input, target], stderr=subprocess.STDOUT)

    except subprocess.CalledProcessError as err:
        dst = open(target, 'wb')
        dst.write("### Compilation failed\n\n")
        dst.write(err.output)
        dst.close()

skslc = sys.argv[1]
inputs = sys.argv[2:]

for input in inputs:
    print("Recompiling " + input + "...")
    noExt, ext = os.path.splitext(input)
    head, tail = os.path.split(noExt)
    targetDir = os.path.join(head, "golden")
    if not os.path.isdir(targetDir):
        os.mkdir(targetDir)
    target = os.path.join(targetDir, tail)
    if ext == ".fp":
        compile(skslc, input, target, ".h")
        compile(skslc, input, target, ".cpp")
    elif ext == ".sksl":
        compile(skslc, input, target, ".glsl")
    else:
        print("### Unrecognized file type for " + input + ", skipped")
