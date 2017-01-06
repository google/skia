#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess

print subprocess.check_output('xcrun --sdk iphoneos --show-sdk-path'.split())
