# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import contextlib
import math
import socket
import sys
import time

from urllib.request import urlopen

HASHES_URL = sys.argv[1]
RETRIES = 5
TIMEOUT = 60
WAIT_BASE = 15

socket.setdefaulttimeout(TIMEOUT)
for retry in range(RETRIES):
  try:
    with contextlib.closing(
        urlopen(HASHES_URL, timeout=TIMEOUT)) as w:
      hashes = w.read().decode('utf-8')
      with open(sys.argv[2], 'w') as f:
        f.write(hashes)
        break
  except Exception as e:
    print('Failed to get uninteresting hashes from %s:' % HASHES_URL)
    print(e)
    if retry == RETRIES:
      raise
    waittime = WAIT_BASE * math.pow(2, retry)
    print('Retry in %d seconds.' % waittime)
    time.sleep(waittime)
