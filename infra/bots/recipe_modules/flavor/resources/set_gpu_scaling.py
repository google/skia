# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys
import time

ADB = sys.argv[1]
freq = sys.argv[2]
idle_timer = "10000"

log = subprocess.check_output([ADB, 'root']).decode('utf-8')
# check for message like 'adbd cannot run as root in production builds'
print(log)
if 'cannot' in log:
  raise Exception('adb root failed')

subprocess.check_output([ADB, 'shell', 'stop', 'thermald']).decode('utf-8')

subprocess.check_output([ADB, 'shell', 'echo "%s" > '
    '/sys/class/kgsl/kgsl-3d0/gpuclk' % freq]).decode('utf-8')

actual_freq = subprocess.check_output([ADB, 'shell', 'cat '
    '/sys/class/kgsl/kgsl-3d0/gpuclk']).decode('utf-8').strip()
if actual_freq != freq:
  raise Exception('Frequency (actual, expected) (%s, %s)'
                  % (actual_freq, freq))

subprocess.check_call([ADB, 'shell', 'echo "%s" > '
    '/sys/class/kgsl/kgsl-3d0/idle_timer' % idle_timer])

actual_timer = subprocess.check_output([ADB, 'shell', 'cat '
    '/sys/class/kgsl/kgsl-3d0/idle_timer']).decode('utf-8').strip()
if actual_timer != idle_timer:
  raise Exception('idle_timer (actual, expected) (%s, %s)'
                  % (actual_timer, idle_timer))

for s in ['force_bus_on', 'force_rail_on', 'force_clk_on']:
  subprocess.check_call([ADB, 'shell', 'echo "1" > '
      '/sys/class/kgsl/kgsl-3d0/%s' % s])
  actual_set = subprocess.check_output([ADB, 'shell', 'cat '
      '/sys/class/kgsl/kgsl-3d0/%s' % s]).decode('utf-8').strip()
  if actual_set != "1":
    raise Exception('%s (actual, expected) (%s, 1)'
                    % (s, actual_set))
