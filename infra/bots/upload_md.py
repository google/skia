# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Update and upload markdown files using the output of fiddlecli."""


import argparse
import os
import subprocess
import sys

import git_utils


SKIA_REPO = 'https://skia.googlesource.com/skia.git'
COMMIT_MSG = '''Update markdown files

Automatic commit by the Housekeeper-Nightly-Bookmaker bot.

TBR=rmistry@google.com
NO_MERGE_BUILDS
'''
CC_LIST = ['rmistry@google.com', 'caryclark@google.com']


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("--bookmaker_binary")
  parser.add_argument("--fiddlecli_output")
  args = parser.parse_args()

  with git_utils.NewGitCheckout(repository=SKIA_REPO):
    with git_utils.GitBranch(branch_name='update_md_files',
                             commit_msg=COMMIT_MSG,
                             commit_queue=True,
                             upload=False,
                             cc_list=CC_LIST) as git_branch:
      # Run bookmaker binary.
      cmd = [args.bookmaker_binary,
             '-b', 'docs',
             '-f', args.fiddlecli_output,
             '-r', 'site/user/api',
             ]
      try:
        subprocess.check_call(cmd)
      except subprocess.CalledProcessError as e:
        print >> sys.stderr, (
            'Running %s failed, not uploading markdowns update:\n\n%s' % (
                cmd, e.output))
        sys.exit(1)

      # Verify that only files in the expected directory are going to be
      # committed and uploaded.
      diff_files = subprocess.check_output(['git', 'diff', '--name-only'])
      for diff_file in diff_files.split():
        if not diff_file.startswith('site/user/api/'):
          print >> sys.stderr, (
            'Some files in %s were not in the site/user/api dir. '
            'Not uploading them' % diff_files)
          sys.exit(1)
      if diff_files:
        subprocess.check_call(['git', 'add', '-u'])
        git_branch.commit_and_upload(True)
      else:
        print 'No changes so nothing to upload.'


if '__main__' == __name__:
  main()
