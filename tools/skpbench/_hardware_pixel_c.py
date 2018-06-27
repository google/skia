# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from _hardware import HardwareException, Expectation
from _hardware_android import HardwareAndroid

CPU_CLOCK_RATE = 1326000
# If you run adb cat /sys/devices/57000000.gpu/pstate it shows all
# possible configurations, with a * next to the current one.
GPU_EMC_PROFILE = '04: core 307 MHz emc 1065 MHz a A d D *'
GPU_EMC_PROFILE_ID = '04'

class HardwarePixelC(HardwareAndroid):
  def __init__(self, adb):
    HardwareAndroid.__init__(self, adb)

  def __enter__(self):
    HardwareAndroid.__enter__(self)
    if not self._adb.is_root():
      return self

    self._adb.shell('\n'.join([
      # pylint: disable=line-too-long
      # Based on https://android.googlesource.com/platform/frameworks/base/+/master/libs/hwui/tests/scripts/prep_ryu.sh
      # All CPUs have the same scaling settings, so we only need to set it once
      '''
      stop thermal-engine
      stop perfd

      echo userspace > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
      echo %i > /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq
      echo %i > /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq
      echo %i > /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed
      ''' % tuple(CPU_CLOCK_RATE for _ in range(3)),
      # turn off the fourth core. This will hopefully produce less heat, allowing
      # for more consistent results.  3 cores should be enough to run Ganesh,
      # the graphics driver, and the OS.
      '''
      echo 0 > /sys/devices/system/cpu/cpu3/online''',

      # lock gpu/emc clocks.
      '''
      chown root:root /sys/devices/57000000.gpu/pstate
      echo %s > /sys/devices/57000000.gpu/pstate''' % GPU_EMC_PROFILE_ID]))

    return self

  def filter_line(self, line):
    JUNK = ['NvRmPrivGetChipPlatform: Could not read platform information',
            'Expected on kernels without fuse support, using silicon']
    return False if line in JUNK else HardwareAndroid.filter_line(self, line)

  def sanity_check(self):
    HardwareAndroid.sanity_check(self)

    if not self._adb.is_root():
      return

    # only issue one shell command in an attempt to minimize interference.
    result = self._adb.check('''\
      cat /sys/class/power_supply/bq27742-0/capacity \
          /sys/devices/system/cpu/online \
          /sys/class/thermal/thermal_zone7/temp \
          /sys/class/thermal/thermal_zone0/temp \
          /sys/class/thermal/thermal_zone1/temp \
          /sys/class/thermal/thermal_zone7/cdev1/cur_state \
          /sys/class/thermal/thermal_zone7/cdev0/cur_state
      for N in 0 1 2; do
        cat /sys/devices/system/cpu/cpu$N/cpufreq/scaling_cur_freq
      done
      cat /sys/devices/57000000.gpu/pstate | grep \*$''')

    expectations = \
      [Expectation(int, min_value=30, name='battery', sleeptime=30*60),
       Expectation(str, exact_value='0-2', name='online cpus'),
       Expectation(int, max_value=40000, name='skin temperature'),
       Expectation(int, max_value=86000, name='cpu temperature'),
       Expectation(int, max_value=87000, name='gpu temperature'),
       Expectation(int, exact_value=0, name='cpu throttle'),
       Expectation(int, exact_value=0, name='gpu throttle')] + \
      [Expectation(int, exact_value=CPU_CLOCK_RATE,
                   name='cpu_%i clock rate' % i, sleeptime=30)
       for i in (0, 1, 2)] + \
      [Expectation(str, exact_value=GPU_EMC_PROFILE, name='gpu/emc profile')]

    Expectation.check_all(expectations, result.splitlines())
