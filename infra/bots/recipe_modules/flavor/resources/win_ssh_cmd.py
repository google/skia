#!/usr/bin/env python
# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from __future__ import print_function
import base64
import re
import subprocess
import sys


# Usage: win_ssh_cmd.py <user@host> <cmd shell string> [<fail errorlevel>]
# Runs the given command over ssh and exits with 0 if the command succeeds or 1
# if the command fails. The command is considered to fail if the errorlevel is
# greater than or equal to <fail errorlevel>.


SENTINEL = 'win_ssh_cmd remote command successful'


def main(user_host, cmd, fail_errorlevel):
  ssh_cmd = ['ssh', '-oConnectTimeout=15', '-oBatchMode=yes', user_host,
             '(' + cmd + ') & if not errorlevel %s echo %s' % (
                 fail_errorlevel, SENTINEL)]
  # True if we saw a line matching SENTINEL.
  saw_sentinel = False
  print('Original command:\n%s\nFull command:\n%s' % (
      cmd, ' '.join([repr(s) for s in ssh_cmd])), file=sys.stderr)
  proc = subprocess.Popen(ssh_cmd, stdout=subprocess.PIPE)
  for line in iter(proc.stdout.readline, ''):
    stripped = line.strip()
    if stripped == SENTINEL:
      saw_sentinel = True
    else:
      print(stripped)
  proc.wait()
  sys.stdout.flush()
  if proc.returncode != 0:
    sys.exit(proc.returncode)
  if not saw_sentinel:
    sys.exit(1)
  sys.exit(0)


if __name__ == '__main__':
  if len(sys.argv) < 3:
    print('USAGE: %s <user@host> <cmd shell string>  [<fail errorlevel>]' %
        sys.argv[0], file=sys.stderr)
    sys.exit(1)
  arg_fail_errorlevel = 1 if len(sys.argv) < 4 else sys.argv[3]
  main(sys.argv[1], sys.argv[2], arg_fail_errorlevel)
