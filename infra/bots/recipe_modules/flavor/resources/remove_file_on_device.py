# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import sys

# Remove the path.
adb = sys.argv[1]
path = sys.argv[2]
print('Removing %s' % path)
cmd = [adb, 'shell', 'rm', '-rf', path]
print(' '.join(cmd))
subprocess.check_call(cmd)

# Verify that the path was deleted.
print('Checking for existence of %s' % path)
cmd = [adb, 'shell', 'ls', path]
print(' '.join(cmd))
try:
    output = subprocess.check_output(
        cmd, stderr=subprocess.STDOUT).decode('utf-8')
except subprocess.CalledProcessError as e:
    output = e.output.decode('utf-8')
print('Output was:')
print('======')
print(output)
print('======')
if 'No such file or directory' not in output:
    raise Exception('%s exists despite being deleted' % path)
