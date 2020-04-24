#!/usr/bin/env python
#
# Copyright 2019 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Check the DEPS file for correctness."""


import os
import re
import subprocess
import sys

import utils


INFRA_BOTS_DIR = os.path.dirname(os.path.realpath(__file__))
SKIA_DIR = os.path.abspath(os.path.join(INFRA_BOTS_DIR, os.pardir, os.pardir))


def main():
  """Load the DEPS file and verify that all entries are valid."""
  # Find gclient.py and run that instead of simply "gclient", which calls into
  # update_depot_tools.
  gclient = subprocess.check_output([utils.WHICH, utils.GCLIENT])
  gclient_py = os.path.join(os.path.dirname(gclient), 'gclient.py')
  python = sys.executable or 'python'

  # Obtain the DEPS mapping.
  output = subprocess.check_output(
      [python, gclient_py, 'revinfo'], cwd=SKIA_DIR)

  # Check each entry.
  errs = []
  for e in output.rstrip().splitlines():
    split = e.split(': ')
    if len(split) != 2:
      errs.append(
          'Failed to parse `gclient revinfo` output; invalid format: %s' % e)
    if split[0] == 'skia':
      continue
    split = split[1].split('@')
    if len(split) != 2:
      errs.append(
          'Failed to parse `gclient revinfo` output; invalid format: %s' % e)
    repo = split[0]
    rev = split[1]
    if not 'googlesource.com' in repo:
      errs.append(
          'DEPS must be hosted on googlesource.com; %s is not allowed.' % repo)
    if not re.match(r'^[a-z0-9]{40}$', rev):
      errs.append('%s: "%s" does not look like a commit hash.' % (repo, rev))
  if errs:
    print >> sys.stderr, 'Found problems in DEPS:'
    for err in errs:
      print >> sys.stderr, err
    sys.exit(1)


if __name__ == '__main__':
  main()
