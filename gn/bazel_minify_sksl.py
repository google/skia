#!/usr/bin/env python3
#
# Copyright 2022 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This script has GN shell out to Bazel in order to minify the sources built into the source code.

import subprocess
import sys

result = subprocess.run(
    ['bazelisk', 'run', '//tools/sksl-minify:minify_srcs'], capture_output=True, encoding='utf-8')
if result.returncode != 0:
    print(result.stdout)
    print(result.stderr)
    sys.exit(result.returncode)
