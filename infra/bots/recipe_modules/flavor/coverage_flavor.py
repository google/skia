# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import datetime
import default_flavor
import posixpath


"""Utils for running coverage tests."""


class CoverageFlavorUtils(default_flavor.DefaultFlavorUtils):

  def step(self, name, cmd, **kwargs):
    """Run the given step through coverage."""
    compile_target = 'dm'
    build_cmd = [self.m.vars.skia_dir.join('tools', 'llvm_coverage_build'),
                 compile_target]
    build_env = kwargs.pop('env', {})
    # We have to use Clang 3.6 because earlier versions do not support the
    # compile flags we use and 3.7 and 3.8 hit asserts during compilation.
    build_env['CC'] = '/usr/bin/clang-3.6'
    build_env['CXX'] = '/usr/bin/clang++-3.6'
    build_env['GYP_DEFINES'] = (
        'skia_arch_type=x86_64 '
        'skia_clang_build=1 '
        'skia_gpu=0 '
        'skia_warnings_as_errors=0')
    self.m.step('build %s' % compile_target,
                cmd=build_cmd,
                cwd=self.m.path['checkout'],
                env=build_env,
                **kwargs)

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

    results_dir = self.m.vars.skia_out.join('coverage_results')
    self.create_clean_host_dir(results_dir)

    # Run DM under coverage.
    report_file_basename = '%s.cov' % self.m.vars.got_revision
    report_file = results_dir.join(report_file_basename)
    args = [
        'python',
        self.m.vars.skia_dir.join('tools', 'llvm_coverage_run.py'),
    ] + cmd + ['--outResultsFile', report_file]
    self.m.run(self.m.step, name=name, cmd=args,
               cwd=self.m.path['checkout'], **kwargs)

    # Generate nanobench-style JSON output from the coverage report.
    nanobench_json = results_dir.join('nanobench_%s.json' % (
        self.m.vars.got_revision))
    line_by_line_basename = ('coverage_by_line_%s.json' % (
        self.m.vars.got_revision))
    line_by_line = results_dir.join(line_by_line_basename)
    args = [
        'python',
        self.m.vars.skia_dir.join('tools', 'parse_llvm_coverage.py'),
        '--report', report_file, '--nanobench', nanobench_json,
        '--linebyline', line_by_line]
    args.extend(key)
    args.extend(properties)
    self.m.run(
        self.m.step,
        'Generate Coverage Data',
        cmd=args, cwd=self.m.path['checkout'])

    # Copy files from results_dir into swarming_out_dir.
    for r in self.m.file.listdir('results_dir', results_dir):
      self.m.file.copy(
          'Copy to swarming out', results_dir.join(r),
          self.m.vars.swarming_out_dir)
