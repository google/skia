#!/usr/bin/env python
#
# Copyright 2022 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import shlex
import subprocess
import sys
import tempfile

batchMinify = False

sksl_minify = sys.argv[1]
shared_module = sys.argv[2]
public_module = sys.argv[3]
with open(sys.argv[4], 'r') as reader:
    inputs = shlex.split(reader.read())

def pairwise(iterable):
    # Iterate over an array pairwise (two elements at a time).
    a = iter(iterable)
    return zip(a, a)

def executeWorklist(input, worklist):
    # Invoke sksl-minify, passing in the worklist.
    worklist.close()
    try:
        output = subprocess.check_output([
            sksl_minify, worklist.name], stderr=subprocess.STDOUT).decode('utf-8', errors='ignore')
    except subprocess.CalledProcessError as err:
        if err.returncode != 1:
            print("### " + input + " sksl-minify error:\n")
            print("\n".join(err.output.decode('utf-8', errors='ignore').splitlines()))
            sys.exit(err.returncode)
        pass  # Compile errors (exit code 1) are expected and normal in test code

    # Delete the worklist file now that execution is complete.
    os.remove(worklist.name)

worklist = tempfile.NamedTemporaryFile(suffix='.worklist', delete=False, mode='w')

# The `inputs` array pairs off input files with their matching output directory, e.g.:
#     //skia/resources/sksl/shared/HelloWorld.rts
#     //skia/tests/sksl/shared/
#     //skia/resources/sksl/intrinsics/Abs.rts
#     //skia/tests/sksl/intrinsics/
#     ... (etc) ...
# Here we loop over these inputs and convert them into a worklist file for sksl-minify.
for input, targetDir in pairwise(inputs):
    noExt, ext = os.path.splitext(input)
    head, tail = os.path.split(noExt)

    if not os.path.isdir(targetDir):
        os.mkdir(targetDir)

    target = os.path.join(targetDir, tail)

    if ext == '.rts':
        worklist.write("--shader\n")
    elif ext == '.rtcf':
        worklist.write("--colorfilter\n")
    elif ext == '.rtb':
        worklist.write("--blender\n")
    worklist.write(target + ".minified.sksl\n")
    worklist.write(input + "\n")
    worklist.write(public_module + "\n")
    worklist.write(shared_module + "\n\n")

    # Compile items one at a time.
    if not batchMinify:
        executeWorklist(input, worklist)
        worklist = tempfile.NamedTemporaryFile(suffix='.worklist', delete=False, mode='w')

# Compile everything all in one go.
if batchMinify:
    executeWorklist("", worklist)
else:
    worklist.close()
    os.remove(worklist.name)
