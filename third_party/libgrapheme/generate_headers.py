#!/usr/bin/env python

# Copyright 2023 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
Generate c header out of txt file

Output is a header
'''

import subprocess
import sys

header = open(sys.argv[2], "w")
code = subprocess.Popen(sys.argv[1], shell=True, universal_newlines=True, stdout=header, cwd=sys.argv[3])
exit(code.wait())
