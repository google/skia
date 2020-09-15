#!/usr/bin/env python
#
# Copyright 2020 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

def unlinkIfExists(path):
    try:
        os.unlink(path)
    except OSError:
        pass

def compile(skslc, input, target, extension):
    target += extension
    try:
        subprocess.check_output([skslc, input, target], stderr=subprocess.STDOUT)
        return True

    except subprocess.CalledProcessError as err:
        with open(target, 'wb') as dst:
            dst.write("### Compilation failed:\n\n")
            dst.write("\n".join(err.output.splitlines()))
            dst.write("\n")
        return False

skslc = sys.argv[1]
inputs = sys.argv[2:]

for input in inputs:
    noExt, ext = os.path.splitext(input)
    head, tail = os.path.split(noExt)
    targetDir = os.path.join(head, "golden")
    if not os.path.isdir(targetDir):
        os.mkdir(targetDir)
    target = os.path.join(targetDir, tail)
    if ext == ".fp":
        # First, compile the CPP. If we get an error, stop here.
        if compile(skslc, input, target, ".cpp"):
            # Next, compile the header.
            if compile(skslc, input, target, ".h"):
                # Both files built successfully.
                continue
            else:
                # The header generated an error; this counts as an overall failure for this test.
                # Remove the passing CPP output since it's not relevant in a failure case.
                unlinkIfExists(target + ".cpp")
        else:
            # The CPP generated an error. We didn't actually generate a header, but there might be
            # one from prior runs. Let's remove it for clarity.
            unlinkIfExists(target + ".h")
    elif ext == ".sksl" or ext == ".vert":
        compile(skslc, input, target, ".glsl")
    else:
        print("### Unrecognized file type for " + input + ", skipped")
