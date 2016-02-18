#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import default_flavor
import os
import subprocess
import time


"""Utils for running coverage tests."""


class CoverageFlavorUtils(default_flavor.DefaultFlavorUtils):
  def compile(self, target):
    """Build the given target."""
    cmd = [os.path.join(self._bot_info.skia_dir, 'tools',
                        'llvm_coverage_build'),
           target]
    self._bot_info.run(cmd)

  def step(self, cmd, **kwargs):
    """Run the given step through coverage."""
    # Slice out the 'key' and 'properties' arguments to be reused.
    key = []
    properties = []
    current = None
    for i in xrange(0, len(cmd)):
      if isinstance(cmd[i], basestring) and cmd[i] == '--key':
        current = key
      elif isinstance(cmd[i], basestring) and cmd[i] == '--properties':
        current = properties
      elif isinstance(cmd[i], basestring) and cmd[i].startswith('--'):
        current = None
      if current is not None:
        current.append(cmd[i])

    results_dir = self._bot_info.out_dir.join('coverage_results')
    self.create_clean_host_dir(results_dir)

    # Run DM under coverage.
    report_file_basename = '%s.cov' % self._bot_info.got_revision
    report_file = os.path.join(results_dir, report_file_basename)
    args = [
        'python',
        os.path.join(self._bot_info.skia_dir, 'tools', 'llvm_coverage_run.py'),
    ] + cmd + ['--outResultsFile', report_file]
    self._bot_info.run(args, **kwargs)

    # Generate nanobench-style JSON output from the coverage report.
    git_timestamp = subprocess.check_output(['git', 'log', '-n1',
        self._bot_info.got_revision, '--format=%%ci']).rstrip()
    nanobench_json = results_dir.join('nanobench_%s_%s.json' % (
        self._bot_info.got_revision, git_timestamp))
    line_by_line_basename = ('coverage_by_line_%s_%s.json' % (
        self._bot_info.got_revision, git_timestamp))
    line_by_line = results_dir.join(line_by_line_basename)
    args = [
        'python',
        os.path.join(self._bot_info.skia_dir, 'tools',
                     'parse_llvm_coverage.py'),
        '--report', report_file, '--nanobench', nanobench_json,
        '--linebyline', line_by_line]
    args.extend(key)
    args.extend(properties)
    self._bot_info.run(args)

    # Upload raw coverage data.
    now = time.utcnow()
    gs_json_path = '/'.join((
        str(now.year).zfill(4), str(now.month).zfill(2),
        str(now.day).zfill(2), str(now.hour).zfill(2),
        self._bot_info.name,
        str(self._bot_info.build_number)))
    if self._bot_info.is_trybot:
      gs_json_path = '/'.join(('trybot', gs_json_path,
                               str(self._bot_info.issue)))

    self._bot_info.gsutil_upload(
        'upload raw coverage data',
        source=report_file,
        bucket='skia-infra',
        dest='/'.join(('coverage-raw-v1', gs_json_path, report_file_basename)))

    # Upload nanobench JSON data.
    gsutil_path = self._bot_info.m.path['depot_tools'].join(
        'third_party', 'gsutil', 'gsutil')
    upload_args = [self._bot_info.name,
                   self._bot_info.m.properties['buildnumber'],
                   results_dir,
                   self._bot_info.got_revision, gsutil_path]
    if self._bot_info.is_trybot:
      upload_args.append(self._bot_info.m.properties['issue'])
    self._bot_info.run(
        self._bot_info.m.python,
        'upload nanobench coverage results',
        script=self._bot_info.resource('upload_bench_results.py'),
        args=upload_args,
        cwd=self._bot_info.m.path['checkout'],
        abort_on_failure=False,
        infra_step=True)

    # Upload line-by-line coverage data.
    self._bot_info.gsutil_upload(
        'upload line-by-line coverage data',
        source=line_by_line,
        bucket='skia-infra',
        dest='/'.join(('coverage-json-v1', gs_json_path,
                       line_by_line_basename)))

