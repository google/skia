# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys
import time

ADB = sys.argv[1]
cpu = int(sys.argv[2])
gov = sys.argv[3]

log = subprocess.check_output([ADB, 'root']).decode('utf-8')
# check for message like 'adbd cannot run as root in production builds'
print(log)
if 'cannot' in log:
  raise Exception('adb root failed')

subprocess.check_output([
    ADB, 'shell',
    'echo "%s" > /sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor' % (
        gov, cpu)]).decode('utf-8')
actual_gov = subprocess.check_output([
    ADB, 'shell', 'cat /sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor' %
        cpu]).decode('utf-8').strip()
if actual_gov != gov:
  raise Exception('(actual, expected) (%s, %s)' % (actual_gov, gov))
