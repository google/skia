#!/usr/bin/env python
# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

""" Generate bench_expectations file from a given set of bench data files. """

import argparse
import bench_util
import os
import re
import sys

# Parameters for calculating bench ranges.
RANGE_RATIO_UPPER = 1.5  # Ratio of range for upper bounds.
RANGE_RATIO_LOWER = 2.0  # Ratio of range for lower bounds.
ERR_RATIO = 0.08  # Further widens the range by the ratio of average value.
ERR_UB = 1.0  # Adds an absolute upper error to cope with small benches.
ERR_LB = 1.5

# List of bench configs to monitor. Ignore all other configs.
CONFIGS_TO_INCLUDE = ['simple_viewport_1000x1000',
                      'simple_viewport_1000x1000_gpu',
                      'simple_viewport_1000x1000_scalar_1.100000',
                      'simple_viewport_1000x1000_scalar_1.100000_gpu',
                     ]

# List of flaky entries that should be excluded. Each entry is defined by a list
# of 3 strings, corresponding to the substrings of [bench, config, builder] to
# search for. A bench expectations line is excluded when each of the 3 strings
# in the list is a substring of the corresponding element of the given line. For
# instance, ['desk_yahooanswers', 'gpu', 'Ubuntu'] will skip expectation entries
# of SKP benchs whose name contains 'desk_yahooanswers' on all gpu-related
# configs of all Ubuntu builders.
ENTRIES_TO_EXCLUDE = [
                     ]


def compute_ranges(benches):
  """Given a list of bench numbers, calculate the alert range.

  Args:
    benches: a list of float bench values.

  Returns:
    a list of float [lower_bound, upper_bound].
  """
  minimum = min(benches)
  maximum = max(benches)
  diff = maximum - minimum
  avg = sum(benches) / len(benches)

  return [minimum - diff * RANGE_RATIO_LOWER - avg * ERR_RATIO - ERR_LB,
          maximum + diff * RANGE_RATIO_UPPER + avg * ERR_RATIO + ERR_UB]


def create_expectations_dict(revision_data_points, builder):
  """Convert list of bench data points into a dictionary of expectations data.

  Args:
    revision_data_points: a list of BenchDataPoint objects.
    builder: string of the corresponding buildbot builder name.

  Returns:
    a dictionary of this form:
        keys = tuple of (config, bench) strings.
        values = list of float [expected, lower_bound, upper_bound] for the key.
  """
  bench_dict = {}
  for point in revision_data_points:
    if (point.time_type or  # Not walltime which has time_type ''
        not point.config in CONFIGS_TO_INCLUDE):
      continue
    to_skip = False
    for bench_substr, config_substr, builder_substr in ENTRIES_TO_EXCLUDE:
      if (bench_substr in point.bench and config_substr in point.config and
          builder_substr in builder):
        to_skip = True
        break
    if to_skip:
      continue
    key = (point.config, point.bench)
    if key in bench_dict:
      raise Exception('Duplicate bench entry: ' + str(key))
    bench_dict[key] = [point.time] + compute_ranges(point.per_iter_time)

  return bench_dict


def main():
    """Reads bench data points, then calculate and export expectations.
    """
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-a', '--representation_alg', default='25th',
        help='bench representation algorithm to use, see bench_util.py.')
    parser.add_argument(
        '-b', '--builder', required=True,
        help='name of the builder whose bench ranges we are computing.')
    parser.add_argument(
        '-d', '--input_dir', required=True,
        help='a directory containing bench data files.')
    parser.add_argument(
        '-o', '--output_file', required=True,
        help='file path and name for storing the output bench expectations.')
    parser.add_argument(
        '-r', '--git_revision', required=True,
        help='the git hash to indicate the revision of input data to use.')
    args = parser.parse_args()

    builder = args.builder

    data_points = bench_util.parse_skp_bench_data(
        args.input_dir, args.git_revision, args.representation_alg)

    expectations_dict = create_expectations_dict(data_points, builder)

    out_lines = []
    keys = expectations_dict.keys()
    keys.sort()
    for (config, bench) in keys:
      (expected, lower_bound, upper_bound) = expectations_dict[(config, bench)]
      out_lines.append('%(bench)s_%(config)s_,%(builder)s-%(representation)s,'
          '%(expected)s,%(lower_bound)s,%(upper_bound)s' % {
              'bench': bench,
              'config': config,
              'builder': builder,
              'representation': args.representation_alg,
              'expected': expected,
              'lower_bound': lower_bound,
              'upper_bound': upper_bound})

    with open(args.output_file, 'w') as file_handle:
      file_handle.write('\n'.join(out_lines))


if __name__ == "__main__":
    main()
