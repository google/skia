# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys
import time

ADB = sys.argv[1]
target_percent = float(sys.argv[2])
cpu = int(sys.argv[3])
log = subprocess.check_output([ADB, 'root']).decode('utf-8')
# check for message like 'adbd cannot run as root in production builds'
print(log)
if 'cannot' in log:
  raise Exception('adb root failed')

root = '/sys/devices/system/cpu/cpu%d/cpufreq' %cpu

# All devices we test on give a list of their available frequencies.
available_freqs = subprocess.check_output([ADB, 'shell',
    'cat %s/scaling_available_frequencies' % root]).decode('utf-8')

# Check for message like '/system/bin/sh: file not found'
if available_freqs and '/system/bin/sh' not in available_freqs:
  available_freqs = sorted(
      int(i) for i in available_freqs.strip().split())
else:
  raise Exception('Could not get list of available frequencies: %s' %
                  available_freqs)

maxfreq = available_freqs[-1]
target = int(round(maxfreq * target_percent))
freq = maxfreq
for f in reversed(available_freqs):
  if f <= target:
    freq = f
    break

print('Setting frequency to %d' % freq)

# If scaling_max_freq is lower than our attempted setting, it won't take.
# We must set min first, because if we try to set max to be less than min
# (which sometimes happens after certain devices reboot) it returns a
# perplexing permissions error.
subprocess.check_call([ADB, 'shell', 'echo 0 > '
    '%s/scaling_min_freq' % root])
subprocess.check_call([ADB, 'shell', 'echo %d > '
    '%s/scaling_max_freq' % (freq, root)])
subprocess.check_call([ADB, 'shell', 'echo %d > '
    '%s/scaling_setspeed' % (freq, root)])
time.sleep(5)
actual_freq = subprocess.check_output([ADB, 'shell', 'cat '
    '%s/scaling_cur_freq' % root]).decode('utf-8').strip()
if actual_freq != str(freq):
  raise Exception('(actual, expected) (%s, %d)'
                  % (actual_freq, freq))
