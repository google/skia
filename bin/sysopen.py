#! /usr/bin/env python2
# Copyright 2017 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

def sysopen(arg):
  plat = sys.platform
  if plat.startswith('darwin'):
    subprocess.call(["open", arg])
  elif plat.startswith('win'):
    os.startfile(arg)
  else:
    subprocess.call(["xdg-open", arg])

if __name__ == '__main__':
  for a in sys.argv[1:]:
    sysopen(a)
