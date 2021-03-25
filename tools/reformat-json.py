#!/usr/bin/python


'''
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

'''
Rewrites a JSON file to use Python's standard JSON pretty-print format,
so that subsequent runs of rebaseline.py will generate useful diffs
(only the actual checksum differences will show up as diffs, not obscured
by format differences).

Should not modify the JSON contents in any meaningful way.
'''


# System-level imports
from __future__ import print_function
import argparse
import os
import sys


# Imports from within Skia
#
# We need to add the 'gm' directory, so that we can import gm_json.py within
# that directory.  That script allows us to parse the actual-results.json file
# written out by the GM tool.
# Make sure that the 'gm' dir is in the PYTHONPATH, but add it at the *end*
# so any dirs that are already in the PYTHONPATH will be preferred.
#
# This assumes that the 'gm' directory has been checked out as a sibling of
# the 'tools' directory containing this script, which will be the case if
# 'trunk' was checked out as a single unit.
GM_DIRECTORY = os.path.realpath(
    os.path.join(os.path.dirname(os.path.dirname(__file__)), 'gm'))
if GM_DIRECTORY not in sys.path:
    sys.path.append(GM_DIRECTORY)
import gm_json

def Reformat(filename):
  print('Reformatting file %s...' % filename)
  gm_json.WriteToFile(gm_json.LoadFromFile(filename), filename)

def _Main():
  parser = argparse.ArgumentParser(description='Reformat JSON files in-place.')
  parser.add_argument('filenames', metavar='FILENAME', nargs='+',
                      help='file to reformat')
  args = parser.parse_args()
  for filename in args.filenames:
    Reformat(filename)
  sys.exit(0)

if __name__ == '__main__':
    _Main()
