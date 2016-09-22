# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from _hardware import Hardware
import sys
import time

class HardwareAndroid(Hardware):
  def __init__(self, adb):
    Hardware.__init__(self)
    self.kick_in_time = 5
    self._adb = adb
    self._is_root = self._adb.attempt_root()
    if self._is_root:
      self._adb.remount()
    self._initial_airplane_mode = None
    self._initial_location_providers = None
    self._initial_ASLR = None

  def __enter__(self):
    # turn on airplane mode.
    self._initial_airplane_mode = \
      self._adb.check('settings get global airplane_mode_on')
    self._adb.shell('settings put global airplane_mode_on 1')

    # disable GPS.
    self._initial_location_providers = \
      self._adb.check('settings get secure location_providers_allowed')
    self._initial_location_providers = \
      self._initial_location_providers.replace(',', ' ')
    self._adb.shell('''\
      for PROVIDER in %s; do
        settings put secure location_providers_allowed -$PROVIDER
      done''' % self._initial_location_providers)

    if self._is_root:
      # disable bluetooth, wifi, and mobile data.
      # TODO: can we query these initial values?
      self._adb.shell('''\
        service call bluetooth_manager 8 &&
        svc wifi disable &&
        svc data disable''')

      # kill the gui.
      self._adb.shell('''\
        setprop ctl.stop media &&
        setprop ctl.stop zygote &&
        setprop ctl.stop surfaceflinger &&
        setprop ctl.stop drm''')

      # disable ASLR.
      self._initial_ASLR = \
        self._adb.check('cat /proc/sys/kernel/randomize_va_space')
      self._adb.shell('echo 0 > /proc/sys/kernel/randomize_va_space')
    else:
      print("WARNING: no adb root access; results may be unreliable.",
            file=sys.stderr)

    return Hardware.__enter__(self)

  def __exit__(self, exception_type, exception_value, traceback):
    Hardware.__exit__(self, exception_type, exception_value, traceback)

    if self._is_root:
      # restore ASLR.
      self._adb.shell('echo %s > /proc/sys/kernel/randomize_va_space' %
                      self._initial_ASLR)

      # revive the gui.
      self._adb.shell('''\
        setprop ctl.start drm &&
        setprop ctl.start surfaceflinger &&
        setprop ctl.start zygote &&
        setprop ctl.start media''')

    # restore GPS (doesn't seem to work if we killed the gui).
    self._adb.shell('''\
      for PROVIDER in %s; do
        settings put secure location_providers_allowed +$PROVIDER
      done''' % self._initial_location_providers)

    # restore airplane mode (doesn't seem to work if we killed the gui).
    self._adb.shell('settings put global airplane_mode_on %s' %
                    self._initial_airplane_mode)

  def sanity_check(self):
    Hardware.sanity_check(self)

  def sleep(self, sleeptime):
    Hardware.sleep(self, sleeptime)
