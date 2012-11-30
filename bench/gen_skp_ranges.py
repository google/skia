#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be found
# in the LICENSE file.

""" Analyze recent SkPicture bench data, and output suggested ranges.

The outputs can be edited and pasted to bench_expectations.txt to trigger
buildbot alerts if the actual benches are out of range. Details are documented
in the .txt file.

Currently the easiest way to update bench_expectations.txt is to delete all skp
bench lines, run this script, and redirect outputs (">>") to be added to the
.txt file.
TODO(bensong): find a better way for updating the bench lines in place.

Note: since input data are stored in Google Storage, you will need to set up
the corresponding library.
See http://developers.google.com/storage/docs/gspythonlibrary for details.
"""

__author__ = 'bensong@google.com (Ben Chen)'

import bench_util
import boto
import cStringIO
import optparse
import re
import shutil

from oauth2_plugin import oauth2_plugin


# Ratios for calculating suggested picture bench upper and lower bounds.
BENCH_UB = 1.1  # Allow for 10% room for normal variance on the up side.
BENCH_LB = 0.85

# Further allow for a fixed amount of noise. This is especially useful for
# benches of smaller absolute value. Keeping this value small will not affect
# performance tunings.
BENCH_ALLOWED_NOISE = 10

# List of platforms to track.
PLATFORMS = ['Mac_Float_Bench_32',
             'Nexus10_4-1_Float_Bench_32',
             'Shuttle_Ubuntu12_ATI5770_Float_Bench_32',
            ]

# Filter for configs of no interest. They are old config names replaced by more
# specific ones.
CONFIGS_TO_FILTER = ['gpu', 'raster']

# Template for gsutil uri.
GOOGLE_STORAGE_URI_SCHEME = 'gs'
URI_BUCKET = 'chromium-skia-gm'

# Constants for optparse.
USAGE_STRING = 'USAGE: %s [options]'
HOWTO_STRING = """
Feel free to revise PLATFORMS for your own needs. The default is the most common
combination that we care most about. Platforms that did not run bench_pictures
in the given revision range will not have corresponding outputs.
Please check http://go/skpbench to choose a range that fits your needs.
BENCH_UB, BENCH_LB and BENCH_ALLOWED_NOISE can be changed to expand or narrow
the permitted bench ranges without triggering buidbot alerts.
"""
HELP_STRING = """
Outputs expectation picture bench ranges for the latest revisions for the given
revision range. For instance, --rev_range=6000:6000 will return only bench
ranges for the bots that ran bench_pictures at rev 6000; --rev-range=6000:7000
may have multiple bench data points for each bench configuration, and the code
returns bench data for the latest revision of all available (closer to 7000).
""" + HOWTO_STRING

OPTION_REVISION_RANGE = '--rev-range'
OPTION_REVISION_RANGE_SHORT = '-r'
# Bench bench representation algorithm flag.
OPTION_REPRESENTATION_ALG = '--algorithm'
OPTION_REPRESENTATION_ALG_SHORT = '-a'

# List of valid representation algorithms.
REPRESENTATION_ALGS = ['avg', 'min', 'med', '25th']

def OutputSkpBenchExpectations(rev_min, rev_max, representation_alg):
  """Reads skp bench data from google storage, and outputs expectations.

  Ignores data with revisions outside [rev_min, rev_max] integer range. For
  bench data with multiple revisions, we use higher revisions to calculate
  expected bench values.
  Uses the provided representation_alg for calculating bench representations.
  """
  expectation_dic = {}
  uri = boto.storage_uri(URI_BUCKET, GOOGLE_STORAGE_URI_SCHEME)
  for obj in uri.get_bucket():
    # Filters out non-skp-bench files.
    if (not obj.name.startswith('perfdata/Skia_') or
        obj.name.find('_data_skp_') < 0):
      continue
    # Ignores uninterested platforms.
    platform = obj.name.split('/')[1][5:]  # Removes "Skia_" prefix.
    if platform not in PLATFORMS:
      continue
    # Filters by revision.
    for rev in range(rev_min, rev_max + 1):
      if '_r%s_' % rev not in obj.name:
        continue

    contents = cStringIO.StringIO()
    obj.get_file(contents)
    for point in bench_util.parse('', contents.getvalue().split('\n'),
                                  representation_alg):
      if point.config in CONFIGS_TO_FILTER:
        continue
      # TODO(bensong): the filtering below is only needed during skp generation
      # system transitioning. Change it once the new system (bench name starts
      # with http) is stable for the switch-over, and delete it once we
      # deprecate the old ones.
      if point.bench.startswith('http'):
        continue

      key = '%s_%s_%s,%s-%s' % (point.bench, point.config, point.time_type,
                                platform, representation_alg)
      # It is fine to have later revisions overwrite earlier benches, since we
      # only use the latest bench within revision range to set expectations.
      expectation_dic[key] = point.time
  keys = expectation_dic.keys()
  keys.sort()
  for key in keys:
    bench_val = expectation_dic[key]
    # Prints out expectation lines.
    print '%s,%.3f,%.3f,%.3f' % (key, bench_val,
                                 bench_val * BENCH_LB - BENCH_ALLOWED_NOISE,
                                 bench_val * BENCH_UB + BENCH_ALLOWED_NOISE)

def main():
  """Parses flags and outputs expected Skia picture bench results."""
  parser = optparse.OptionParser(USAGE_STRING % '%prog' + HELP_STRING)
  parser.add_option(OPTION_REVISION_RANGE_SHORT, OPTION_REVISION_RANGE,
      dest='rev_range',
      help='(Mandatory) revision range separated by ":", e.g., 6000:6005')
  parser.add_option(OPTION_REPRESENTATION_ALG_SHORT, OPTION_REPRESENTATION_ALG,
      dest='alg', default='25th',
      help=('Bench representation algorithm. One of '
            '%s. Default to "25th".' % str(REPRESENTATION_ALGS)))
  (options, args) = parser.parse_args()
  if options.rev_range:
    range_match = re.search('(\d+)\:(\d+)', options.rev_range)
    if not range_match:
      parser.error('Wrong format for rev-range [%s]' % options.rev_range)
    else:
      rev_min = int(range_match.group(1))
      rev_max = int(range_match.group(2))
      OutputSkpBenchExpectations(rev_min, rev_max, options.alg)
  else:
    parser.error('Please provide mandatory flag %s' % OPTION_REVISION_RANGE)


if '__main__' == __name__:
  main()
