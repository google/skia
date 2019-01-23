# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Create a CL to update the SKP version."""


import argparse
import os
import subprocess
import sys
import urllib2

import git_utils


COMMIT_MSG = '''Update SKP version

Automatic commit by the RecreateSKPs bot.

TBR=rmistry@google.com
NO_MERGE_BUILDS
'''
SKIA_REPO = 'https://skia.googlesource.com/skia.git'


def main(target_dir):
  # We're going to sync a new, clean Skia checkout to upload the CL to update
  # the SKPs. However, we want to use the scripts from the current checkout,
  # in order to facilitate running this as a try job.
  infrabots_dir = os.path.dirname(os.path.realpath(__file__))
  upload_py = os.path.join(infrabots_dir, 'assets', 'skp', 'upload.py')

  with git_utils.NewGitCheckout(repository=SKIA_REPO):
    # First verify that there are no gen_tasks diffs.
    gen_tasks = os.path.join(os.getcwd(), 'infra', 'bots', 'gen_tasks.go')
    try:
      subprocess.check_call(['go', 'run', gen_tasks, '--test'])
    except subprocess.CalledProcessError as e:
      print >> sys.stderr, (
         'gen_tasks.go failed, not uploading SKP update:\n\n%s' % e.output)
      sys.exit(1)

    # Upload the new version, land the update CL as the recreate-skps user.
    with git_utils.GitBranch(branch_name='update_skp_version',
                             commit_msg=COMMIT_MSG,
                             commit_queue=True):
      upload_cmd = ['python', upload_py, '-t', target_dir]
      if args.chromium_path:
        chromium_revision = (
            subprocess.check_output(['git', 'rev-parse', 'HEAD']).rstrip())
        upload_cmd.extend([
            '--extra_tags', 'chromium_revision:%s' % chromium_revision])
      subprocess.check_call(upload_cmd)
      subprocess.check_call(['go', 'run', gen_tasks])
      subprocess.check_call([
          'git', 'add', os.path.join('infra', 'bots', 'tasks.json')])


if '__main__' == __name__:
  parser = argparse.ArgumentParser()
  parser.add_argument("--target_dir")
  parser.add_argument("--chromium_path")
  args = parser.parse_args()
  main(args.target_dir)
