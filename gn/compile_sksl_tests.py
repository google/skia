#!/usr/bin/env python
#
# Copyright 2020 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys
import tempfile

skslc = sys.argv[1]
lang = sys.argv[2]
settings = sys.argv[3]
inputs = sys.argv[4:]

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
worklist = tempfile.NamedTemporaryFile(suffix='.worklist', delete=False)

# Convert the list of command-line inputs into a worklist file sfor skslc.
for input in inputs:
    noExt, ext = os.path.splitext(input)
    head, tail = os.path.split(noExt)
    targetDir = os.path.join(head, "golden")
    if not os.path.isdir(targetDir):
        os.mkdir(targetDir)

    target = os.path.join(targetDir, tail)
    if settings == "--nosettings":
        target += "StandaloneSettings"

    targets.append(target)

    if lang == "--fp":
        worklist.write(input + "\n")
        worklist.write(target + ".cpp\n")
        worklist.write(settings + "\n\n")
        worklist.write(input + "\n")
        worklist.write(target + ".h\n")
        worklist.write(settings + "\n\n")
    elif lang == "--glsl":
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
    else:
        sys.exit("### Expected one of: --fp --glsl --metal --spirv, got " + lang)

# Invoke skslc, passing in the worklist.
worklist.close()
try:
    output = subprocess.check_output([skslc, worklist.name], stderr=subprocess.STDOUT)
except subprocess.CalledProcessError as err:
    if err.returncode != 1:
        print("### skslc error:\n")
        print("\n".join(err.output.splitlines()))
        sys.exit(err.returncode)
    pass  # Compile errors (exit code 1) are expected and normal in test code

os.remove(worklist.name)

# A special case cleanup pass, just for CPP and H files: if either one of these files starts with
# `### Compilation failed`, its sibling should be replaced by an empty file. This improves clarity
# during code review; a failure on either file means that success on the sibling is irrelevant.
if lang == "--fp":
    for target in targets:
        cppFile = open(target + '.cpp', 'r')
        hFile = open(target + '.h', 'r')
        if cppFile.readline().startswith("### Compilation failed"):
            # The CPP had a compilation failure. Clear the header file.
            hFile.close()
            makeEmptyFile(target + '.h')
        elif hFile.readline().startswith("### Compilation failed"):
            # The header had a compilation failure. Clear the CPP file.
            cppFile.close()
            makeEmptyFile(target + '.cpp')
