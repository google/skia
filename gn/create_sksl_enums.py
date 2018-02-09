#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

src = open(sys.argv[1], 'r')
dst = open(sys.argv[2], 'wb')
dst.write('R"(')
for line in src.readlines():
    if not line.startswith("#"):
        dst.write(line)
dst.write(')"\n')
src.close()
dst.close()
