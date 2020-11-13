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

if settings != "--settings" and settings != "--nosettings":
    sys.exit("### Expected --settings or --nosettings, got " + settings)

skslcArgs = [skslc]
targets = []

# Convert the list of command-line inputs into a worklist file sfor skslc.
for input in inputs:
    noExt, _ = os.path.splitext(input)
    head, tail = os.path.split(noExt)
    targetDir = os.path.join(head, "golden")
    if not os.path.isdir(targetDir):
        os.mkdir(targetDir)

    target = os.path.join(targetDir, tail)
    if settings == "--nosettings":
        target += "StandaloneSettings"

    targets.append(target)

    if lang == "--fp":
        skslcArgs.append("--")
        skslcArgs.append(input)
        skslcArgs.append(target + ".cpp")
        skslcArgs.append(settings)
        skslcArgs.append("--")
        skslcArgs.append(input)
        skslcArgs.append(target + ".h")
        skslcArgs.append(settings)
    elif lang == "--glsl":
        skslcArgs.append("--")
        skslcArgs.append(input)
        skslcArgs.append(target + ".glsl")
        skslcArgs.append(settings)
    elif lang == "--metal":
        skslcArgs.append("--")
        skslcArgs.append(input)
        skslcArgs.append(target + ".metal")
        skslcArgs.append(settings)
    else:
        sys.exit("### Expected one of: --fp --glsl --metal, got " + lang)

# Invoke skslc on every target that needs to be compiled.
try:
    output = subprocess.check_output(skslcArgs, stderr=subprocess.STDOUT)
except subprocess.CalledProcessError as err:
    if err.returncode != 1:
        print("### skslc error:\n")
        print("\n".join(err.output.splitlines()))
        sys.exit(err.returncode)
    pass  # Compile errors (exit code 1) are expected and normal in test code

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
