# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Create a CL to update the SKP version."""


import argparse
import os
import subprocess
import sys

import git_utils
import utils


COMMIT_MSG = '''Update SKP version

Automatic commit by the RecreateSKPs bot.

TBR=rmistry@google.com
NO_MERGE_BUILDS
'''


def main():
  # We're going to sync a new, clean Skia checkout to upload the CL to update
  # the SKPs. However, we want to use the scripts from the current checkout,
  # in order to facilitate running this as a try job.
  infrabots_dir = os.path.dirname(os.path.realpath(__file__))
  skp_dir = os.path.join(infrabots_dir, 'assets', 'skp')
  
  with git_utils.NewGitCheckout(repository=utils.SKIA_REPO):
    # First verify that there are no gen_tasks diffs.
    tmp_infrabots_dir = os.path.join(os.getcwd(), 'infra', 'bots')
    tmp_gen_tasks = os.path.join(tmp_infrabots_dir, 'gen_tasks.go')
    try:
      subprocess.check_call(['go', 'run', tmp_gen_tasks, '--test'])
    except subprocess.CalledProcessError as e:
      print >> sys.stderr, (
         'gen_tasks.go failed, not uploading SKP update:\n\n%s' % e.output)
      sys.exit(1)

    # Upload the new version, land the update CL as the recreate-skps user.
    with git_utils.GitBranch(branch_name='update_skp_version',
                             commit_msg=COMMIT_MSG,
                             commit_queue=True):
      # We used upload.py from the repo that this script lives in, NOT the temp
      # repo we've created. Therefore, the VERSION file was written in that repo
      # so we need to copy it to the temp repo in order to commit it.
      src = os.path.join(skp_dir, 'VERSION')
      dst = os.path.join(
          os.getcwd(), 'infra', 'bots', 'assets', 'skp', 'VERSION')
      subprocess.check_call(['cp', src, dst])
      subprocess.check_call(['go', 'run', tmp_gen_tasks])
      subprocess.check_call([
          'git', 'add', os.path.join('infra', 'bots', 'tasks.json')])


if '__main__' == __name__:
  main()
