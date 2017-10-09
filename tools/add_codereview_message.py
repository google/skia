#!/usr/bin/python2

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Add message to codereview issue.

This script takes a codereview issue number as its argument and a (possibly
multi-line) message on stdin.  It appends the message to the given issue.

Usage:
  echo MESSAGE | %prog CODEREVIEW_ISSUE
or:
  %prog CODEREVIEW_ISSUE <<EOF
  MESSAGE
  EOF
or:
  %prog --help
"""

import optparse
import sys

import fix_pythonpath  # pylint: disable=W0611
from common.py.utils import find_depot_tools  # pylint: disable=W0611
import rietveld


RIETVELD_URL = 'https://codereview.chromium.org'


def add_codereview_message(issue, message):
  """Add a message to a given codereview.

  Args:
      codereview_url: (string) we will extract the issue number from
          this url, or this could simply be the issue number.
      message: (string) message to add.
  """
  # Passing None for the email and auth_config will result in a prompt or
  # reuse of existing cached credentials.
  my_rietveld = rietveld.Rietveld(RIETVELD_URL, email=None, auth_config=None)

  my_rietveld.add_comment(issue, message)


def main(argv):
  """main function; see module-level docstring and GetOptionParser help.

  Args:
      argv: sys.argv[1:]-type argument list.
  """
  option_parser = optparse.OptionParser(usage=__doc__)
  _, arguments = option_parser.parse_args(argv)

  if len(arguments) > 1:
    option_parser.error('Extra arguments.')
  if len(arguments) != 1:
    option_parser.error('Missing issue number.')

  message = sys.stdin.read()
  add_codereview_message(int(arguments[0]), message)


if __name__ == '__main__':
  main(sys.argv[1:])

