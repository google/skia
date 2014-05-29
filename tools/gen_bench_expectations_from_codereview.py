#!/usr/bin/python

# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Generate new bench expectations from results of trybots on a code review."""


import collections
import compare_codereview
import os
import re
import shutil
import subprocess
import sys


BENCH_DATA_URL = 'gs://chromium-skia-gm/perfdata/%s/%s/*'
CHECKOUT_PATH = os.path.realpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)), os.pardir))
TMP_BENCH_DATA_DIR = os.path.join(CHECKOUT_PATH, '.bench_data')


TryBuild = collections.namedtuple(
    'TryBuild', ['builder_name', 'build_number', 'is_finished'])


def find_all_builds(codereview_url):
  """Finds and returns information about trybot runs for a code review.

  Args:
    codereview_url: URL of the codereview in question.

  Returns:
      List of NamedTuples: (builder_name, build_number, is_finished)
  """
  results = compare_codereview.CodeReviewHTMLParser().parse(codereview_url)
  try_builds = []
  for builder, data in results.iteritems():
    if builder.startswith('Perf'):
      build_num = data.url.split('/')[-1] if data.url else None
      is_finished = (data.status not in ('pending', 'try-pending') and
                     build_num is not None)
      try_builds.append(TryBuild(builder_name=builder,
                                 build_number=build_num,
                                 is_finished=is_finished))
  return try_builds


def _all_trybots_finished(try_builds):
  """Return True iff all of the given try jobs have finished.

  Args:
      try_builds: list of TryBuild instances.

  Returns:
      True if all of the given try jobs have finished, otherwise False.
  """
  for try_build in try_builds:
    if not try_build.is_finished:
      return False
  return True


def all_trybots_finished(codereview_url):
  """Return True iff all of the try jobs on the given codereview have finished.

  Args:
      codereview_url: string; URL of the codereview.

  Returns:
      True if all of the try jobs have finished, otherwise False.
  """
  return _all_trybots_finished(find_all_builds(codereview_url))


def get_bench_data(builder, build_num, dest_dir):
  """Download the bench data for the given builder at the given build_num.

  Args:
      builder: string; name of the builder.
      build_num: string; build number.
      dest_dir: string; destination directory for the bench data.
  """
  url = BENCH_DATA_URL % (builder, build_num)
  subprocess.check_call(['gsutil', 'cp', '-R', url, dest_dir],
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)


def find_revision_from_downloaded_data(dest_dir):
  """Finds the revision at which the downloaded data was generated.

  Args:
      dest_dir: string; directory holding the downloaded data.

  Returns:
      The revision (git commit hash) at which the downloaded data was
      generated, or None if no revision can be found.
  """
  for data_file in os.listdir(dest_dir):
    match = re.match('bench_(?P<revision>[0-9a-fA-F]{2,40})_data.*', data_file)
    if match:
      return match.group('revision')
  return None


class TrybotNotFinishedError(Exception):
  pass


def gen_bench_expectations_from_codereview(codereview_url,
                                           error_on_unfinished=True):
  """Generate bench expectations from a code review.

  Scans the given code review for Perf trybot runs. Downloads the results of
  finished trybots and uses them to generate new expectations for their
  waterfall counterparts.

  Args:
      url: string; URL of the code review.
      error_on_unfinished: bool; throw an error if any trybot has not finished.
  """
  try_builds = find_all_builds(codereview_url)

  # Verify that all trybots have finished running.
  if error_on_unfinished and not _all_trybots_finished(try_builds):
    raise TrybotNotFinishedError('Not all trybots have finished.')

  failed_data_pull = []
  failed_gen_expectations = []

  if os.path.isdir(TMP_BENCH_DATA_DIR):
    shutil.rmtree(TMP_BENCH_DATA_DIR)

  for try_build in try_builds:
    try_builder = try_build.builder_name
    builder = try_builder.replace('-Trybot', '')

    # Download the data.
    dest_dir = os.path.join(TMP_BENCH_DATA_DIR, builder)
    os.makedirs(dest_dir)
    try:
      get_bench_data(try_builder, try_build.build_number, dest_dir)
    except subprocess.CalledProcessError:
      failed_data_pull.append(try_builder)
      continue

    # Find the revision at which the data was generated.
    revision = find_revision_from_downloaded_data(dest_dir)
    if not revision:
      # If we can't find a revision, then something is wrong with the data we
      # downloaded. Skip this builder.
      failed_data_pull.append(try_builder)
      continue

    # Generate new expectations.
    output_file = os.path.join(CHECKOUT_PATH, 'expectations', 'bench',
                               'bench_expectations_%s.txt' % builder)
    try:
      subprocess.check_call(['python',
                             os.path.join(CHECKOUT_PATH, 'bench',
                                          'gen_bench_expectations.py'),
                             '-b', builder, '-o', output_file,
                             '-d', dest_dir, '-r', revision])
    except subprocess.CalledProcessError:
      failed_gen_expectations.append(builder)

  failure = ''
  if failed_data_pull:
    failure += 'Failed to load data for: %s\n\n' % ','.join(failed_data_pull)
  if failed_gen_expectations:
    failure += 'Failed to generate expectations for: %s\n\n' % ','.join(
        failed_gen_expectations)
  if failure:
    raise Exception(failure)


if __name__ == '__main__':
  gen_bench_expectations_from_codereview(sys.argv[1])

