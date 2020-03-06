#!/usr/bin/env python
#
# Copyright 2020 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import os
import re
import subprocess
import sys

from infra import git
from infra import go


def run_abandon_cls():
  '''Create a CL to add infra support for the new branch and remove the old.'''
  owner = git.git('config', 'user.email').rstrip()
  if not owner:
    print >> sys.stderr, ('No configured git user; please run '
                          '"git config user.email <your email>".')
    sys.exit(1)
  go.mod_download()
  go.install(go.INFRA_GO+'/scripts/abandon_gerrit_cls')
  subprocess.check_call([
      'abandon_gerrit_cls',
      '--gerrit_instance', 'https://skia-review.googlesource.com',
      '--abandon_reason', '',
      '--last_modified_before_days', '0',
  ])


def main():
  go.check()
  run_abandon_cls()


if __name__ == '__main__':
  main()
