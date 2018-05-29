#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

windk = sys.argv[1]

# get the first version
root = windk + '/VC/Tools/MSVC/'
if os.path.exists(root):
  for vc_dir in os.listdir(root):
    print root + vc_dir
    sys.exit(0)
