#!/usr/bin/env python

# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Submit one or more try jobs."""


import argparse
import json
import os
import re
import subprocess
import sys
import tempfile


BUCKET_SKIA_PRIMARY = 'skia/skia.primary'
BUCKET_SKIA_INTERNAL = 'skia-internal/skia.internal'
INFRA_BOTS = os.path.join('infra', 'bots')
TASKS_JSON = os.path.join(INFRA_BOTS, 'tasks.json')
REPO_INTERNAL = 'https://skia.googlesource.com/internal_test.git'
TMP_DIR = os.path.join(tempfile.gettempdir(), 'sktry')

SKIA_ROOT = os.path.realpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)), os.pardir))
SKIA_INFRA_BOTS = os.path.join(SKIA_ROOT, INFRA_BOTS)
sys.path.insert(0, SKIA_INFRA_BOTS)

import utils


def find_repo_root():
  """Find the root directory of the current repository."""
  cwd = os.getcwd()
  while True:
    if os.path.isdir(os.path.join(cwd, '.git')):
      return cwd
    next_cwd = os.path.dirname(cwd)
    if next_cwd == cwd:
      raise Exception('Failed to find repo root!')
    cwd = next_cwd


def get_jobs(repo):
  """Obtain the list of jobs from the given repo."""
  # Maintain a copy of the repo in the temp dir.
  if not os.path.isdir(TMP_DIR):
    os.mkdir(TMP_DIR)
  with utils.chdir(TMP_DIR):
    dirname = repo.split('/')[-1]
    if not os.path.isdir(dirname):
      subprocess.check_call([
          utils.GIT, 'clone', '--mirror', repo, dirname])
    with utils.chdir(dirname):
      subprocess.check_call([utils.GIT, 'remote', 'update'])
      jobs = json.loads(subprocess.check_output([
          utils.GIT, 'show', 'master:%s' % JOBS_JSON]))
      return (BUCKET_SKIA_INTERNAL, jobs)


def main():
  # Parse arguments.
  d = 'Helper script for triggering try jobs.'
  parser = argparse.ArgumentParser(description=d)
  parser.add_argument('--list', action='store_true', default=False,
                      help='Just list the jobs; do not trigger anything.')
  parser.add_argument('--internal', action='store_true', default=False,
                      help=('If set, include internal jobs. You must have '
                            'permission to view internal repos.'))
  parser.add_argument('job', nargs='?', default=None,
                      help='Job name or regular expression to match job names.')
  args = parser.parse_args()

  # Load and filter the list of jobs.
  jobs = []
  tasks_json = os.path.join(find_repo_root(), TASKS_JSON)
  with open(tasks_json) as f:
    tasks_cfg = json.load(f)
  skia_primary_jobs = []
  for k, v in tasks_cfg['jobs'].iteritems():
    skia_primary_jobs.append(k)
  skia_primary_jobs.sort()

  # TODO(borenet): This assumes that the current repo is associated with the
  # skia.primary bucket. This will work for most repos but it would be better to
  # look up the correct bucket to use.
  jobs.append((BUCKET_SKIA_PRIMARY, skia_primary_jobs))
  if args.internal:
    jobs.append(get_jobs(REPO_INTERNAL))
  if args.job:
    filtered_jobs = []
    for bucket, job_list in jobs:
      filtered = [j for j in job_list if re.search(args.job, j)]
      if len(filtered) > 0:
        filtered_jobs.append((bucket, filtered))
    jobs = filtered_jobs

  # Display the list of jobs.
  if len(jobs) == 0:
    print 'Found no jobs matching "%s"' % repr(args.job)
    sys.exit(1)
  count = 0
  for bucket, job_list in jobs:
    count += len(job_list)
  print 'Found %d jobs:' % count
  for bucket, job_list in jobs:
    print '  %s:' % bucket
    for j in job_list:
      print '    %s' % j
  if args.list:
    return

  if count > 1:
    # Prompt before triggering jobs.
    resp = raw_input('\nDo you want to trigger these jobs? (y/n or i for '
                     'interactive): ')
    print ''
    if resp != 'y' and resp != 'i':
      sys.exit(1)
    if resp == 'i':
      filtered_jobs = []
      for bucket, job_list in jobs:
        new_job_list = []
        for j in job_list:
          incl = raw_input(('Trigger %s? (y/n): ' % j))
          if incl == 'y':
            new_job_list.append(j)
        if len(new_job_list) > 0:
          filtered_jobs.append((bucket, new_job_list))
      jobs = filtered_jobs

  # Trigger the try jobs.
  for bucket, job_list in jobs:
    cmd = ['git', 'cl', 'try', '-B', bucket]
    for j in job_list:
      cmd.extend(['-b', j])
    try:
      subprocess.check_call(cmd)
    except subprocess.CalledProcessError:
      # Output from the command will fall through, so just exit here rather than
      # printing a stack trace.
      sys.exit(1)


if __name__ == '__main__':
  main()
