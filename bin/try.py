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


BUCKET = 'skia.primary'
CHECKOUT_ROOT = os.path.realpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)), os.pardir))
JOBS_JSON = os.path.join(CHECKOUT_ROOT, 'infra', 'bots', 'jobs.json')


def main():
  # Parse arguments.
  d = 'Helper script for triggering try jobs defined in %s.' % JOBS_JSON
  parser = argparse.ArgumentParser(description=d)
  parser.add_argument('--list', action='store_true', default=False,
                      help='Just list the jobs; do not trigger anything.')
  parser.add_argument('job', nargs='?', default=None,
                      help='Job name or regular expression to match job names.')
  args = parser.parse_args()

  # Load and filter the list of jobs.
  with open(JOBS_JSON) as f:
    jobs = json.load(f)
  if args.job:
    jobs = [j for j in jobs if re.search(args.job, j)]

  # Display the list of jobs.
  if len(jobs) == 0:
    print 'Found no jobs matching "%s"' % repr(args.job)
    sys.exit(1)
  print 'Found %d jobs:' % len(jobs)
  for j in jobs:
    print '  %s' % j
  if args.list:
    return

  # Prompt before triggering jobs.
  resp = raw_input('\nDo you want to trigger these jobs? (y/n) ')
  if resp != 'y':
    sys.exit(1)

  # Trigger the try jobs.
  cmd = ['git', 'cl', 'try', '-B', BUCKET]
  for j in jobs:
    cmd.extend(['-b', j])
  try:
    subprocess.check_call(cmd)
  except subprocess.CalledProcessError:
    # Output from the command will fall through, so just exit here rather than
    # printing a stack trace.
    sys.exit(1)


if __name__ == '__main__':
  main()
