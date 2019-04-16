#!/usr/bin/env python
#
# Copyright 2019 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import subprocess
import sys

from infra import git
from infra import go


REFS_HEADS_PREFIX = 'refs/heads/'
CHROME_REF_PREFIX = REFS_HEADS_PREFIX + 'chrome/m'
SUPPORTED_CHROME_BRANCHES = 2  # Per infra policy; see skbug.com/8940


def get_chrome_branches():
  '''Return all Chrome milestone branches as tuples of (milestone, ref).'''
  refs = git.git('ls-remote', 'origin', 'refs/heads/*')
  chrome_branches = []
  for line in refs.splitlines():
    ref = line.split()[1]
    if ref.startswith(CHROME_REF_PREFIX):
      m = int(ref[len(CHROME_REF_PREFIX):])
      chrome_branches.append((m, ref))
  chrome_branches.sort(reverse=True)
  return chrome_branches


def main():
  owner = git.git('config', 'user.email').rstrip()
  if not owner:
    print >> sys.stderr, 'No configured git user; please run "git config user.email <your email>".'
    sys.exit(1)
  branches = get_chrome_branches()
  new_branch = branches[0][1][len(REFS_HEADS_PREFIX):]
  old_branch = branches[SUPPORTED_CHROME_BRANCHES][1][len(REFS_HEADS_PREFIX):]
  go.get(go.INFRA_GO+'/go/supported_branches/cmd/new-branch')
  subprocess.check_call(['new-branch',
                         '--branch', new_branch,
                         '--delete', old_branch,
                         '--owner', owner])


if __name__ == '__main__':
  main()
