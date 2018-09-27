# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create a CL to update the go_deps asset version."""


import os
import subprocess
import sys

import git_utils


COMMIT_MSG = '''Update go_deps asset

Automatic commit by the UpdateGoDEPS bot.

TBR=borenet@google.com
'''
SKIA_REPO = 'https://skia.googlesource.com/skia.git'


def main():
  with git_utils.NewGitCheckout(repository=SKIA_REPO):
    # First verify that there are no gen_tasks diffs.
    gen_tasks = os.path.join(os.getcwd(), 'infra', 'bots', 'gen_tasks.go')
    try:
      subprocess.check_call(['go', 'run', gen_tasks, '--test'])
    except subprocess.CalledProcessError as e:
      print >> sys.stderr, (
         'gen_tasks.go failed, not updating Go DEPS:\n\n%s' % e.output)
      sys.exit(1)

    # Upload the new version, land the update CL as the update-go-deps user.
    with git_utils.GitBranch(branch_name='update_go_deps_version',
                             commit_msg=COMMIT_MSG,
                             commit_queue=True):
      script = os.path.join(
          os.getcwd(), 'infra', 'bots', 'assets', 'go_deps',
          'create_and_upload.py')
      subprocess.check_call(['python', script])
      subprocess.check_call(['go', 'run', gen_tasks])
      subprocess.check_call([
          'git', 'add', os.path.join('infra', 'bots', 'tasks.json')])


if '__main__' == __name__:
  main()
