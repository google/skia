#!/usr/bin/python
# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Find and display recent changes in the given GM.

Example usage:

$ python gm/show_gm_changes.py Test-Mac10.7-MacMini4.1-GeForce320M-x86-Debug \
shadertext_gpu --autogen-path .gm-actuals

Rev    Hash
15990  10904734222736193002
10729  10752292282035416719
8504   2915063876615374518
71     7546128203733045901
"""


import argparse
import json
import os
import re
import subprocess
import sys


def _get_hash_and_last_change(gm_name, filepath):
  """Find the current hash for the given GM and the last-changed revision.

  This function runs "svn blame", which is slow.

  Args:
      gm_name: string; name of the GM in question.
      filepath: string; path to the actual-results.json file.
  Returns:
      tuple of the form (last_changed_rev, hash), where last_changed_rev is an
      int and hash is a string, or (None, None) if the file does not exist, the
      GM is not found in the file, or some other problem occurs. 
  """
  if not os.path.isfile(filepath):
    # If the file doesn't exist, we may have synced to before it was created.
    return (None, None)
  output = subprocess.check_output(['svn', 'blame', '--force', filepath])
  pattern = (r'^\s+\d+\s+.+\s+"%s.png" : {\s*\n\s+\d+\s+.+\s+"allowed-digests" '
             ': \[\s*\n\s+(\d+)\s+.+\s+\[ "bitmap-64bitMD5",\s+\n*(\d+)')
  match = re.search(pattern % gm_name, output, re.MULTILINE)
  if match:
    try:
      return (int(match.groups()[0]), match.groups()[1])
    except Exception:
      # If there are any problems with the above (incorrect number of matches,
      # inability to parse an integer), just return None.
      return (None, None)
  return (None, None)


def find_changes(builder_name, gm_name, autogen_path):
  """Find and return recent changes in the given GM.

  This function runs "svn blame" and "svn update" numerous times and is
  therefore very slow.

  Args:
      builder_name: string; name of the builder.
      gm_name: string; name of the GM.
      autogen_path: string; path to skia-autogen checkout.
  Yields:
      tuples of the form: (autogen_revision, hash)
  """
  actuals_path = os.path.join(autogen_path, builder_name, 'actual-results.json')

  # Capture the initial state of the skia-autogen checkout so that we can return
  # to the same state later.
  orig_rev = subprocess.check_output(['svnversion', '.'],
                                     cwd=autogen_path).rstrip()

  try:
    last_change_rev, hash = _get_hash_and_last_change(gm_name, actuals_path)
    while last_change_rev:
      yield (str(last_change_rev), hash)
      # Sync to the revision just *before* the last change
      subprocess.check_call(['svn', 'update', '-r', str(last_change_rev - 1)],
                            cwd=autogen_path,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
      last_change_rev, hash = _get_hash_and_last_change(gm_name, actuals_path)
  finally:
    # Return the repository to its initial state.
    subprocess.check_call(['svn', 'update', '-r', orig_rev],
                          cwd=autogen_path,
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE)


def main():
  """Find and display recent changes in the given GM."""
  parser = argparse.ArgumentParser(description=sys.modules[__name__].__doc__)
  parser.add_argument('builder_name', help='Name of the builder.')
  parser.add_argument('gm_name', help='Name of the GM.')
  parser.add_argument('--autogen-path', default=os.curdir,
                      help=('Path to a skia-autogen checkout. This checkout '
                            'will be modified but the script will attempt to '
                            'restore it to its original state. Default: '
                            '"%(default)s"'))
  args = parser.parse_args()

  print 'Rev\tHash'
  for change in find_changes(args.builder_name, args.gm_name,
                             args.autogen_path):
    print '\t'.join(change)
  

if __name__ == '__main__':
  sys.exit(main())
