#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import sys

msvc = int(sys.argv[1])
windk = sys.argv[2]

# for now, just assume
vc_version = '14.11.25503'

if msvc == 2015:
  print windk + '/VC'
else:
  print windk + '/VC/Tools/MSVC/' + vc_version
