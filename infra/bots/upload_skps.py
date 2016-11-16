# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Create a CL to update the SKP version."""


import argparse
import os
import subprocess
import sys

sys.path.insert(0, os.path.join(os.getcwd(), 'common'))
# pylint:disable=F0401
from py.utils import git_utils


# rmistry
CHROMIUM_SKIA = 'https://chromium.googlesource.com/skia.git'
SKIA_COMMITTER_EMAIL = 'update-skps@skia.org'
SKIA_COMMITTER_NAME = 'UpdateSKPs'
COMMIT_MSG = '''Update SKP version

Automatic commit by the RecreateSKPs bot.

TBR=%s
NO_MERGE_BUILDS
''' % SKIA_COMMITTER_EMAIL
SKIA_REPO = 'https://skia.googlesource.com/skia.git'
GIT_COOKIES_DIR = '/tmp/creds/'
GIT_COOKIES_LOCATION = '/tmp/creds/.gitcookies'


def main(target_dir):
  # subprocess.check_call(['git', 'config', '--local', 'user.name',
  #                        SKIA_COMMITTER_NAME])
  # subprocess.check_call(['git', 'config', '--local', 'user.email',
  #                        SKIA_COMMITTER_EMAIL])
  # subprocess.check_call(['git', 'config', '--local', 'http.cookiefile',
  #                        GIT_COOKIES_LOCATION])
  if CHROMIUM_SKIA in subprocess.check_output(['git', 'remote', '-v']):
    subprocess.check_call(['git', 'remote', 'set-url', 'origin', SKIA_REPO,
                           CHROMIUM_SKIA])

  # TODO(rmistry): Experimenting starts.
  # TODO(rmistry): Documentation on why we need this.
  # TODO(rmistry): Delete GIT_COOKIES_DIR at the end.
  # Set HOME env variable to point to the git cookies dir. We could use
  # 'git config --local http.cookiefile' instead but th
  # os.environ['HOME'] = GIT_COOKIES_DIR
  config_dict = {
    'user.name': SKIA_COMMITTER_NAME,
    'user.email': SKIA_COMMITTER_EMAIL,
    'http.cookiefile': GIT_COOKIES_LOCATION,
  }
  with git_utils.GitLocalConfig(config_dict):
    with git_utils.GitBranch(branch_name='update_skp_version',
                             commit_msg=COMMIT_MSG,
                             commit_queue=False):  # TODO(rmistry): Change to True
      pass
  return
  # TODO(rmistry): Experimenting ends.

  # Download CIPD.
  cipd_sha1 = os.path.join(os.getcwd(), 'infra', 'bots', 'tools', 'luci-go',
                           'linux64', 'cipd.sha1')
  subprocess.check_call(['download_from_google_storage', '-s', cipd_sha1,
                         '--bucket', 'chromium-luci'])

  # First verify that there are no gen_tasks diffs.
  gen_tasks = os.path.join(os.getcwd(), 'infra', 'bots', 'gen_tasks.go')
  try:
    subprocess.check_call(['go', 'run', gen_tasks, '--test'])
  except subprocess.CalledProcessError as e:
    print >> sys.stderr, ('gen_tasks.go failed, not uploading SKP update:\n\n%s'
                          % e.output)
    sys.exit(1)

  # Upload the new version, land the update CL.
  with git_utils.GitBranch(branch_name='update_skp_version',
                           commit_msg=COMMIT_MSG,
                           commit_queue=True):
    upload_script = os.path.join(
        os.getcwd(), 'infra', 'bots', 'assets', 'skp', 'upload.py')
    subprocess.check_call(['python', upload_script, '-t', target_dir])
    subprocess.check_call(['go', 'run', gen_tasks])
    subprocess.check_call([
        'git', 'add', os.path.join('infra', 'bots', 'tasks.json')])


if '__main__' == __name__:
  parser = argparse.ArgumentParser()
  parser.add_argument("--target_dir")
  args = parser.parse_args()
  main(args.target_dir)
