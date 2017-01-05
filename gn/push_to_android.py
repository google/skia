#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

host, serial, stamp = sys.argv[1:]
device = '/data/local/tmp/' + os.path.basename(host)

# adb push is verbose, so eat its output with check_output().
subprocess.check_output(['adb', '-s', serial, 'push', host, device])
subprocess.check_call(['adb', '-s', serial, 'shell', 'chmod', '+x', device])

# Touch a file to let GN/Ninja know we succeeded.
with open(stamp, 'w'):
  pass
