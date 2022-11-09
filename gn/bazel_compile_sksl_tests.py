#!/usr/bin/env python3
#
# Copyright 2022 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This script has GN shell out to Bazel in order to compile all the skslc test files

import subprocess
import sys

sksl_tests = ['glsl_tests', 'glsl_nosettings_tests', 'metal_tests', 'skvm_tests',
              'stage_tests', 'spirv_tests', 'wgsl_tests']

for target in sksl_tests:
    result = subprocess.run(
        ['bazelisk', 'run', '//tools/skslc:compile_'+target], capture_output=True, encoding='utf-8')
    if result.returncode != 0:
        print(result.stdout)
        print(result.stderr)
        sys.exit(result.returncode)
