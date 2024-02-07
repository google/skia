# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import sys

bin_dir = sys.argv[1]
sh = sys.argv[2]
adb = sys.argv[3]
subprocess.check_call([adb, 'shell', 'sh', bin_dir + sh])
try:
  sys.exit(int(subprocess.check_output([
      adb, 'shell', 'cat', bin_dir + 'rc']).decode('utf-8')))
except ValueError:
  print("Couldn't read the return code.  Probably killed for OOM.")
  sys.exit(1)
