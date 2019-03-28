#!/usr/bin/env python
# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=line-too-long

import collections
import os
import re
import subprocess
import sys

# Usage: win_ssh_cmd.py <user@host> <cmd shell string>
# Runs the given command over ssh and exits with 0 if the command succeeds or 1
# if the command fails.

def main(user_host, cmd):
  sentinel = 'win_ssh_cmd remote command successful'
  ssh_cmd = ['ssh', '-oConnectTimeout=15', '-oBatchMode=yes', '-t', '-t',
             user_host, '(' + cmd + ') && echo "%s"' % sentinel]
  last = None
  proc = subprocess.Popen(ssh_cmd, stdout=subprocess.PIPE)
  for line in iter(proc.stdout.readline, ''):
    if last:
      sys.stdout.write(last)
    if line.strip() == sentinel:
      last = line
    else:
      sys.stdout.write(line)
  proc.wait()
  if proc.returncode != 0:
    if last:
      sys.stdout.write(last)
    sys.exit(proc.returncode)
  if not last.strip() == sentinel:
    sys.exit(1)
  sys.exit(0)


if __name__ == '__main__':
  if len(sys.argv) != 3:
    print >> sys.stderr, 'USAGE: %s <user@host> <cmd shell string>' % (
        sys.argv[0])
    sys.exit(1)
  main(sys.argv[1], sys.argv[2])
