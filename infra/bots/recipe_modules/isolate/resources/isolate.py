#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Calls the isolate Go executable in the checkout, failing if it cannot run.
"""

# TODO(djd): make the caller invoke the Go binary directly, kill this script.

import os
import subprocess
import sys


def try_go(path, args):
  """Tries to run the Go implementation of isolate.
  """
  luci_go = os.path.join(os.path.dirname(path), 'luci-go')
  if sys.platform == 'win32':
    exe = os.path.join(luci_go, 'win64', 'isolate.exe')
  elif sys.platform == 'darwin':
    exe = os.path.join(luci_go, 'mac64', 'isolate')
  else:
    exe = os.path.join(luci_go, 'linux64', 'isolate')

  return subprocess.call([exe] + args)


def main():
  path = sys.argv[1]
  args = sys.argv[2:]
  return try_go(path, args)


if __name__ == '__main__':
  sys.exit(main())
