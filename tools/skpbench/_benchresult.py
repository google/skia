# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Parses an skpbench result from a line of output text."""

from __future__ import print_function
import re
import sys

class BenchResult:
  FLOAT_REGEX = '[-+]?(\d+(\.\d*)?|\.\d+)([eE][-+]?\d+)?'
  PATTERN = re.compile('^(?P<accum_pad> *)'
                       '(?P<accum>' + FLOAT_REGEX + ')'
                       '(?P<median_pad> +)'
                       '(?P<median>' + FLOAT_REGEX + ')'
                       '(?P<max_pad> +)'
                       '(?P<max>' + FLOAT_REGEX + ')'
                       '(?P<min_pad> +)'
                       '(?P<min>' + FLOAT_REGEX + ')'
                       '(?P<stddev_pad> +)'
                       '(?P<stddev>' + FLOAT_REGEX + '%)'
                       '(?P<samples_pad> +)'
                       '(?P<samples>\d+)'
                       '(?P<sample_ms_pad> +)'
                       '(?P<sample_ms>\d+)'
                       '(?P<clock_pad> +)'
                       '(?P<clock>[cg]pu)'
                       '(?P<metric_pad> +)'
                       '(?P<metric>ms|fps)'
                       '(?P<config_pad> +)'
                       '(?P<config>[^\s]+)'
                       '(?P<bench_pad> +)'
                       '(?P<bench>[^\s]+)$')

  @classmethod
  def match(cls, text):
    match = cls.PATTERN.search(text)
    return cls(match) if match else None

  def __init__(self, match):
    self.accum = float(match.group('accum'))
    self.median = float(match.group('median'))
    self.max = float(match.group('max'))
    self.min = float(match.group('min'))
    self.stddev = float(match.group('stddev')[:-1]) # Drop '%' sign.
    self.samples = int(match.group('samples'))
    self.sample_ms = int(match.group('sample_ms'))
    self.clock = match.group('clock')
    self.metric = match.group('metric')
    self.config = match.group('config')
    self.bench = match.group('bench')
    self._match = match

  def get_string(self, name):
    return self._match.group(name)

  def format(self, config_suffix=None):
    if not config_suffix or config_suffix == '':
      return self._match.group(0)
    else:
      values = list()
      for name in ['accum', 'median', 'max', 'min', 'stddev',
                   'samples', 'sample_ms', 'clock', 'metric', 'config']:
        values.append(self.get_string(name + '_pad'))
        values.append(self.get_string(name))
      values.append(config_suffix)
      bench_pad = self.get_string('bench_pad')
      values.append(bench_pad[min(len(config_suffix), len(bench_pad) - 1):])
      values.append(self.get_string('bench'))
      return ''.join(values)
