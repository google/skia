# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from _hardware import HardwareException, Expectation
from _hardware_android import HardwareAndroid
from collections import namedtuple
import itertools

CPU_CLOCK_RATE = 1670400
GPU_CLOCK_RATE = 510000000

DEVFREQ_DIRNAME = '/sys/class/devfreq'
DEVFREQ_THROTTLE = 0.74
DEVFREQ_BLACKLIST = ('b00000.qcom,kgsl-3d0', 'soc:qcom,kgsl-busmon',
                     'soc:qcom,m4m')
DevfreqKnobs = namedtuple('knobs',
                          ('available_governors', 'available_frequencies',
                           'governor', 'min_freq', 'max_freq', 'cur_freq'))

class HardwarePixel(HardwareAndroid):
  def __init__(self, adb):
    HardwareAndroid.__init__(self, adb)
    self._discover_devfreqs()

  def __enter__(self):
    HardwareAndroid.__enter__(self)
    if not self._adb.is_root():
      return self

    self._adb.shell('\n'.join(['''\
      stop thermal-engine
      stop thermald
      stop perfd
      stop mpdecision''',

      # enable and lock the two fast cores.
      '''
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

      # gpu perf commands from
      # https://developer.qualcomm.com/qfile/28823/lm80-p0436-11_adb_commands.pdf
      '''
      echo 0 > /sys/class/kgsl/kgsl-3d0/bus_split
      echo 1 > /sys/class/kgsl/kgsl-3d0/force_bus_on
      echo 1 > /sys/class/kgsl/kgsl-3d0/force_rail_on
      echo 1 > /sys/class/kgsl/kgsl-3d0/force_clk_on
      echo 1000000 > /sys/class/kgsl/kgsl-3d0/idle_timer
      echo userspace > /sys/class/kgsl/kgsl-3d0/devfreq/governor
      echo 2 > /sys/class/kgsl/kgsl-3d0/max_pwrlevel
      echo 2 > /sys/class/kgsl/kgsl-3d0/min_pwrlevel
      echo 2 > /sys/class/kgsl/kgsl-3d0/thermal_pwrlevel
      echo %i > /sys/class/kgsl/kgsl-3d0/devfreq/max_freq
      echo %i > /sys/class/kgsl/kgsl-3d0/devfreq/min_freq
      echo %i > /sys/class/kgsl/kgsl-3d0/max_gpuclk
      echo %i > /sys/class/kgsl/kgsl-3d0/gpuclk''' %
      tuple(GPU_CLOCK_RATE for _ in range(4))] + \

      self._devfreq_lock_cmds))

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
      ['/sys/class/kgsl/kgsl-3d0/thermal_pwrlevel',
       '/sys/kernel/debug/clk/gpu_gx_gfx3d_clk/measure',
       '/sys/kernel/debug/clk/bimc_clk/measure',
       '/sys/class/thermal/thermal_zone22/temp',
       '/sys/class/thermal/thermal_zone23/temp'] + \
      self._devfreq_sanity_knobs))

    expectations = \
      [Expectation(int, min_value=30, name='battery', sleeptime=30*60),
       Expectation(str, exact_value='2-3', name='online cpus')] + \
      [Expectation(int, exact_value=CPU_CLOCK_RATE, name='cpu_%i clock rate' %i)
       for i in range(2, 4)] + \
      [Expectation(int, exact_value=2, name='gpu thermal power level'),
       Expectation(long, min_value=(GPU_CLOCK_RATE - 5000),
                   max_value=(GPU_CLOCK_RATE + 5000),
                   name='measured gpu clock'),
       Expectation(long, min_value=902390000, max_value=902409999,
                   name='measured ddr clock', sleeptime=10),
       Expectation(int, max_value=41000, name='pm8994_tz temperature'),
       Expectation(int, max_value=40, name='msm_therm temperature')] + \
      self._devfreq_sanity_expectations

    Expectation.check_all(expectations, result.splitlines())

  def _discover_devfreqs(self):
    self._devfreq_lock_cmds = list()
    self._devfreq_sanity_knobs = list()
    self._devfreq_sanity_expectations = list()

    results = iter(self._adb.check('''\
      KNOBS='%s'
      for DEVICE in %s/*; do
        if cd $DEVICE && ls $KNOBS >/dev/null; then
          basename $DEVICE
          cat $KNOBS
        fi
      done 2>/dev/null''' %
      (' '.join(DevfreqKnobs._fields), DEVFREQ_DIRNAME)).splitlines())

    while True:
      batch = tuple(itertools.islice(results, 1 + len(DevfreqKnobs._fields)))
      if not batch:
        break

      devfreq = batch[0]
      if devfreq in DEVFREQ_BLACKLIST:
        continue

      path = '%s/%s' % (DEVFREQ_DIRNAME, devfreq)

      knobs = DevfreqKnobs(*batch[1:])
      if not 'performance' in knobs.available_governors.split():
        print('WARNING: devfreq %s does not have performance governor' % path)
        continue

      self._devfreq_lock_cmds.append('echo performance > %s/governor' % path)

      frequencies = map(int, knobs.available_frequencies.split())
      if frequencies:
        # choose the lowest frequency that is >= DEVFREQ_THROTTLE * max.
        frequencies.sort()
        target = DEVFREQ_THROTTLE * frequencies[-1]
        idx = len(frequencies) - 1
        while idx > 0 and frequencies[idx - 1] >= target:
          idx -= 1
        bench_frequency = frequencies[idx]
        self._devfreq_lock_cmds.append('echo %i > %s/min_freq' %
                                      (bench_frequency, path))
        self._devfreq_lock_cmds.append('echo %i > %s/max_freq' %
                                      (bench_frequency, path))
        self._devfreq_sanity_knobs.append('%s/cur_freq' % path)
        self._devfreq_sanity_expectations.append(
          Expectation(int, exact_value=bench_frequency,
                      name='%s/cur_freq' % path))
