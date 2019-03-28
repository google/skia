#!/usr/bin/env python
# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import base64
import collections
import os
import re
import subprocess
import sys

# Usage: win_ssh_cmd.py <user@host> <cmd shell string>
# Runs the given command over ssh and exits with 0 if the command succeeds or 1
# if the command fails. Ignores all ANSI escape codes.


SENTINEL = 'win_ssh_cmd remote command successful'

# Based on https://stackoverflow.com/a/14693789; edited to incorporate OSC
# based on https://en.wikipedia.org/wiki/ANSI_escape_code#Escape_sequences
ANSI_ESCAPE_RE = re.compile(r'\x1B(\[[0-?]*[ -/]*[@-~]|\].*(\x1B\\|\x07))')

def main(user_host, cmd):
  ssh_cmd = ['ssh', '-oConnectTimeout=15', '-oBatchMode=yes', '-t', '-t',
             user_host, '(' + cmd + ') && echo "%s"' % SENTINEL]
  # orig_lines stores the full output of ssh_cmd.
  orig_lines = []
  # True if we saw a line matching SENTINEL.
  saw_sentinel = False
  print >> sys.stderr, 'Original command:\n%s\nFull command:\n%s' % (
      cmd, ' '.join([repr(s) for s in ssh_cmd]))
  proc = subprocess.Popen(ssh_cmd, stdout=subprocess.PIPE)
  for line in iter(proc.stdout.readline, ''):
    orig_lines.append(line)
    line = ANSI_ESCAPE_RE.sub('', line)
    stripped = line.strip()
    if stripped == SENTINEL:
      saw_sentinel = True
    elif stripped != ''
      print stripped
  proc.wait()
  print >> sys.stderr, 'Raw output base64-encoded:\n%s' % (
      base64.b64encode(''.join(orig_lines)))
  if proc.returncode != 0:
    sys.exit(proc.returncode)
  if not saw_sentinel:
    sys.exit(1)
  sys.exit(0)


if __name__ == '__main__':
  if len(sys.argv) != 3:
    print >> sys.stderr, 'USAGE: %s <user@host> <cmd shell string>' % (
        sys.argv[0])
    sys.exit(1)
  main(sys.argv[1], sys.argv[2])
