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

        # stop services which can change clock speed
        '''
        stop thermal-engine
        stop perfd''']))

      self.lock_top_three_cores()

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
      speed = freqs[int((len(freqs)-1)*.66)]
      self._adb.shell('''echo 1 > /sys/devices/system/cpu/cpu{id}/online
      echo userspace > /sys/devices/system/cpu/cpu{id}/cpufreq/scaling_governor
      echo {speed} > /sys/devices/system/cpu/cpu{id}/cpufreq/scaling_max_freq
      echo {speed} > /sys/devices/system/cpu/cpu{id}/cpufreq/scaling_min_freq
      echo {speed} > /sys/devices/system/cpu/cpu{id}/cpufreq/scaling_setspeed'''.format(id=i, speed=speed))
