#!/usr/bin/env python
#
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import os
import subprocess
import sys


kitchen = os.path.join(os.getcwd(), 'kitchen')
logdog_url = 'logdog://logs.chromium.org/%s/%s/+/annotations' % (
    sys.argv[4], os.environ['SWARMING_TASK_ID'])

cmd = [
  kitchen, 'cook',
    '-checkout-dir', 'recipe_bundle',
    '-mode', 'swarming',
    '-luci-system-account', 'system',
    '-cache-dir', 'cache',
    '-temp-dir', 'tmp',
    '-known-gerrit-host', 'android.googlesource.com',
    '-known-gerrit-host', 'boringssl.googlesource.com',
    '-known-gerrit-host', 'chromium.googlesource.com',
    '-known-gerrit-host', 'dart.googlesource.com',
    '-known-gerrit-host', 'fuchsia.googlesource.com',
    '-known-gerrit-host', 'go.googlesource.com',
    '-known-gerrit-host', 'llvm.googlesource.com',
    '-known-gerrit-host', 'skia.googlesource.com',
    '-known-gerrit-host', 'webrtc.googlesource.com',
    '-workdir', '.',
    '-recipe', sys.argv[2],
    '-properties', sys.argv[3],
    '-logdog-annotation-url', logdog_url,
]
print 'running command: %s' % ' '.join(cmd)
subprocess.check_call(cmd)
