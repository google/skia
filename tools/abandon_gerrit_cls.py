#!/usr/bin/env python
#
# Copyright 2020 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Bulk abandon Gerrit CLs."""


import argparse
import os
import re
import subprocess
import sys

from infra import git
from infra import go


def run_abandon_cls(args):
  """Bulk abandon Gerrit CLs."""
  go.mod_download()
  go.install(go.INFRA_GO+'/scripts/abandon_gerrit_cls')
  subprocess.check_call([
      'abandon_gerrit_cls',
      '--gerrit_instance', args.gerrit_instance,
      '--abandon_reason', args.abandon_reason,
      '--last_modified_before_days', str(args.last_modified_before_days),
  ])


def main():
  # TODO(rmistry): Instead of attempting to keep these args in sync, defer to
  # abandon_gerrit_cls for argument parsing.
  d = 'Helper script for bulk abandoning gerrit CLs'
  parser = argparse.ArgumentParser(description=d)
  parser.add_argument(
      '--gerrit-instance', '-g', default='https://skia-review.googlesource.com',
      help='Name of the gerrit instance.')
  parser.add_argument(
      '--abandon-reason', '-a', default='',
      help='Will be used as reason for abandoning.')
  parser.add_argument(
      '--last-modified-before-days', '-l', default=0,
      help='If 3 is specified then all CLs that were modified after 3 days ago '
           'will be returned.')
  args = parser.parse_args()

  go.check()
  run_abandon_cls(args)


if __name__ == '__main__':
  main()
