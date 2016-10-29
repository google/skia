# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from _hardware import HardwareException, Expectation
from _hardware_android import HardwareAndroid

CPU_CLOCK_RATE = 1836000
GPU_EMC_PROFILE = '0c: core 921 MHz emc 1600 MHz a A d D *'
GPU_EMC_PROFILE_ID = '0c'

class HardwarePixelC(HardwareAndroid):
  def __init__(self, adb):
    HardwareAndroid.__init__(self, adb)

  def __enter__(self):
    self._lock_clocks()
    return HardwareAndroid.__enter__(self)

  def __exit__(self, exception_type, exception_value, exception_traceback):
    HardwareAndroid.__exit__(self, exception_type,
                             exception_value, exception_traceback)
    self._unlock_clocks()

  def _lock_clocks(self):
    if not self._is_root:
      return

    # lock cpu clocks.
    self._adb.shell('''\
      for N in $(seq 0 3); do
        echo userspace > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_governor
        echo %i > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_setspeed
      done''' % CPU_CLOCK_RATE)

    # lock gpu/emc clocks.
    self._adb.shell('''\
      chown root:root /sys/devices/57000000.gpu/pstate
      echo %s > /sys/devices/57000000.gpu/pstate''' % GPU_EMC_PROFILE_ID)

  def _unlock_clocks(self):
    if not self._is_root:
      return

    # unlock gpu/emc clocks.
    self._adb.shell('''\
      echo auto > /sys/devices/57000000.gpu/pstate
      chown system:system /sys/devices/57000000.gpu/pstate''')

    # unlock cpu clocks.
    self._adb.shell('''\
      for N in $(seq 0 3); do
        echo 0 > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_setspeed
        echo interactive > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_governor
      done''')

  def sanity_check(self):
    HardwareAndroid.sanity_check(self)

    if not self._is_root:
      return

    # only issue one shell command in an attempt to minimize interference.
    result = self._adb.check_lines('''\
      cat /sys/class/power_supply/bq27742-0/capacity \
          /sys/class/thermal/thermal_zone7/temp \
          /sys/class/thermal/thermal_zone0/temp \
          /sys/class/thermal/thermal_zone1/temp \
          /sys/class/thermal/thermal_zone7/cdev1/cur_state \
          /sys/class/thermal/thermal_zone7/cdev0/cur_state
      for N in $(seq 0 3); do
        cat /sys/devices/system/cpu/cpu$N/cpufreq/scaling_cur_freq
      done
      cat /sys/devices/57000000.gpu/pstate | grep \*$''')

    expectations = \
      [Expectation(int, min_value=30, name='battery', sleeptime=30*60),
       Expectation(int, max_value=40000, name='skin temperature'),
       Expectation(int, max_value=86000, name='cpu temperature'),
       Expectation(int, max_value=87000, name='gpu temperature'),
       Expectation(int, exact_value=0, name='cpu throttle'),
       Expectation(int, exact_value=0, name='gpu throttle')] + \
      [Expectation(int, exact_value=CPU_CLOCK_RATE,
                   name='cpu_%i clock rate' % i, sleeptime=30)
       for i in range(4)] + \
      [Expectation(str, exact_value=GPU_EMC_PROFILE, name='gpu/emc profile')]

    Expectation.check_all(expectations, result)

  def sleep(self, sleeptime):
    self._unlock_clocks()
    HardwareAndroid.sleep(self, sleeptime)
    self._lock_clocks()
