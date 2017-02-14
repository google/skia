#!/usr/bin/env python
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import re
import subprocess
import sys

logs = ""
err = None
try:
  logs = subprocess.check_output(sys.argv[2:], stderr=subprocess.STDOUT).strip()
except subprocess.CalledProcessError as e:
  err = e

if err and err.output:
  logs = err.output

basedir = sys.argv[1]
stack_line = r'^(?P<path>.+)\(\+?(?P<addr>.*)\) \[(?P<addr2>.+)\]'
extra_path = r'/.*\.\./'
found_stack = False
for line in logs.splitlines():
  line = line.strip()

  m = re.search(stack_line, line)
  if m:
    path = m.group('path')
    addr = m.group('addr')
    addr2 = m.group('addr2')
    if True or os.path.exists(path):
      if not addr or not addr.startswith('0x'):
        addr = addr2
      sym = subprocess.check_output(['addr2line', '-Cfpe', path, addr]).strip()
      if sym and not sym.startswith('?'):
        if path.startswith(basedir):
          path = path[len(basedir)+1:]
        sym = re.sub(extra_path, '', sym)
        line = path + ' ' + sym

  print line

if err:
  sys.exit(err.returncode)
