# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
import re
import subprocess
import sys

class Adb:
  def __init__(self, device_serial=None, echofile=None):
    self.__invocation = ['adb']
    if device_serial:
      self.__invocation.extend(['-s', device_serial])
    self.__echofile = echofile
    self.__is_root = None

  def shell(self, cmd):
    if self.__echofile:
      self.__echo_cmd(cmd)
    subprocess.call(self.__invocation + ['shell', cmd], stdout=sys.stderr)

  def check(self, cmd):
    if self.__echofile:
      self.__echo_cmd(cmd)
    result = subprocess.check_output(self.__invocation + ['shell', cmd])
    if self.__echofile:
      print(result, file=self.__echofile)
    return result

  def root(self):
    if not self.is_root():
      subprocess.call(self.__invocation + ['root'], stdout=sys.stderr)
      self.__is_root = None
    return self.is_root()

  def is_root(self):
    if self.__is_root is None:
      self.__is_root = ('root' == self.check('whoami').strip())
    return self.__is_root

  def remount(self):
    subprocess.call(self.__invocation + ['remount'], stdout=sys.stderr)

  def __echo_cmd(self, cmd):
    escaped = [re.sub(r'([^a-zA-Z0-9])', r'\\\1', x)
               for x in cmd.strip().splitlines()]
    subprocess.call(self.__invocation + \
                    ['shell', 'echo', '$(whoami)@$(getprop ro.serialno)$',
                     " '\n>' ".join(escaped)],
                    stdout=self.__echofile)
