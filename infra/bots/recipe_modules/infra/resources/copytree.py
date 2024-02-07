# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import shutil
import sys

shutil.copytree(sys.argv[1], sys.argv[2], dirs_exist_ok=True)
