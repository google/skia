# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import sys

src = sys.argv[1] + '/*'
dest   = sys.argv[2]

# We can't use rsync to communicate with the chromebooks because the
# chromebooks don't have rsync installed on them.
print(subprocess.check_output(
      'scp -r %s %s' % (src, dest), shell=True).decode('utf-8'))