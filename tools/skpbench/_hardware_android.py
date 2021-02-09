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
    self.warmup_time = 5
    self._adb = adb
    self.desiredClock = 0.66

    if self._adb.root():
      self._adb.remount()

  def __enter__(self):
    Hardware.__enter__(self)
    if not self._adb.is_root() and self._adb.root():
      self._adb.remount()

    self._adb.shell('\n'.join([
      # turn on airplane mode.
      '''
      settings put global airplane_mode_on 1''',

      # disable GPS.
      '''
      settings put secure location_providers_allowed -gps
      settings put secure location_providers_allowed -wifi
      settings put secure location_providers_allowed -network''']))

    if self._adb.is_root():

      # For explanation of variance reducing steps, see
      # https://g3doc.corp.google.com/engedu/portal/android/g3doc/learn/develop/performance/content/best/reliable-startup-latency.md?cl=head

      self._adb.shell('\n'.join([
        # disable bluetooth, wifi, and mobile data.
        '''
        service call bluetooth_manager 8
        svc wifi disable
        svc data disable''',

        # kill the gui.
        '''
        setprop ctl.stop media
        setprop ctl.stop zygote
        setprop ctl.stop surfaceflinger
        setprop ctl.stop drm''',

        # disable ASLR
        '''
        echo 0 > /proc/sys/kernel/randomize_va_space''',
        ]))

      self.lock_top_three_cores()

      self.lock_adreno_gpu()

    else:
      print("WARNING: no adb root access; results may be unreliable.",
            file=sys.stderr)

    return self

  def __exit__(self, exception_type, exception_value, traceback):
    Hardware.__exit__(self, exception_type, exception_value, traceback)
    self._adb.reboot() # some devices struggle waking up; just hard reboot.

  def sanity_check(self):
    Hardware.sanity_check(self)

  def print_debug_diagnostics(self):
    # search for and print thermal trip points that may have been exceeded.
    self._adb.shell('''\
      THERMALDIR=/sys/class/thermal
      if [ ! -d $THERMALDIR ]; then
        exit
      fi
      for ZONE in $(cd $THERMALDIR; echo thermal_zone*); do
        cd $THERMALDIR/$ZONE
        if [ ! -e mode ] || grep -Fxqv enabled mode || [ ! -e trip_point_0_temp ]; then
          continue
        fi
        TEMP=$(cat temp)
        TRIPPOINT=trip_point_0_temp
        if [ $TEMP -le $(cat $TRIPPOINT) ]; then
          echo "$ZONE ($(cat type)): temp=$TEMP <= $TRIPPOINT=$(cat $TRIPPOINT)" 1>&2
        else
          let i=1
          while [ -e trip_point_${i}_temp ] &&
                [ $TEMP -gt $(cat trip_point_${i}_temp) ]; do
            TRIPPOINT=trip_point_${i}_temp
            let i=i+1
          done
          echo "$ZONE ($(cat type)): temp=$TEMP > $TRIPPOINT=$(cat $TRIPPOINT)" 1>&2
        fi
      done''')

    Hardware.print_debug_diagnostics(self)

  # expects a float between 0 and 100 representing where along the list of freqs to choose a value.
  def setDesiredClock(self, c):
    self.desiredClock = c / 100

  def lock_top_three_cores(self):
    # Lock the clocks of the fastest three cores and disable others.
    # Assumes root privlidges
    core_count = int(self._adb.check('cat /proc/cpuinfo | grep processor | wc -l'))
    max_speeds = []
    for i in range(core_count):
      khz = int(self._adb.check('cat /sys/devices/system/cpu/cpu%i/cpufreq/cpuinfo_max_freq' % i))
      max_speeds.append((khz, i)) # the tuple's first position and it will be the sort key
    cores_in_desc_order_of_max_speed = [a[1] for a in sorted(max_speeds, reverse=True)]
    top_cores = cores_in_desc_order_of_max_speed[:3]
    disable_cores = cores_in_desc_order_of_max_speed[3:]
    if disable_cores:
      self._adb.shell('\n'.join([('echo 0 > /sys/devices/system/cpu/cpu%i/online' % i) for i in disable_cores]))
    # since thermal-engine will be disabled, don't pick the max freq to lock these at,
    # pick something lower, so it doesn't get too hot (it'd reboot)
    # get a list of available scaling frequencies and pick one 2/3 of the way up.
    for i in top_cores:
      freqs = self._adb.check('cat /sys/devices/system/cpu/cpu%i/cpufreq/scaling_available_frequencies' % i).split()
      speed = freqs[int((len(freqs)-1) * self.desiredClock)]
      self._adb.shell('''echo 1 > /sys/devices/system/cpu/cpu{id}/online
      echo userspace > /sys/devices/system/cpu/cpu{id}/cpufreq/scaling_governor
      echo {speed} > /sys/devices/system/cpu/cpu{id}/cpufreq/scaling_max_freq
      echo {speed} > /sys/devices/system/cpu/cpu{id}/cpufreq/scaling_min_freq
      echo {speed} > /sys/devices/system/cpu/cpu{id}/cpufreq/scaling_setspeed'''.format(id=i, speed=speed))

  def lock_adreno_gpu(self):
    # Use presence of /sys/class/kgsl to indicate Adreno GPU
    exists = self._adb.check('test -d /sys/class/kgsl && echo y')
    if (exists.strip() != 'y'):
      print('Not attempting Adreno GPU clock locking steps')
      return

    # variance reducing changes
    self._adb.shell('''
      echo 0 > /sys/class/kgsl/kgsl-3d0/bus_split
      echo 1 > /sys/class/kgsl/kgsl-3d0/force_clk_on
      echo 10000 > /sys/class/kgsl/kgsl-3d0/idle_timer''')

    freqs = self._adb.check('cat /sys/class/kgsl/kgsl-3d0/devfreq/available_frequencies').split()
    speed = freqs[int((len(freqs)-1) * self.desiredClock)]

    # Set GPU to performance mode and lock clock
    self._adb.shell('''
      echo performance > /sys/class/kgsl/kgsl-3d0/devfreq/governor
      echo {speed} > /sys/class/kgsl/kgsl-3d0/devfreq/max_freq
      echo {speed} > /sys/class/kgsl/kgsl-3d0/devfreq/min_freq'''.format(speed=speed))

    # Set GPU power level
    self._adb.shell('''
      echo 1 > /sys/class/kgsl/kgsl-3d0/max_pwrlevel
      echo 1 > /sys/class/kgsl/kgsl-3d0/min_pwrlevel''')
