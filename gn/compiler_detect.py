#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import sys
cc,cxx = sys.argv[1:3]

cc_ver  = subprocess.check_output('%s --version' % cc, shell=True)
cxx_ver = subprocess.check_output('%s --version' % cxx, shell=True)

def gcc_yyyymmdd(s):
  tokens = s.split()
  return int(tokens[3])

if 'clang' in cc_ver and 'clang' in cxx_ver:
  print 'clang'
elif 'GCC' in cc_ver and 'GCC' in cxx_ver:
  if gcc_yyyymmdd(cc_ver) > 20160000 and gcc_yyyymmdd(cxx_ver) > 20160000:
    print 'new_gcc'
  else:
    print 'old_gcc'
else:
  print 'unknown'
