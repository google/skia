#!/usr/bin/env python
# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import sys
import threading
import time

# Usage: ios_debug_cmd.py <bundle_id> <args...>
# Runs the given app with the given args with full debug output enabled, then
# outputs syslog for the period when the command was running. Exits with the
# exit code of idevicedebug.


bundle_id = sys.argv[1]
args = sys.argv[2:]

logp = subprocess.Popen(['idevicesyslog'], stdout=subprocess.PIPE, bufsize=-1)
log = ''
def collect_log():
  global log
  while True:
    out = logp.stdout.read()
    if out:
      log = log + out
    else:
      return

logt = threading.Thread(target=collect_log)
logt.start()

rv = subprocess.call(['idevicedebug', '--debug', 'run', bundle_id] + args)

print('\n\nreturned %d' % rv)

logp.terminate()
print('\n\nreading syslog...')
logt.join()
print('syslog follows')
print(log)

exit(rv)
