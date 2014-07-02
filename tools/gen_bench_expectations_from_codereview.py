#!/usr/bin/python

# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Generate new bench expectations from results of trybots on a code review."""


import collections
import compare_codereview
import json
import os
import re
import shutil
import subprocess
import sys
import urllib2


BENCH_DATA_URL = 'gs://chromium-skia-gm/perfdata/%s/%s/bench_*_data_*'
BUILD_STATUS_SUCCESS = 0
BUILD_STATUS_WARNINGS = 1
CHECKOUT_PATH = os.path.realpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)), os.pardir))
TMP_BENCH_DATA_DIR = os.path.join(CHECKOUT_PATH, '.bench_data')


TryBuild = collections.namedtuple(
    'TryBuild', ['builder_name', 'build_number', 'is_finished', 'json_url'])


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
      build_num = None
      json_url = None
      if data.url:
        split_url = data.url.split('/')
        build_num = split_url[-1]
        split_url.insert(split_url.index('builders'), 'json')
        json_url = '/'.join(split_url)
      is_finished = (data.status not in ('pending', 'try-pending') and
                     build_num is not None)
      try_builds.append(TryBuild(builder_name=builder,
                                 build_number=build_num,
                                 is_finished=is_finished,
                                 json_url=json_url))
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
  subprocess.check_call(['gsutil', 'cp', '-R', url, dest_dir])


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


def _step_succeeded(try_build, step_name):
  """Return True if the given step succeeded and False otherwise.

  This function talks to the build master's JSON interface, which is slow.

  TODO(borenet): There are now a few places which talk to the master's JSON
  interface. Maybe it'd be worthwhile to create a module which does this.

  Args:
      try_build: TryBuild instance; the build we're concerned about.
      step_name: string; name of the step we're concerned about.
  """
  step_url = '/'.join((try_build.json_url, 'steps', step_name))
  step_data = json.load(urllib2.urlopen(step_url))
  # step_data['results'] may not be present if the step succeeded. If present,
  # it is a list whose first element is a result code, per the documentation:
  # http://docs.buildbot.net/latest/developer/results.html
  result = step_data.get('results', [BUILD_STATUS_SUCCESS])[0]
  if result in (BUILD_STATUS_SUCCESS, BUILD_STATUS_WARNINGS):
    return True
  return False


def gen_bench_expectations_from_codereview(codereview_url,
                                           error_on_unfinished=True,
                                           error_on_try_failure=True):
  """Generate bench expectations from a code review.

  Scans the given code review for Perf trybot runs. Downloads the results of
  finished trybots and uses them to generate new expectations for their
  waterfall counterparts.

  Args:
      url: string; URL of the code review.
      error_on_unfinished: bool; throw an error if any trybot has not finished.
      error_on_try_failure: bool; throw an error if any trybot failed an
          important step.
  """
  try_builds = find_all_builds(codereview_url)

  # Verify that all trybots have finished running.
  if error_on_unfinished and not _all_trybots_finished(try_builds):
    raise TrybotNotFinishedError('Not all trybots have finished.')

  failed_run = []
  failed_data_pull = []
  failed_gen_expectations = []

  # Don't even try to do anything if BenchPictures, PostBench, or
  # UploadBenchResults failed.
  for try_build in try_builds:
    for step in ('BenchPictures', 'PostBench', 'UploadBenchResults'):
      if not _step_succeeded(try_build, step):
        msg = '%s failed on %s!' % (step, try_build.builder_name)
        if error_on_try_failure:
          raise Exception(msg)
        print 'WARNING: %s Skipping.' % msg
        failed_run.append(try_build.builder_name)

  if os.path.isdir(TMP_BENCH_DATA_DIR):
    shutil.rmtree(TMP_BENCH_DATA_DIR)

  for try_build in try_builds:
    try_builder = try_build.builder_name

    # Even if we're not erroring out on try failures, we can't generate new
    # expectations for failed bots.
    if try_builder in failed_run:
      continue

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

