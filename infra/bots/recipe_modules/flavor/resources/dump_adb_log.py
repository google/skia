# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

out = sys.argv[1]
adb = sys.argv[2]
log = subprocess.check_output([
    adb, 'logcat', '-d']).decode('utf-8', errors='ignore')
for line in log.split('\\n'):
  tokens = line.split()
  if len(tokens) == 11 and tokens[-7] == 'F' and tokens[-3] == 'pc':
    addr, path = tokens[-2:]
    local = os.path.join(out, os.path.basename(path))
    if os.path.exists(local):
      try:
        sym = subprocess.check_output([
            'addr2line', '-Cfpe', local, addr]).decode('utf-8')
        line = line.replace(addr, addr + ' ' + sym.strip())
      except subprocess.CalledProcessError:
        pass
  print(line)
