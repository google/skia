#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

dirpath, = sys.argv[1:]

print(os.path.isdir(dirpath))


