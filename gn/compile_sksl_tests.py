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
lang = sys.argv[2]
settings = sys.argv[3]
inputs = sys.argv[4:]

def makeEmptyFile(path):
    try:
        open(path, 'wb').close()
    except OSError:
        pass

def compile(skslc, input, target, extension):
    target += extension
    try:
        subprocess.check_output([skslc, input, target, settings], stderr=subprocess.STDOUT)
        return True

    except subprocess.CalledProcessError as err:
        with open(target, 'wb') as dst:
            dst.write("### Compilation failed:\n\n")
            dst.write("\n".join(err.output.splitlines()))
            dst.write("\n")
        return False

if settings != "--settings" and settings != "--nosettings":
    sys.exit("### Expected --settings or --nosettings, got " + settings)

for input in inputs:
    noExt, ext = os.path.splitext(input)
    head, tail = os.path.split(noExt)
    targetDir = os.path.join(head, "golden")
    if not os.path.isdir(targetDir):
        os.mkdir(targetDir)

    target = os.path.join(targetDir, tail)
    if settings == "--nosettings":
        target += "StandaloneSettings"

    if lang == "--fp":
        # First, compile the CPP. If we get an error, stop here.
        if compile(skslc, input, target, ".cpp"):
            # Next, compile the header.
            if compile(skslc, input, target, ".h"):
                # Both files built successfully.
                continue
            else:
                # The header generated an error; this counts as an overall failure for this test.
                # Blank out the passing CPP output since it's not relevant in a failure case.
                makeEmptyFile(target + ".cpp")
        else:
            # The CPP generated an error. We didn't actually generate a header at all, but Ninja
            # expects an output file to exist or it won't reach steady-state.
            makeEmptyFile(target + ".h")
    elif lang == "--glsl":
        compile(skslc, input, target, ".glsl")
    elif lang == "--metal":
        compile(skslc, input, target, ".metal")
    else:
        sys.exit("### Expected one of: --fp --glsl --metal, got " + lang)
