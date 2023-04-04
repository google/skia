#!/usr/bin/env python3
#
# Copyright 2023 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import hashlib
import os
import shutil
import subprocess
import sys

target = sys.argv[1]
output = sys.argv[2]
subprocess.run(["bazelisk", "build", target], check=True)

expected_output = os.path.join(os.getcwd(), os.path.basename(output))
if not os.path.exists(expected_output):
    shutil.copyfile(output, expected_output)
    os.chmod(expected_output, 0o755)
else:
    created_hash = hashlib.sha256(open(output, 'rb').read()).hexdigest()
    existing_hash = hashlib.sha256(open(expected_output, 'rb').read()).hexdigest()
    if created_hash != existing_hash:
        os.remove(expected_output)
        shutil.copyfile(output, expected_output)
        os.chmod(expected_output, 0o755)
