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

CQ_INCLUDE_CHROMIUM_BUCKETS = [
    'luci.chromium.try',
]


def addChromiumBuckets(f):
  for bucket in CQ_INCLUDE_CHROMIUM_BUCKETS:
    f.write('[bucket "%s"]\n' % bucket)


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
        addChromiumBuckets(f)

      # Adding all Skia jobs.
      f.write('[bucket "luci.skia.skia.primary"]\n')
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
