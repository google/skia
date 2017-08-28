#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

msvc = int(sys.argv[1])
windk = sys.argv[2]

if msvc == 2015:
  print windk + '/VC'
else:
  # get the first version
  dir = os.listdir(windk + '/VC/Tools/MSVC/')[0]
  if os.path.exists(dir):
    print dir
    sys.exit(0)
  # fallback to a version
  vc_version = '14.11.25503'
  print windk + '/VC/Tools/MSVC/' + vc_version
