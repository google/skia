# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess

def shell(cmd, device_serial=None):
  if device_serial is None:
    subprocess.call(['adb', 'shell', cmd])
  else:
    subprocess.call(['adb', '-s', device_serial, 'shell', cmd])

def check(cmd, device_serial=None):
  if device_serial is None:
    out = subprocess.check_output(['adb', 'shell', cmd])
  else:
    out = subprocess.check_output(['adb', '-s', device_serial, 'shell', cmd])
  return out.rstrip()
