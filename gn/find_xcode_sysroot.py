#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function

import subprocess
import sys

(sdk,) = sys.argv[1:]

print(subprocess.check_output(['xcrun', '--sdk', sdk, '--show-sdk-path']))
