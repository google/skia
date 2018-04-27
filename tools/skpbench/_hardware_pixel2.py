# Copyright 2018 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from _hardware import Expectation
from _hardware_android import HardwareAndroid

CPU_CLOCK_RATE = 2035200
MEM_CLOCK_RATE = 13763
GPU_CLOCK_RATE = 670000000
GPU_POWER_LEVEL = 1  # lower is faster, minimum is 0

class HardwarePixel2(HardwareAndroid):
  def __init__(self, adb):
    HardwareAndroid.__init__(self, adb)

  def __enter__(self):
    HardwareAndroid.__enter__(self)
    if not self._adb.is_root():
      return self

    self._adb.shell('\n'.join([
      '''
      stop thermal-engine
      stop perfd''',

      # turn off the slow cores and one fast core
      '''
      for N in 0 1 2 3 7; do
        echo 0 > /sys/devices/system/cpu/cpu$N/online
      done''',

      # lock 3 fast cores: two for Skia and one for the OS
      '''
      for N in 4 5 6; do
        echo 1 > /sys/devices/system/cpu/cpu$N/online
        echo userspace > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_governor
        echo %i > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_max_freq
        echo %i > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_min_freq
        echo %i > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_setspeed
      done''' % tuple(CPU_CLOCK_RATE for _ in range(3)),

      # Set GPU bus and idle timer
      '''
      echo 0 > /sys/class/kgsl/kgsl-3d0/bus_split''',
      # csmartdalton, 4-26-2018: this line hangs my device
      # echo 1 > /sys/class/kgsl/kgsl-3d0/force_clk_on
      '''
      echo 10000 > /sys/class/kgsl/kgsl-3d0/idle_timer''',

      # Set mem frequency to max
      '''
      echo %i > /sys/class/devfreq/soc\:qcom,gpubw/min_freq
      echo %i > /sys/class/devfreq/soc\:qcom,gpubw/max_freq
      echo %i > /sys/class/devfreq/soc\:qcom,cpubw/min_freq
      echo %i > /sys/class/devfreq/soc\:qcom,cpubw/max_freq
      echo %i > /sys/class/devfreq/soc\:qcom,mincpubw/min_freq
      echo %i > /sys/class/devfreq/soc\:qcom,mincpubw/max_freq
      echo %i > /sys/class/devfreq/soc\:qcom,memlat-cpu0/min_freq
      echo %i > /sys/class/devfreq/soc\:qcom,memlat-cpu0/max_freq''' %
      tuple(MEM_CLOCK_RATE for _ in range(8)),

      # Set GPU to performance mode
      '''
      echo performance > /sys/class/kgsl/kgsl-3d0/devfreq/governor
      echo %i > /sys/class/kgsl/kgsl-3d0/devfreq/max_freq
      echo %i > /sys/class/kgsl/kgsl-3d0/devfreq/min_freq''' %
      tuple(GPU_CLOCK_RATE for _ in range(2)),

      # Set GPU power level
      '''
      echo %i > /sys/class/kgsl/kgsl-3d0/max_pwrlevel
      echo %i > /sys/class/kgsl/kgsl-3d0/min_pwrlevel''' %
      tuple(GPU_POWER_LEVEL for _ in range(2))]))

    assert('msm_therm' == self._adb.check(\
                          'cat /sys/class/thermal/thermal_zone10/type').strip())
    assert('pm8998_tz' == self._adb.check(\
                          'cat /sys/class/thermal/thermal_zone7/type').strip())

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
       for i in range(4, 7)] + \
      # Unfortunately we can't monitor the gpu clock:
      #
      #   /sys/class/kgsl/kgsl-3d0/devfreq/cur_freq
      #
      # It doesn't respect the min_freq/max_freq values when not under load.
      ['/sys/kernel/debug/clk/bimc_clk/measure',
       '/sys/class/kgsl/kgsl-3d0/temp',
       '/sys/class/kgsl/kgsl-3d0/throttling',
       '/sys/class/thermal/thermal_zone10/temp',
       '/sys/class/thermal/thermal_zone7/temp']))

    expectations = \
      [Expectation(int, min_value=30, name='battery', sleeptime=30*60),
       Expectation(str, exact_value='4-6', name='online cpus')] + \
      [Expectation(int, exact_value=CPU_CLOCK_RATE, name='cpu_%i clock rate' %i)
       for i in range(4, 7)] + \
      [Expectation(long, min_value=902390000, max_value=902409999,
                   name='measured ddr clock', sleeptime=10),
       Expectation(int, max_value=750, name='gpu temperature'),
       Expectation(int, exact_value=1, name='gpu throttling'),
       Expectation(int, max_value=75, name='msm_therm temperature'),
       Expectation(int, max_value=75000, name='pm8998_tz temperature')]

    Expectation.check_all(expectations, result.splitlines())
