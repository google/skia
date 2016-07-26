# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import datetime
import default_flavor
import posixpath
import ssh_devices


"""Utils for running coverage tests."""


class CoverageFlavorUtils(default_flavor.DefaultFlavorUtils):

  def step(self, name, cmd, **kwargs):
    """Run the given step through coverage."""
    compile_target = 'dm'
    build_cmd = [self._skia_api.skia_dir.join('tools', 'llvm_coverage_build'),
                 compile_target]
    self._skia_api.run(self._skia_api.m.step,
                       'build %s' % compile_target,
                       cmd=build_cmd,
                       cwd=self._skia_api.m.path['checkout'])

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

    results_dir = self._skia_api.skia_out.join('coverage_results')
    self.create_clean_host_dir(results_dir)

    # Run DM under coverage.
    report_file_basename = '%s.cov' % self._skia_api.got_revision
    report_file = results_dir.join(report_file_basename)
    args = [
        'python',
        self._skia_api.skia_dir.join('tools', 'llvm_coverage_run.py'),
    ] + cmd + ['--outResultsFile', report_file]
    self._skia_api.run(self._skia_api.m.step, name=name, cmd=args,
                       cwd=self._skia_api.m.path['checkout'], **kwargs)

    # Generate nanobench-style JSON output from the coverage report.
    nanobench_json = results_dir.join('nanobench_%s.json' % (
        self._skia_api.got_revision))
    line_by_line_basename = ('coverage_by_line_%s.json' % (
        self._skia_api.got_revision))
    line_by_line = results_dir.join(line_by_line_basename)
    args = [
        'python',
        self._skia_api.skia_dir.join('tools', 'parse_llvm_coverage.py'),
        '--report', report_file, '--nanobench', nanobench_json,
        '--linebyline', line_by_line]
    args.extend(key)
    args.extend(properties)
    self._skia_api.run(
        self._skia_api.m.step,
        'Generate Coverage Data',
        cmd=args, cwd=self._skia_api.m.path['checkout'])

    # Copy files from results_dir into swarming_out_dir.
    for r in self._skia_api.m.file.listdir('results_dir', results_dir):
      self._skia_api.m.file.copy(
          'Copy to swarming out', results_dir.join(r),
          self._skia_api.swarming_out_dir)
