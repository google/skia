# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from _hardware import HardwareException, Expectation
from _hardware_android import HardwareAndroid

CPU_CLOCK_RATE = 1728000
GPU_CLOCK_RATE = 510000000

class HardwareNexus6P(HardwareAndroid):
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
    if not self._adb.is_root():
      return

    self._adb.shell('''\
      stop thermal-engine
      stop thermald
      stop perfd
      stop mpdecision''')

    # enable and lock 3 of 4 big cores.
    self._adb.shell('''\
      for N in 4 5 6; do
        echo 1 > /sys/devices/system/cpu/cpu$N/online
        echo userspace > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_governor
        echo %i > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_max_freq
        echo %i > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_min_freq
        echo %i > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_setspeed
      done''' % tuple(CPU_CLOCK_RATE for _ in range(3)))

    # turn off all other cores
    self._adb.shell('''\
      for N in 0 1 2 3 7; do
        echo 0 > /sys/devices/system/cpu/cpu$N/online
      done''')

    # gpu/ddr perf commands from
    # https://developer.qualcomm.com/qfile/28823/lm80-p0436-11_adb_commands.pdf
    self._adb.shell('''\
      echo 0 > /sys/class/kgsl/kgsl-3d0/bus_split
      echo 1 > /sys/class/kgsl/kgsl-3d0/force_bus_on
      echo 1 > /sys/class/kgsl/kgsl-3d0/force_rail_on
      echo 1 > /sys/class/kgsl/kgsl-3d0/force_clk_on
      echo 1000000 > /sys/class/kgsl/kgsl-3d0/idle_timer
      echo performance > /sys/class/kgsl/kgsl-3d0/devfreq/governor
      echo %i > /sys/class/kgsl/kgsl-3d0/devfreq/max_freq
      echo %i > /sys/class/kgsl/kgsl-3d0/devfreq/min_freq
      echo %i > /sys/class/kgsl/kgsl-3d0/gpuclk''' %
      tuple(GPU_CLOCK_RATE for _ in range(3)))

    # ddr perf commands from
    # https://developer.qualcomm.com/qfile/28823/lm80-p0436-11_adb_commands.pdf
    self._adb.shell('''\
      echo performance > /sys/class/devfreq/qcom,cpubw.32/governor
      echo 9887 > /sys/class/devfreq/qcom,cpubw.32/max_freq
      echo 9887 > /sys/class/devfreq/qcom,cpubw.32/min_freq
      echo performance > /sys/class/devfreq/qcom,gpubw.70/governor
      echo 9887 > /sys/class/devfreq/qcom,gpubw.70/max_freq
      echo 9887 > /sys/class/devfreq/qcom,gpubw.70/min_freq''')

  def _unlock_clocks(self):
    if not self._adb.is_root():
      return

    # restore ddr settings to default.
    self._adb.shell('''\
      echo 1525 > /sys/class/devfreq/qcom,cpubw.32/min_freq
      echo 9887 > /sys/class/devfreq/qcom,cpubw.32/max_freq
      echo bw_hwmon > /sys/class/devfreq/qcom,cpubw.32/governor
      echo 1525 > /sys/class/devfreq/qcom,gpubw.70/min_freq
      echo 9887 > /sys/class/devfreq/qcom,gpubw.70/max_freq
      echo bw_hwmon > /sys/class/devfreq/qcom,gpubw.70/governor''')

    # restore gpu settings to default.
    self._adb.shell('''\
      echo 180000000 > /sys/class/kgsl/kgsl-3d0/devfreq/min_freq
      echo 600000000 > /sys/class/kgsl/kgsl-3d0/devfreq/max_freq
      echo 180000000 > /sys/class/kgsl/kgsl-3d0/gpuclk
      echo msm-adreno-tz > /sys/class/kgsl/kgsl-3d0/devfreq/governor
      echo 0 > /sys/class/kgsl/kgsl-3d0/idle_timer
      echo 0 > /sys/class/kgsl/kgsl-3d0/force_clk_on
      echo 0 > /sys/class/kgsl/kgsl-3d0/force_rail_on
      echo 0 > /sys/class/kgsl/kgsl-3d0/force_bus_on
      echo 1 > /sys/class/kgsl/kgsl-3d0/bus_split''')

    # turn the disabled cores back on.
    self._adb.shell('''\
      for N in 7 3 2 1 0; do
        echo 1 > /sys/devices/system/cpu/cpu$N/online
      done''')

    # unlock the 3 enabled big cores.
    self._adb.shell('''\
      for N in 6 5 4; do
        echo 633600 > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_min_freq
        echo 1958400 > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_max_freq
        echo 0 > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_setspeed
        echo interactive > /sys/devices/system/cpu/cpu$N/cpufreq/scaling_governor
      done''')

    self._adb.shell('''\
      start mpdecision
      start perfd
      start thermald
      start thermal-engine''')

  def sanity_check(self):
    HardwareAndroid.sanity_check(self)

    if not self._adb.is_root():
      return

    result = self._adb.check('''\
      cat /sys/class/power_supply/battery/capacity \
          /sys/devices/system/cpu/online \
          /sys/class/thermal/thermal_zone14/temp \
          /sys/class/thermal/thermal_zone15/temp \
          /sys/kernel/debug/clk/oxili_gfx3d_clk/measure \
          /sys/kernel/debug/clk/bimc_clk/measure
      for N in 4 5 6; do
        cat /sys/devices/system/cpu/cpu$N/cpufreq/scaling_cur_freq
      done''')

    expectations = \
      [Expectation(int, min_value=30, name='battery', sleeptime=30*60),
       Expectation(str, exact_value='4-6', name='online cpus'),
       Expectation(int, max_value=88, name='tsens_tz_sensor13'),
       Expectation(int, max_value=88, name='tsens_tz_sensor14'),
       Expectation(long, min_value=(GPU_CLOCK_RATE - 5000),
                   max_value=(GPU_CLOCK_RATE + 5000), name='gpu clock rate'),
       Expectation(long, min_value=647995000, max_value=648007500,
                   name='ddr clock rate', sleeptime=10)] + \
      [Expectation(int, exact_value=CPU_CLOCK_RATE, name='cpu_%i clock rate' %i)
       for i in range(4, 7)]

    Expectation.check_all(expectations, result.splitlines())

  def sleep(self, sleeptime):
    self._unlock_clocks()
    HardwareAndroid.sleep(self, sleeptime)
    self._lock_clocks()
