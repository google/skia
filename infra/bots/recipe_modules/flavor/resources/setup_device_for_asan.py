# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys
import time

ADB = sys.argv[1]
ASAN_SETUP = sys.argv[2]

def wait_for_device():
  while True:
    time.sleep(5)
    print('Waiting for device')
    subprocess.check_call([ADB, 'wait-for-device'])
    bit1 = subprocess.check_output([ADB, 'shell', 'getprop',
                                   'dev.bootcomplete']).decode('utf-8')
    bit2 = subprocess.check_output([ADB, 'shell', 'getprop',
                                   'sys.boot_completed']).decode('utf-8')
    if '1' in bit1 and '1' in bit2:
      print('Device detected')
      break

log = subprocess.check_output([ADB, 'root']).decode('utf-8')
# check for message like 'adbd cannot run as root in production builds'
print(log)
if 'cannot' in log:
  raise Exception('adb root failed')

output = subprocess.check_output([ADB, 'disable-verity']).decode('utf-8')
print(output)

if 'already disabled' not in output:
  print('Rebooting device')
  subprocess.check_call([ADB, 'reboot'])
  wait_for_device()

def installASAN(revert=False):
  # ASAN setup script is idempotent, either it installs it or
  # says it's installed.  Returns True on success, false otherwise.
  out = subprocess.check_output([ADB, 'wait-for-device']).decode('utf-8')
  print(out)
  cmd = [ASAN_SETUP]
  if revert:
    cmd = [ASAN_SETUP, '--revert']
  process = subprocess.Popen(cmd, env={'ADB': ADB},
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)

  # this also blocks until command finishes
  (stdout, stderr) = process.communicate()
  print(stdout.decode('utf-8'))
  print('Stderr: %s' % stderr.decode('utf-8'))
  return process.returncode == 0

if not installASAN():
  print('Trying to revert the ASAN install and then re-install')
  # ASAN script sometimes has issues if it was interrupted or partially applied
  # Try reverting it, then re-enabling it
  if not installASAN(revert=True):
    raise Exception('reverting ASAN install failed')

  # Sleep because device does not reboot instantly
  time.sleep(10)

  if not installASAN():
    raise Exception('Tried twice to setup ASAN and failed.')

# Sleep because device does not reboot instantly
time.sleep(10)
wait_for_device()
# Sleep again to hopefully avoid error "secure_mkdirs failed: No such file or
# directory" when pushing resources to the device.
time.sleep(60)
