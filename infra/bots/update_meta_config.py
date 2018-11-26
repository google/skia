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


SKIA_REPO_TEMPLATE = 'https://skia.googlesource.com/%s.git'

CQ_INCLUDE_CHROMIUM_TRYBOTS = [
    ('luci.chromium.try', [
        'android_optional_gpu_tests_rel',
        'linux-blink-rel',
        'linux_chromium_compile_dbg_ng',
        'linux_chromium_dbg_ng',
        'linux_chromium_rel_ng',
        'linux_optional_gpu_tests_rel',
        'mac10.10-blink-rel',
        'mac10.11-blink-rel',
        'mac10.12-blink-rel',
        'mac10.13-blink-rel',
        'mac10.13_retina-blink-rel',
        'mac_chromium_compile_dbg_ng',
        'mac_chromium_compile_rel_ng',
        'mac_chromium_dbg_ng',
        'mac_chromium_rel_ng',
        'mac_optional_gpu_tests_rel',
        'win10-blink-rel',
        'win7-blink-rel',
        'win_chromium_compile_dbg_ng',
        'win_chromium_dbg_ng',
        'win_optional_gpu_tests_rel',
    ]),
    ('master.tryserver.chromium.linux', [
        'linux_chromium_compile_rel_ng',
    ]),
    ('master.tryserver.chromium.win', [
        'win_chromium_compile_rel_ng',
        'win7_chromium_rel_ng',
        'win10_chromium_x64_rel_ng',
    ]),
    ('master.tryserver.chromium.android', [
        'android_blink_rel',
        'android_compile_dbg',
        'android_compile_rel',
        'android_n5x_swarming_dbg',
        'android_n5x_swarming_rel',
    ])
]


def addChromiumTrybots(f):
  for master, bots in CQ_INCLUDE_CHROMIUM_TRYBOTS:
    f.write('[bucket "%s"]\n' % master)
    for bot in bots:
      f.write('\tbuilder = %s\n' % bot)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument("--repo_name")
  parser.add_argument("--tasks_json")
  args = parser.parse_args()

  skia_repo = SKIA_REPO_TEMPLATE % args.repo_name
  with git_utils.NewGitCheckout(repository=skia_repo):
    # Fetch and checkout the meta/config branch.
    subprocess.check_call(['git', 'fetch', skia_repo, 'refs/meta/config:cfg'])
    subprocess.check_call(['git', 'checkout', 'cfg'])

    # Create list of tryjobs from tasks_json.
    tryjobs = []
    with open(args.tasks_json) as tasks_json:
      data = json.load(tasks_json)
      for job in data['jobs'].keys():
        if not job.startswith('Upload-'):
          tryjobs.append(job)
    tryjobs.sort()

    # Write to buildbucket.config.
    buildbucket_config = os.path.join(os.getcwd(), 'buildbucket.config')
    with open(buildbucket_config, 'w') as f:

      if args.repo_name == 'skia':
        addChromiumTrybots(f)

      # Adding all Skia jobs.
      f.write('[bucket "skia.primary"]\n')
      for job in tryjobs:
        f.write('\tbuilder = ' + job + '\n')

    subprocess.check_call(['git', 'add', 'buildbucket.config'])
    try:
      subprocess.check_call(
          ['git', 'commit', '-m', 'Update builders in buildbucket.config'])
    except subprocess.CalledProcessError:
      print 'No changes to buildbucket.config'
      return

    subprocess.check_call(['git', 'push', skia_repo, 'cfg:refs/meta/config'])


if '__main__' == __name__:
  main()
