# Copyright 2025 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import subprocess
import time


def run(*cmd, capture_output=False):
  print('==================================')
  print(' '.join(cmd))
  print('----------------------------------')
  result = subprocess.run(cmd, check=True, capture_output=capture_output, timeout=180)
  print('==================================')
  return result


def adb_connect(adb, device):
  while True:
    result = run(adb, 'connect', device, capture_output=True)
    if result.returncode == 0 and 'unable to connect' not in result.stdout.decode():
      break
    time.sleep(1)

def main():
  parser = argparse.ArgumentParser(
                    prog='wait_for_device.py',
                    description='Wait for an Android device to connect.')
  parser.add_argument('--adb', default='adb')
  parser.add_argument('--connect-to')
  args = parser.parse_args()

  if args.connect_to:
    adb_connect(args.adb, args.connect_to)

  run(args.adb, 'wait-for-device', 'shell',
      'while [[ -z $(getprop sys.boot_completed) ]]; do sleep 1; done')


if __name__ == '__main__':
  main()
