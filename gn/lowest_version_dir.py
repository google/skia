#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import re
import sys

dirpath = sys.argv[1]
regex = re.compile(sys.argv[2])
childpath = sys.argv[3]

matches = sorted(filter(regex.match, os.listdir(dirpath)))
for match in matches:
    path = os.path.join(dirpath, match, childpath)
    if os.path.isdir(path):
        print path
        break
