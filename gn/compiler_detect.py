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

def parse_gcc_version(s):
  tokens = s.split()
  major, minor, patch = tokens[2].split('.')
  return int(major),int(minor),int(patch)

if 'clang' in cc_ver and 'clang' in cxx_ver:
  print 'clang'
elif 'GCC' in cc_ver and 'GCC' in cxx_ver:
  if parse_gcc_version(cc_ver)[0] >= 6 and parse_gcc_version(cxx_ver)[0] >= 6:
    print 'new_gcc'
  else:
    print 'old_gcc'
else:
  print 'unknown'
