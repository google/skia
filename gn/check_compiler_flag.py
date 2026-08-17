#!/usr/bin/env python
#
# Copyright 2026 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Checks whether a compiler supports a given flag.
# Prints "true" or "false" to stdout for consumption by GN's exec_script().

import os
import shlex
import subprocess
import sys

def main():
    if len(sys.argv) != 3:
        print("false")
        return

    compiler = sys.argv[1]
    flag = sys.argv[2]

    compiler_cmd = shlex.split(compiler, posix=(os.name != 'nt'))
    cmd = compiler_cmd + ['-Werror', flag, '-fsyntax-only', '-x', 'c++', os.devnull]

    try:
        subprocess.check_call(
            cmd,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            timeout=10,
        )
        print("true")
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired, OSError):
        print("false")

if __name__ == '__main__':
    main()
