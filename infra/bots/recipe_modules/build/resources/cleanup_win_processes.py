# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# [VPYTHON:BEGIN]
# wheel: <
#  name: "infra/python/wheels/psutil/${vpython_platform}"
#  version: "version:5.8.0.chromium.2"
# >
# [VPYTHON:END]

import psutil
for p in psutil.process_iter():
  try:
    if p.name in ('mspdbsrv.exe', 'vctip.exe', 'cl.exe', 'link.exe'):
      p.kill()
  except psutil._error.AccessDenied:
    pass
