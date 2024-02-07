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
value = int(sys.argv[3])

log = subprocess.check_output([ADB, 'root']).decode('utf-8')
# check for message like 'adbd cannot run as root in production builds'
print(log)
if 'cannot' in log:
  raise Exception('adb root failed')

# If we try to echo 1 to an already online cpu, adb returns exit code 1.
# So, check the value before trying to write it.
prior_status = subprocess.check_output([ADB, 'shell', 'cat '
    '/sys/devices/system/cpu/cpu%d/online' % cpu]).decode('utf-8').strip()
if prior_status == str(value):
  print('CPU %d online already %d' % (cpu, value))
  sys.exit()

subprocess.check_call([ADB, 'shell', 'echo %s > '
    '/sys/devices/system/cpu/cpu%d/online' % (value, cpu)])
actual_status = subprocess.check_output([ADB, 'shell', 'cat '
    '/sys/devices/system/cpu/cpu%d/online' % cpu]).decode('utf-8').strip()
if actual_status != str(value):
  raise Exception('(actual, expected) (%s, %d)' % (actual_status, value))
