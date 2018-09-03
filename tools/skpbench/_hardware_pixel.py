# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from _hardware import Expectation
from _hardware_android import HardwareAndroid

CPU_CLOCK_RATE = 1670400
GPU_CLOCK_RATE = 315000000

class HardwarePixel(HardwareAndroid):
  def __init__(self, adb):
    HardwareAndroid.__init__(self, adb)

  def __enter__(self):
    HardwareAndroid.__enter__(self)
    if not self._adb.is_root():
      return self

    self._adb.shell('\n'.join([
      # enable and lock the two fast cores.
      '''
      stop thermal-engine
      stop perfd

      for N in 3 2; do
        echo 1 > /sys/devices/system/cpu/cpu$N/online
        echo userspace > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_governor
        echo %i > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_max_freq
        echo %i > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_min_freq
        echo %i > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_setspeed
      done''' % tuple(CPU_CLOCK_RATE for _ in range(3)),

      # turn off the two slow cores
      '''
      for N in 1 0; do
        echo 0 > /sys/devices/system/cpu/cpu$N/online
      done''',

      # pylint: disable=line-too-long

      # Set GPU bus and idle timer
      # Set DDR frequency to max
      # Set GPU to performance mode, 315 MHZ
      # See https://android.googlesource.com/platform/frameworks/base/+/master/libs/hwui/tests/scripts/prep_marlfish.sh
      '''
      echo 0 > /sys/class/kgsl/kgsl-3d0/bus_split
      echo 1 > /sys/class/kgsl/kgsl-3d0/force_clk_on
      echo 10000 > /sys/class/kgsl/kgsl-3d0/idle_timer

      echo 13763 > /sys/class/devfreq/soc:qcom,gpubw/min_freq

      echo performance > /sys/class/kgsl/kgsl-3d0/devfreq/governor
      echo %i > /sys/class/kgsl/kgsl-3d0/devfreq/max_freq
      echo %i > /sys/class/kgsl/kgsl-3d0/devfreq/min_freq

      echo 4 > /sys/class/kgsl/kgsl-3d0/max_pwrlevel
      echo 4 > /sys/class/kgsl/kgsl-3d0/min_pwrlevel''' %
      tuple(GPU_CLOCK_RATE for _ in range(2))]))

    return self

  def sanity_check(self):
    HardwareAndroid.sanity_check(self)

    if not self._adb.is_root():
      return

    result = self._adb.check(' '.join(
      ['cat',
       '/sys/class/power_supply/battery/capacity',
       '/sys/devices/system/cpu/online'] + \
      ['/sys/devices/system/cpu/cpu%i/cpufreq/scaling_cur_freq' % i
       for i in range(2, 4)] + \
      ['/sys/kernel/debug/clk/bimc_clk/measure',
       '/sys/class/thermal/thermal_zone22/temp',
       '/sys/class/thermal/thermal_zone23/temp']))

    expectations = \
      [Expectation(int, min_value=30, name='battery', sleeptime=30*60),
       Expectation(str, exact_value='2-3', name='online cpus')] + \
      [Expectation(int, exact_value=CPU_CLOCK_RATE, name='cpu_%i clock rate' %i)
       for i in range(2, 4)] + \
       [Expectation(long, min_value=902390000, max_value=902409999,
                   name='measured ddr clock', sleeptime=10),
       Expectation(int, max_value=41000, name='pm8994_tz temperature'),
       Expectation(int, max_value=40, name='msm_therm temperature')]

    Expectation.check_all(expectations, result.splitlines())
