#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys
import tempfile

skslc = sys.argv[1]
clangFormat = sys.argv[2]
fetchClangFormat = sys.argv[3]
processors = sys.argv[4:]

exeSuffix = '.exe' if sys.platform.startswith('win') else '';
targets = []
worklist = tempfile.NamedTemporaryFile(suffix='.worklist', delete=False)

# Fetch clang-format if it's not present already.
if not os.path.isfile(clangFormat + exeSuffix):
    subprocess.check_call([sys.executable, fetchClangFormat]);

# Build a worklist of all the fragment processors that we want to compile.
for p in processors:
    noExt, _ = os.path.splitext(p)
    head, tail = os.path.split(noExt)
    targetDir = os.path.join(head, "generated")
    if not os.path.isdir(targetDir):
        os.mkdir(targetDir)
    target = os.path.join(targetDir, tail)
    targets.append(target + ".h")
    targets.append(target + ".cpp")

    worklist.write(p + "\n")
    worklist.write(target + ".h\n\n")
    worklist.write(p + "\n")
    worklist.write(target + ".cpp\n\n")

# Invoke skslc, passing in the worklist.
worklist.close()
try:
    output = subprocess.check_output([skslc, worklist.name], stderr=subprocess.STDOUT)
except subprocess.CalledProcessError as err:
    print("### skslc error:\n")
    print("\n".join(err.output.splitlines()))

os.remove(worklist.name)

# Invoke clang-format on every generated target.
try:
    output = subprocess.check_output([clangFormat, "--sort-includes=false", "-i"] + targets,
                                     stderr=subprocess.STDOUT)
except subprocess.CalledProcessError as err:
    print("### clang-format error:\n")
    print("\n".join(err.output.splitlines()))
