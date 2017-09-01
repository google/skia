#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

src,out = sys.argv[1:3]
moc = os.environ["QT_PATH"] + '/bin/moc'
subprocess.call([moc, src, '-o', out])

