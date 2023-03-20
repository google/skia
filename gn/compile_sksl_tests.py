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
input_root_dir = sys.argv[4]
output_root_dir = sys.argv[5]
# The last arg is a file containing a space seperated list of filenames
input_file = sys.argv[6]
with open(input_file, 'r') as reader:
    inputs = shlex.split(reader.read())

def executeWorklist(input, worklist):
    # Invoke skslc, passing in the worklist.
    worklist.close()
    try:
        output = subprocess.check_output([
            skslc, worklist.name], stderr=subprocess.STDOUT).decode('utf-8', errors='ignore')
    except subprocess.CalledProcessError as err:
        if err.returncode != 1:
            print("### " + input + " skslc error:\n")
            print("\n".join(err.output.decode('utf-8', errors='ignore').splitlines()))
            sys.exit(err.returncode)
        pass  # Compile errors (exit code 1) are expected and normal in test code

    # Delete the worklist file now that execution is complete.
    os.remove(worklist.name)

def extensionForSpirvAsm(ext):
    return ext if (ext == '.frag' or ext == '.vert') else '.frag'

if settings != "--settings" and settings != "--nosettings":
    sys.exit("### Expected --settings or --nosettings, got " + settings)

worklist = tempfile.NamedTemporaryFile(suffix='.worklist', delete=False, mode='w')

# Here we loop over the inputs and convert them into a worklist file for sksl-minify.
for input in inputs:
    # Derive the target path from the input filename and remove the extension so it can
    # end with .minified.sksl
    target = input.replace(input_root_dir, output_root_dir)
    target = os.path.splitext(target)[0]
    target_dir = os.path.dirname(target)
    if not os.path.isdir(target_dir):
        os.mkdir(target_dir)

    noExt, ext = os.path.splitext(input)
    head, tail = os.path.split(noExt)

    if settings == "--nosettings":
        target += "StandaloneSettings"

    if lang == "--glsl":
        worklist.write(input + "\n")
        worklist.write(target + ".glsl\n")
        worklist.write(settings + "\n\n")
    elif lang == "--metal":
        worklist.write(input + "\n")
        worklist.write(target + ".metal\n")
        worklist.write(settings + "\n\n")
    elif lang == "--hlsl":
        worklist.write(input + "\n")
        worklist.write(target + ".hlsl\n")
        worklist.write(settings + "\n\n")
    elif lang == "--spirv":
        worklist.write(input + "\n")
        worklist.write(target + ".asm" + extensionForSpirvAsm(ext) + "\n")
        worklist.write(settings + "\n\n")
    elif lang == "--skrp":
        worklist.write(input + "\n")
        worklist.write(target + ".skrp\n")
        worklist.write(settings + "\n\n")
    elif lang == "--skvm":
        worklist.write(input + "\n")
        worklist.write(target + ".skvm\n")
        worklist.write(settings + "\n\n")
    elif lang == "--stage":
        worklist.write(input + "\n")
        worklist.write(target + ".stage\n")
        worklist.write(settings + "\n\n")
    elif lang == "--wgsl":
        worklist.write(input + "\n")
        worklist.write(target + ".wgsl\n")
        worklist.write(settings + "\n\n")
    else:
        sys.exit("### Expected one of: --glsl --metal --hlsl --spirv --skrp " +
                 "--skvm --stage --wgsl, got " + lang)

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
