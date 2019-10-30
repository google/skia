#!/usr/bin/python2
#
# Copyright 2019 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Helper script that takes as input 2 CSVs downloaded from perf.skia.org and
# outputs a CSV with test_name, avg_value1 (from CSV1), avg_value2 (from CSV2),
# perc_diff between avg_value1 and avg_value2.
# This script also discards NUM_OUTLIERS_TO_REMOVE min values and
# NUM_OUTLIERS_TO_REMOVE max values.


import csv
import optparse
import sys
import re


MISSING_STR = 'N/A'
NUM_OUTLIERS_TO_REMOVE = 2


def read_from_csv(csv_file):
  test_to_avg = {}
  with open(csv_file, 'rb') as f:
    csv_reader = csv.reader(f, delimiter=',')
    # First row should contain headers. Validate that it does.
    header_row = csv_reader.next()
    if header_row[0] != 'id':
      raise Exception('%s in unexpected format' % csv_file)
    p = re.compile('^.*,test=(.*),$')
    for v in csv_reader:
      # Extract the test name.
      result = p.search(v[0])
      test_name = result.group(1)

      vals = [float(i) for i in v[1:]]
      vals.sort()
      # Discard outliers.
      vals = vals[NUM_OUTLIERS_TO_REMOVE:-NUM_OUTLIERS_TO_REMOVE]
      # Find the avg val.
      avg_val = reduce(lambda x, y: x+y, vals) / float(len(vals))
      test_to_avg[test_name] = avg_val
  return test_to_avg


def combine_results(d1, d2):
  test_to_result = {}
  for test1, v1 in d1.items():
    v2 = d2.get(test1, MISSING_STR)
    perc_diff = MISSING_STR
    if v2 != MISSING_STR:
      diff = v2 - v1
      avg = (v2 + v1)/2
      perc_diff = 0 if avg == 0 else diff/avg * 100
    result = {
        'test_name': test1,
        'csv1': v1,
        'csv2': v2,
        'perc_diff': perc_diff,
    }
    test_to_result[test1] = result

  # Also add keys in d2 and not d1.
  for test2, v2 in d2.items():
    if test2 in test_to_result:
      continue
    test_to_result[test2] = {
      'test_name': test2,
      'csv1': MISSING_STR,
      'csv2': v2,
      'perc_diff': MISSING_STR,
    }

  return test_to_result


def write_to_csv(output_dict, output_csv):
  with open(output_csv, 'w') as f:
    fieldnames = ['test_name', 'csv1', 'csv2', 'perc_diff']
    writer = csv.DictWriter(f, fieldnames=fieldnames)
    writer.writeheader()
    tests = output_dict.keys()
    tests.sort()
    for test in tests:
      writer.writerow(output_dict[test])


def parse_and_output(csv1, csv2, output_csv):
  test_to_avg1 = read_from_csv(csv1)
  test_to_avg2 = read_from_csv(csv2)
  output_dict = combine_results(test_to_avg1, test_to_avg2)
  write_to_csv(output_dict, output_csv)


def main():
  option_parser = optparse.OptionParser()
  option_parser.add_option(
      '', '--csv1', type=str,
      help='The first CSV to parse.')
  option_parser.add_option(
      '', '--csv2', type=str,
      help='The second CSV to parse.')
  option_parser.add_option(
      '', '--output_csv', type=str,
      help='The file to write the output CSV to.')
  options, _ = option_parser.parse_args()
  sys.exit(parse_and_output(options.csv1, options.csv2, options.output_csv))


if __name__ == '__main__':
  main()
