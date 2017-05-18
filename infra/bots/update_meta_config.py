# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Update meta/config of the specified Skia repo."""


import argparse
import json
import os
import subprocess
import sys
import urllib2

import git_utils


SKIA_COMMITTER_EMAIL = 'update-meta-config@skia.org'
SKIA_COMMITTER_NAME = 'Update Meta Config'
SKIA_REPO_TEMPLATE = 'https://skia.googlesource.com/%s.git'

CQ_INCLUDE_CHROMIUM_TRYBOTS = [
    ('master.tryserver.blink', [
        'linux_trusty_blink_rel',
        'linux_trusty_blink_dbg',
    ]),
    ('master.tryserver.chromium.linux', [
        'linux_chromium_compile_dbg_ng',
        'linux_chromium_compile_rel_ng',
        'linux_chromium_dbg_ng',
        'linux_chromium_rel_ng',
        'linux_optional_gpu_tests_rel',
    ]),
    ('master.tryserver.chromium.mac', [
        'mac_chromium_compile_dbg_ng',
        'mac_chromium_compile_rel_ng',
        'mac_chromium_dbg_ng',
        'mac_chromium_rel_ng',
        'mac_optional_gpu_tests_rel',
    ]),
    ('master.tryserver.chromium.win', [
        'win_chromium_compile_dbg_ng',
        'win_chromium_compile_rel_ng',
        'win_chromium_dbg_ng',
        'win_chromium_rel_ng',
        'win_optional_gpu_tests_rel',
    ]),
    ('master.tryserver.chromium.android', [
        'android_compile_dbg',
        'android_compile_rel',
        'android_optional_gpu_tests_rel',
    ])
]


def addChromiumTrybots(f):
  for master, bots in CQ_INCLUDE_CHROMIUM_TRYBOTS:
    f.write('[bucket "%s"]\n' % master)
    for bot in bots:
      f.write('\tbuilder = %s\n' % bot)


def main(gitcookies, repo_name, tasks_json):
  skia_repo = SKIA_REPO_TEMPLATE % repo_name
  with git_utils.NewGitCheckout(repository=skia_repo):
    # Fetch and checkout the meta/config branch.
    subprocess.check_call(['git', 'fetch', skia_repo, 'refs/meta/config:cfg'])
    subprocess.check_call(['git', 'checkout', 'cfg'])

    # Create list of tryjobs from tasks_json.
    tryjobs = []
    with open(tasks_json) as tasks_json:
      data = json.load(tasks_json)
      for job in data['jobs'].keys():
        if not job.startswith('Upload-'):
          tryjobs.append(job)
    tryjobs.sort()

    # Write to buildbucket.config.
    buildbucket_config = os.path.join(os.getcwd(), 'buildbucket.config')
    with open(buildbucket_config, 'w') as f:

      if repo_name == 'skia':
        addChromiumTrybots(f)

      # Adding all Skia jobs.
      f.write('[bucket "skia.primary"]\n')
      for job in tryjobs:
        f.write('\tbuilder = ' + job + '\n')

    # Push the change as the update-meta-config user.
    config_dict = {
      'user.name': SKIA_COMMITTER_NAME,
      'user.email': SKIA_COMMITTER_EMAIL,
      'http.cookiefile': gitcookies,
    }
    with git_utils.GitLocalConfig(config_dict):
      subprocess.check_call(['git', 'add', 'buildbucket.config'])
      try:
        subprocess.check_call(
            ['git', 'commit', '-m', 'Update builders in buildbucket.config'])
      except subprocess.CalledProcessError:
        print 'No changes to buildbucket.config'
        return

      subprocess.check_call(['git', 'push', skia_repo, 'cfg:refs/meta/config'])


if '__main__' == __name__:
  parser = argparse.ArgumentParser()
  parser.add_argument("--gitcookies")
  parser.add_argument("--repo_name")
  parser.add_argument("--tasks_json")
  args = parser.parse_args()
  main(args.gitcookies, args.repo_name, args.tasks_json)
