#!/usr/bin/python
#
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

""" adb_list_devices: list information about attached Android devices. """


import os
import re
import shlex
import subprocess
import sys

# This file, which resides on every Android device, contains a great deal of
# information about the device.
INFO_FILE = '/system/build.prop'

# Default set of properties to query about a device.
DEFAULT_PROPS_TO_GET = ['ro.product.device', 'ro.build.version.release',
                        'ro.build.type']


def GetDeviceInfo(adb, serial, props_to_get):
  """ Return a list of values (or "<Unknown>" if no value can be found) for the
  given set of properties for the device with the given serial number.

  adb: path to the ADB program.
  serial: serial number of the target device.
  props_to_get: list of strings indicating which properties to determine.
  """
  device_proc = subprocess.Popen([adb, '-s', serial, 'shell', 'cat',
                                  INFO_FILE], stdout=subprocess.PIPE)
  code = device_proc.wait()
  if code != 0:
    raise Exception('Could not query device with serial number %s.' % serial)
  output = device_proc.stdout.read()
  device_info = []
  for prop in props_to_get:
    # Find the property in the outputs
    search_str = r'%s=(\S+)' % prop
    match = re.search(search_str, output)
    if not match:
      value = '<Unknown>'
    else:
      value = match.group(1)
    device_info.append(value)
  return device_info


def PrintPrettyTable(data, file=None):
  """ Print out the given data in a nicely-spaced format. This function scans
  the list multiple times and uses extra memory, so don't use it for big data
  sets.

  data: list of lists of strings, where each list represents a row of data.
      This table is assumed to be rectangular; if the length of any list differs
      some of the output may not get printed.
  file: file-like object into which the table should be written. If none is
      provided, the table is written to stdout.
  """
  if not file:
    file = sys.stdout
  column_widths = [0 for length in data[0]]
  for line in data:
    column_widths = [max(longest_len, len(prop)) for \
                    longest_len, prop in zip(column_widths, line)]
  for line in data:
    for prop, width in zip(line, column_widths):
      file.write(prop.ljust(width + 1))
    file.write('\n')


def FindADB(hint=None):
  """ Attempt to find the ADB program using the following sequence of steps.
  Returns the path to ADB if it can be found, or None otherwise.
  1. If a hint was provided, is it a valid path to ADB?
  2. Is ADB in PATH?
  3. Is there an environment variable for ADB?
  4. If the ANDROID_SDK_ROOT variable is set, try to find ADB in the SDK
     directory.

  hint: string indicating a possible path to ADB.
  """
  # 1. If a hint was provided, does it point to ADB?
  if hint:
    if os.path.basename(hint) == 'adb':
      adb = hint
    else:
      adb = os.path.join(hint, 'adb')
    if subprocess.Popen([adb, 'version'], stdout=subprocess.PIPE).wait() == 0:
      return adb

  # 2. Is 'adb' in our PATH?
  adb = 'adb'
  if subprocess.Popen([adb, 'version'], stdout=subprocess.PIPE).wait() == 0:
    return adb

  # 3. Is there an environment variable for ADB?
  try:
    adb = os.environ.get('ADB')
    if subprocess.Popen([adb, 'version'], stdout=subprocess.PIPE).wait() == 0:
      return adb    
  except:
    pass

  # 4. If ANDROID_SDK_ROOT is set, try to find ADB in the SDK directory.
  try:
    sdk_dir = os.environ.get('ANDROID_SDK_ROOT')
    adb = os.path.join(sdk_dir, 'platform-tools', 'adb')
    if subprocess.Popen([adb, 'version'], stdout=subprocess.PIPE).wait() == 0:
      return adb 
  except:
    pass
  return None


def main(argv):
  """ Print out information about connected Android devices. By default, print
  the serial number, status, device name, OS version, and build type of each
  device. If any arguments are supplied on the command line, print the serial
  number and status for each device along with values for those arguments
  interpreted as properties.
  """
  if len(argv) > 1:
    props_to_get = argv[1:]
  else:
    props_to_get = DEFAULT_PROPS_TO_GET
  adb = FindADB()
  if not adb:
    raise Exception('Could not find ADB!')
  proc = subprocess.Popen([adb, 'devices'], stdout=subprocess.PIPE)
  code = proc.wait()
  if code != 0:
    raise Exception('Failure in ADB: could not find attached devices.')
  header = ['Serial', 'Status']
  header.extend(props_to_get)
  output_lines = [header]
  for line in proc.stdout:
    line = line.rstrip()
    if line != 'List of devices attached' and line != '':
      line_list = shlex.split(line)
      serial = line_list[0]
      status = line_list[1]
      device_info = [serial, status]
      device_info.extend(GetDeviceInfo(adb, serial, props_to_get))
      output_lines.append(device_info)
  PrintPrettyTable(output_lines)


if __name__ == '__main__':
  sys.exit(main(sys.argv))