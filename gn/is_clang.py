#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import sys
cc,cxx = sys.argv[1:3]

def check_output(cmd):
  output = subprocess.check_output('%s --version' % cmd, shell=True)
  if sys.version_info[0] == 3:
    output = output.decode(encoding='utf-8', errors='strict')
  return output

if ('clang' in check_output(cc) and 'clang' in check_output(cxx)):
  print('true')
else:
  print('false')

