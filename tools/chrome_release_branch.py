#!/usr/bin/env python
#
# Copyright 2019 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import os
import re
import subprocess
import sys

from infra import git
from infra import go

_TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.realpath(os.path.join(_TOOLS_DIR, os.pardir))
_INFRA_BOTS = os.path.join(_REPO_ROOT, 'infra', 'bots')
sys.path.insert(0, _INFRA_BOTS)
import git_utils


REFS_HEADS_PREFIX = 'refs/heads/'
CHROME_REF_PREFIX = REFS_HEADS_PREFIX + 'chrome/m'
SK_MILESTONE_H = os.path.join('include', 'core', 'SkMilestone.h')
SK_MILESTONE_TMPL = r'#define SK_MILESTONE %s'
SK_MILESTONE_RE = SK_MILESTONE_TMPL % r'(\d+)'
SKIA_REPO = 'https://skia.googlesource.com/skia.git'
SUPPORTED_CHROME_BRANCHES = 2  # Per infra policy; see skbug.com/8940
UPDATE_MILESTONE_COMMIT_MSG = '''Update Skia milestone to %d'''


def get_current_milestone():
  '''Read SkMilestone.h and parse out the current milestone.'''
  sk_milestone = os.path.join(_REPO_ROOT, SK_MILESTONE_H)
  with open(sk_milestone, 'r') as f:
    contents = f.read()
  for line in contents.splitlines():
    m = re.match(SK_MILESTONE_RE, line)
    if m:
      return int(m.groups()[0])
  print >> sys.stderr, (
      'Failed to parse %s; has the format changed?' % SK_MILESTONE_H)
  sys.exit(1)


def create_new_branch(new_branch, branch_at):
  '''Create a temporary checkout of the repo, create the new branch and push.'''
  b = new_branch[len(REFS_HEADS_PREFIX):]
  with git_utils.NewGitCheckout(SKIA_REPO, local=_REPO_ROOT):
    git.git('checkout', '-b', b)
    git.git('reset', '--hard', branch_at)
    git.git('push', '--set-upstream', 'origin', b)


def update_milestone(m):
  '''Update SkMilestone.h to match the given milestone number.'''
  with git_utils.NewGitCheckout(SKIA_REPO, local=_REPO_ROOT):
    with git_utils.GitBranch(
        'update_milestone', UPDATE_MILESTONE_COMMIT_MSG % m):
      with open(SK_MILESTONE_H, 'r+') as f:
        contents = re.sub(
            SK_MILESTONE_RE, SK_MILESTONE_TMPL % str(m), f.read(), flags=re.M)
        f.seek(0)
        f.write(contents)
        f.truncate()
      git.git('diff')


def update_infra_config(old_branch, new_branch):
  '''Create a CL to add infra support for the new branch and remove the old.'''
  owner = git.git('config', 'user.email').rstrip()
  if not owner:
    print >> sys.stderr, ('No configured git user; please run '
                          '"git config user.email <your email>".')
    sys.exit(1)
  go.get(go.INFRA_GO+'/go/supported_branches/cmd/new-branch')
  subprocess.check_call(['new-branch',
                         '--branch', new_branch[len(REFS_HEADS_PREFIX):],
                         '--delete', old_branch[len(REFS_HEADS_PREFIX):],
                         '--owner', owner,
                         '--exclude-trybots=chromium.*',
                         '--exclude-trybots=.*Android_Framework.*'])


def main():
  if len(sys.argv) != 2 or '--help' in sys.argv or '-h' in sys.argv:
    print >> sys.stderr, 'Usage: %s <commit hash for branch>' % sys.argv[0]
    sys.exit(1)
  go.check()
  branch_at = sys.argv[1]
  m = get_current_milestone()
  new_branch = '%s%d' % (CHROME_REF_PREFIX, m)
  old_branch = '%s%d' % (CHROME_REF_PREFIX, m-SUPPORTED_CHROME_BRANCHES)
  print 'Creating branch %s and removing support (eg. CQ) for %s' % (
      new_branch, old_branch)
  create_new_branch(new_branch, branch_at)
  update_milestone(m+1)
  update_infra_config(old_branch, new_branch)


if __name__ == '__main__':
  main()
