#!/usr/bin/env python
#
# Copyright 2020 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import shlex
import subprocess
import sys
import tempfile

batchCompile = True

skslc = sys.argv[1]
lang = sys.argv[2]
settings = sys.argv[3]
with open(sys.argv[4], 'r') as reader:
    inputs = shlex.split(reader.read())

def pairwise(iterable):
    # Iterate over an array pairwise (two elements at a time).
    a = iter(iterable)
    return zip(a, a)

def executeWorklist(input, worklist):
    # Invoke skslc, passing in the worklist.
    worklist.close()
    try:
        output = subprocess.check_output([skslc, worklist.name], stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as err:
        if err.returncode != 1:
            print("### " + input + " skslc error:\n")
            print("\n".join(err.output.splitlines()))
            sys.exit(err.returncode)
        pass  # Compile errors (exit code 1) are expected and normal in test code

    # Delete the worklist file now that execution is complete.
    os.remove(worklist.name)

def makeEmptyFile(path):
    try:
        open(path, 'wb').close()
    except OSError:
        pass

def extensionForSpirvAsm(ext):
    return ext if (ext == '.frag' or ext == '.vert' or ext == '.geom') else '.frag'

if settings != "--settings" and settings != "--nosettings":
    sys.exit("### Expected --settings or --nosettings, got " + settings)

targets = []
worklist = tempfile.NamedTemporaryFile(suffix='.worklist', delete=False, mode='w')

# The `inputs` array pairs off input files with their matching output directory, e.g.:
#     //skia/tests/sksl/shared/test.sksl
#     //skia/tests/sksl/shared/golden/
#     //skia/tests/sksl/intrinsics/abs.sksl
#     //skia/tests/sksl/intrinsics/golden/
#     ... (etc) ...
# Here we loop over these inputs and convert them into a worklist file for skslc.
for input, targetDir in pairwise(inputs):
    noExt, ext = os.path.splitext(input)
    head, tail = os.path.split(noExt)
    if not os.path.isdir(targetDir):
        os.mkdir(targetDir)

    target = os.path.join(targetDir, tail)
    if settings == "--nosettings":
        target += "StandaloneSettings"

    targets.append(target)

    if lang == "--glsl":
        worklist.write(input + "\n")
        worklist.write(target + ".glsl\n")
        worklist.write(settings + "\n\n")
    elif lang == "--metal":
        worklist.write(input + "\n")
        worklist.write(target + ".metal\n")
        worklist.write(settings + "\n\n")
    elif lang == "--spirv":
        worklist.write(input + "\n")
        worklist.write(target + ".asm" + extensionForSpirvAsm(ext) + "\n")
        worklist.write(settings + "\n\n")
    elif lang == "--skvm":
        worklist.write(input + "\n")
        worklist.write(target + ".skvm\n")
        worklist.write(settings + "\n\n")
    elif lang == "--stage":
        worklist.write(input + "\n")
        worklist.write(target + ".stage\n")
        worklist.write(settings + "\n\n")
    else:
        sys.exit("### Expected one of: --glsl --metal --spirv --skvm --stage --dsl, got " + lang)

    # Compile items one at a time.
    if not batchCompile:
        executeWorklist(input, worklist)
        worklist = tempfile.NamedTemporaryFile(suffix='.worklist', delete=False, mode='w')

# Compile everything all in one go.
if batchCompile:
    executeWorklist("", worklist)
else:
    worklist.close()
    os.remove(worklist.name)
