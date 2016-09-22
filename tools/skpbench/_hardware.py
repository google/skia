# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import time

class HardwareException(Exception):
  def __init__(self, message, sleeptime=60):
    Exception.__init__(self, message)
    self.sleeptime = sleeptime

class Hardware:
  def __init__(self):
    self.kick_in_time = 0

  def __enter__(self):
    return self

  def __exit__(self, exception_type, exception_value, traceback):
    pass

  def sanity_check(self):
    '''Raises a HardwareException if any hardware state is not as expected.'''
    pass

  def sleep(self, sleeptime):
    '''Puts the hardware into a resting state for a fixed amount of time.'''
    time.sleep(sleeptime)
