#!/usr/bin/env python2.7
#
# Copyright 2019 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

# Arguments to the script:
#  app              path to binary to package, e.g. out/Debug/gen/dm
app, = sys.argv[1:]

# write dSYM directory
subprocess.check_call(['dsymutil', app])
