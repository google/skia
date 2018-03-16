#!/usr/bin/env python
#
# Copyright 2018 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import hashlib
import sys

h = hashlib.md5()
for fn in sys.argv:
  with open(fn, 'rb') as f:
    h.update(f.read())

print(h.hexdigest())
