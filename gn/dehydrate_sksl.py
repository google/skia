#!/usr/bin/env python
#
# Copyright 2020 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

sksl_precompile = sys.argv[1]
targetDir = sys.argv[2]
modules = sys.argv[3:]

for module in modules:
    try:
        noExt, _ = os.path.splitext(module)
        head, tail = os.path.split(noExt)
        if not os.path.isdir(targetDir):
            os.mkdir(targetDir)
        target = os.path.join(targetDir, tail)
        subprocess.check_output([
            sksl_precompile, target + ".dehydrated.sksl", module]).decode('utf-8')
    except subprocess.CalledProcessError as err:
        print("### Error compiling " + module + ":")
        print(err.output)
        exit(1)
