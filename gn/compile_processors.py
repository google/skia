#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

def process(skslc, src, dst):
    path, _ = os.path.splitext(src)
    filename = os.path.split(path)[1]
    if subprocess.call([skslc, src, os.path.join(dst, filename + ".h")]) > 0:
        exit(1)
    if subprocess.call([skslc, src, os.path.join(dst, filename + ".cpp")]) > 0:
        exit(1)

def main():
    skslc = sys.argv[1]
    dst = sys.argv[2]
    processors = sys.argv[3:]
    for p in processors:
        process(skslc, p, dst)

if __name__ == "__main__":
    main()
