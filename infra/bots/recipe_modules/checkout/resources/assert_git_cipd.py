# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import sys

which = 'where' if sys.platform == 'win32' else 'which'
git = subprocess.check_output([which, 'git']).decode('utf-8')
print('git was found at %s' % git)
if 'cipd_bin_packages' not in git:
  print('Git must be obtained through CIPD.', file=sys.stderr)
  sys.exit(1)
