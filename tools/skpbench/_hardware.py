# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import time

class Hardware:
  """Locks down and monitors hardware for benchmarking.

  This is a common base for classes that can control the specific hardware
  we are running on. Its purpose is to lock the hardware into a constant
  benchmarking mode for the duration of a 'with' block. e.g.:

    with hardware:
      run_benchmark()

  While benchmarking, the caller must call sanity_check() frequently to verify
  the hardware state has not changed.

  """

  def __init__(self):
    self.warmup_time = 0

  def __enter__(self):
    return self

  def __exit__(self, exception_type, exception_value, traceback):
    pass

  def filter_line(self, line):
    """Returns False if the provided output line can be suppressed."""
    return True

  def sanity_check(self):
    """Raises a HardwareException if any hardware state is not as expected."""
    pass

  def print_debug_diagnostics(self):
    """Prints any info that may help improve or debug hardware monitoring."""
    pass

  def sleep(self, sleeptime):
    """Puts the hardware into a resting state for a fixed amount of time."""
    time.sleep(sleeptime)


class HardwareException(Exception):
  """Gets thrown when certain hardware state is not what we expect.

  Generally this happens because of thermal conditions or other variables beyond
  our control, and the appropriate course of action is to take a short nap
  before resuming the benchmark.

  """

  def __init__(self, message, sleeptime=60):
    Exception.__init__(self, message)
    self.sleeptime = sleeptime


class Expectation:
  """Simple helper for checking the readings on hardware gauges."""
  def __init__(self, value_type, min_value=None, max_value=None,
               exact_value=None, name=None, sleeptime=60):
    self.value_type = value_type
    self.min_value = min_value
    self.max_value = max_value
    self.exact_value = exact_value
    self.name = name
    self.sleeptime = sleeptime

  def check(self, stringvalue):
    typedvalue = self.value_type(stringvalue)
    if self.min_value is not None and typedvalue < self.min_value:
       raise HardwareException("%s is too low (%s, min=%s)" %
                               (self.name, stringvalue, str(self.min_value)),
                               sleeptime=self.sleeptime)
    if self.max_value is not None and typedvalue > self.max_value:
       raise HardwareException("%s is too high (%s, max=%s)" %
                               (self.name, stringvalue, str(self.max_value)),
                               sleeptime=self.sleeptime)
    if self.exact_value is not None and typedvalue != self.exact_value:
       raise HardwareException("unexpected %s (%s, expected=%s)" %
                               (self.name, stringvalue, str(self.exact_value)),
                               sleeptime=self.sleeptime)

  @staticmethod
  def check_all(expectations, stringvalues):
    if len(stringvalues) != len(expectations):
      raise Exception("unexpected reading from hardware gauges "
                      "(expected %i values):\n%s" %
                      (len(expectations), '\n'.join(stringvalues)))

    for value, expected in zip(stringvalues, expectations):
      expected.check(value)
