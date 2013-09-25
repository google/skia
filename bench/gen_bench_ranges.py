#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be found
# in the LICENSE file.

""" Analyze recent SkPicture or Microbench data, and output suggested ranges.

The outputs can be edited and pasted to bench_expectations_<builder>.txt to
trigger buildbot alerts if the actual benches are out of range. Details are
documented in the corresponding .txt file for each builder.

Currently the easiest way to batch update bench_expectations_<builder>.txt is to
delete all bench lines, run this script, and redirect outputs (">>") to be added
to the corresponding .txt file for each perf builder.
You can also just manually change a few lines of interest, of course.

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
BENCH_LB = 0.9

# Further allow for a fixed amount of noise. This is especially useful for
# benches of smaller absolute value. Keeping this value small will not affect
# performance tunings.
BENCH_ALLOWED_NOISE = 10

# Name prefix for benchmark builders.
BENCH_BUILDER_PREFIX = 'Perf-'

# List of platforms to track. Feel free to change it to meet your needs.
PLATFORMS = ['Perf-Mac10.8-MacMini4.1-GeForce320M-x86-Release',
             'Perf-Android-Nexus7-Tegra3-Arm7-Release',
             'Perf-Ubuntu12-ShuttleA-ATI5770-x86-Release',
             'Perf-Win7-ShuttleA-HD2000-x86-Release',
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
or benchmain in the given revision range will not have corresponding outputs.
Please check http://go/skpbench to choose a range that fits your needs.
BENCH_UB, BENCH_LB and BENCH_ALLOWED_NOISE can be changed to expand or narrow
the permitted bench ranges without triggering buidbot alerts.
"""
HELP_STRING = """
Outputs expectation picture bench ranges for the latest revisions for the given
revision range. For instance, --rev_range=6000:6000 will return only bench
ranges for the bots that ran benches at rev 6000; --rev-range=6000:7000
may have multiple bench data points for each bench configuration, and the code
returns bench data for the latest revision of all available (closer to 7000).
""" + HOWTO_STRING

OPTION_REVISION_RANGE = '--rev-range'
OPTION_REVISION_RANGE_SHORT = '-r'
# Bench representation algorithm flag.
OPTION_REPRESENTATION_ALG = '--algorithm'
OPTION_REPRESENTATION_ALG_SHORT = '-a'
# Bench type to examine. Either 'micro' or 'skp'.
OPTION_BENCH_TYPE = '--bench-type'
OPTION_BENCH_TYPE_SHORT = '-b'

# List of valid bench types.
BENCH_TYPES = ['micro', 'skp']
# List of valid representation algorithms.
REPRESENTATION_ALGS = ['avg', 'min', 'med', '25th']

def OutputBenchExpectations(bench_type, rev_min, rev_max, representation_alg):
  """Reads bench data from google storage, and outputs expectations.

  Ignores data with revisions outside [rev_min, rev_max] integer range. For
  bench data with multiple revisions, we use higher revisions to calculate
  expected bench values.
  bench_type is either 'micro' or 'skp', according to the flag '-b'.
  Uses the provided representation_alg for calculating bench representations.
  """
  if bench_type not in BENCH_TYPES:
    raise Exception('Not valid bench_type! (%s)' % BENCH_TYPES)
  expectation_dic = {}
  uri = boto.storage_uri(URI_BUCKET, GOOGLE_STORAGE_URI_SCHEME)
  for obj in uri.get_bucket():
    # Filters out non-bench files.
    if ((not obj.name.startswith('perfdata/%s' % BENCH_BUILDER_PREFIX) and
         not obj.name.startswith(
             'playback/perfdata/%s' % BENCH_BUILDER_PREFIX)) or
         obj.name.find('_data') < 0):
      continue
    if ((bench_type == 'micro' and obj.name.find('_data_skp_') > 0) or
        (bench_type == 'skp' and obj.name.find('_skp_') < 0)):
      # Skips wrong bench type.
      continue
    # Ignores uninterested platforms.
    platform = obj.name.split('/')[1]
    if not platform.startswith(BENCH_BUILDER_PREFIX):
      platform = obj.name.split('/')[2]
    if not platform.startswith(BENCH_BUILDER_PREFIX):
      continue  # Ignores non-platform object
    if platform not in PLATFORMS:
      continue
    # Filters by revision.
    to_filter = True
    for rev in range(rev_min, rev_max + 1):
      if '_r%s_' % rev in obj.name:
        to_filter = False
        break
    if to_filter:
      continue
    contents = cStringIO.StringIO()
    obj.get_file(contents)
    for point in bench_util.parse('', contents.getvalue().split('\n'),
                                  representation_alg):
      if point.config in CONFIGS_TO_FILTER:
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
  """Parses flags and outputs expected Skia bench results."""
  parser = optparse.OptionParser(USAGE_STRING % '%prog' + HELP_STRING)
  parser.add_option(OPTION_REVISION_RANGE_SHORT, OPTION_REVISION_RANGE,
      dest='rev_range',
      help='(Mandatory) revision range separated by ":", e.g., 6000:6005')
  parser.add_option(OPTION_BENCH_TYPE_SHORT, OPTION_BENCH_TYPE,
      dest='bench_type', default='skp',
      help=('Bench type, either "skp" or "micro". Default to "skp".'))
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
      OutputBenchExpectations(options.bench_type, rev_min, rev_max, options.alg)
  else:
    parser.error('Please provide mandatory flag %s' % OPTION_REVISION_RANGE)


if '__main__' == __name__:
  main()
