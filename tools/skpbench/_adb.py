# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re
import subprocess

class Adb:
  def __init__(self, device_serial=None):
    self.__invocation = ['adb']
    if device_serial:
      self.__invocation.extend(['-s', device_serial])

  def shell(self, cmd):
    subprocess.call(self.__invocation + ['shell', cmd])

  def check(self, cmd):
    result = subprocess.check_output(self.__invocation + ['shell', cmd])
    return result.rstrip()

  def check_lines(self, cmd):
    result = self.check(cmd)
    return re.split('[\r\n]+', result)

  def get_device_model(self):
    result = self.check('getprop | grep ro.product.model')
    result = re.match(r'\[ro.product.model\]:\s*\[(.*)\]', result)
    return result.group(1) if result else 'unknown_product'

  def is_root(self):
    return self.check('echo $USER') == 'root'

  def attempt_root(self):
    if self.is_root():
      return True
    subprocess.call(self.__invocation + ['root'])
    return self.is_root()

  def remount(self):
    subprocess.call(self.__invocation + ['remount'])
