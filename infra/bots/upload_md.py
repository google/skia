# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Upload updated markdown files."""


import argparse
import json
import os
import subprocess
import sys
import urllib2

import git_utils


SKIA_COMMITTER_EMAIL = 'update-docs@skia.org'
SKIA_COMMITTER_NAME = 'Update Markdown files'
SKIA_REPO = 'https://skia.googlesource.com/skia.git'
COMMIT_MSG = '''Update markdown files

Automatic commit by the Housekeeper-Weekly-Bookmaker bot.

TBR=%s
NO_MERGE_BUILDS
''' % SKIA_COMMITTER_EMAIL


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("--gitcookies")
  parser.add_argument("--bookmaker_binary")
  parser.add_argument("--fiddlecli_output")
  args = parser.parse_args()

  with git_utils.NewGitCheckout(repository=SKIA_REPO):
    config_dict = {
        'user.name': SKIA_COMMITTER_NAME,
        'user.email': SKIA_COMMITTER_EMAIL,
        'http.cookiefile': args.gitcookies,
    }
    # Skip GCE Auth in depot_tools/gerrit_utils.py. Use gitcookies instead.
    os.environ['SKIP_GCE_AUTH_FOR_GIT'] = 'True'
    os.environ['GIT_COOKIES_PATH'] = args.gitcookies

    with git_utils.GitLocalConfig(config_dict):
      with git_utils.GitBranch(branch_name='update_md_files',
                               commit_msg=COMMIT_MSG,
                               commit_queue=True):
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
              'Running %s failed, not uploading markdowns update:\n\n%s' % (cmd, e.output))
          sys.exit(1)
        subprocess.check_call(['git', 'add', '-u'])

if '__main__' == __name__:
  main()
