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

print(sorted(filter(regex.match, os.listdir(dirpath)))[-1])
