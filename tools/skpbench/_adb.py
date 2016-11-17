# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
import re
import subprocess
import sys

class Adb:
  def __init__(self, device_serial=None, echo=False):
    self.__invocation = ['adb']
    if device_serial:
      self.__invocation.extend(['-s', device_serial])
    self.__echo = echo
    self.__is_root = None

  def shell(self, cmd):
    if self.__echo:
      self.__echo_shell_cmd(cmd)
    self.__invoke('shell', cmd)

  def check(self, cmd):
    if self.__echo:
      self.__echo_shell_cmd(cmd)
    result = subprocess.check_output(self.__invocation + ['shell', cmd])
    if self.__echo:
      print(result, file=sys.stderr)
    return result

  def root(self):
    if not self.is_root():
      self.__invoke('root')
      self.__invoke('wait-for-device')
      self.__is_root = None
    return self.is_root()

  def is_root(self):
    if self.__is_root is None:
      self.__is_root = ('root' == self.check('whoami').strip())
    return self.__is_root

  def remount(self):
    self.__invoke('remount')

  def __echo_shell_cmd(self, cmd):
    escaped = [re.sub(r'([^a-zA-Z0-9])', r'\\\1', x)
               for x in cmd.strip().splitlines()]
    self.__invoke('shell', 'echo', '$(whoami)@$(getprop ro.serialno)$',
                  " '\n>' ".join(escaped))

  def __invoke(self, *args):
    subprocess.call(self.__invocation + list(args), stdout=sys.stderr)
