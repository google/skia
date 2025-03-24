# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import sys

args = sys.argv[1:]
# Add '/*' to the source directory.
args[-2] = args[-2] + '/*'

# We can't use rsync to communicate with the chromebooks because the
# chromebooks don't have rsync installed on them.
cmd = ['scp', '-r'] + args
print(subprocess.check_output(
      ' '.join(cmd), shell=True).decode('utf-8'))
