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
skslcArgs = [skslc]
clangFormatArgs = [clangFormat, "--sort-includes=false", "-i"]

# Fetch clang-format if it's not present already.
if not os.path.isfile(clangFormat + exeSuffix):
    subprocess.check_call([sys.executable, fetchClangFormat]);

# Build argument lists for all the fragment processors that we want to compile.
for p in processors:
    noExt, _ = os.path.splitext(p)
    head, tail = os.path.split(noExt)
    targetDir = os.path.join(head, "generated")
    if not os.path.isdir(targetDir):
        os.mkdir(targetDir)
    target = os.path.join(targetDir, tail)
    clangFormatArgs.append(target + ".h")
    clangFormatArgs.append(target + ".cpp")
    skslcArgs.append("--");
    skslcArgs.append(p);
    skslcArgs.append(target + ".h");
    skslcArgs.append("--");
    skslcArgs.append(p);
    skslcArgs.append(target + ".cpp");

# Invoke skslc on every target that needs to be compiled.
try:
    output = subprocess.check_output(skslcArgs, stderr=subprocess.STDOUT)
except subprocess.CalledProcessError as err:
    print("### skslc error:\n")
    print("\n".join(err.output.splitlines()))
    sys.exit(err.returncode)

# Invoke clang-format on every generated target.
try:
    output = subprocess.check_output(clangFormatArgs, stderr=subprocess.STDOUT)
except subprocess.CalledProcessError as err:
    print("### clang-format error:\n")
    print("\n".join(err.output.splitlines()))
    sys.exit(err.returncode)
