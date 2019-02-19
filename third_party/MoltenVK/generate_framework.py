#!/usr/bin/env python2.7
#
# Copyright 2019 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import sys

subprocess.check_call(['./fetchDependencies'])
subprocess.check_call(['xcodebuild',
                       '-quiet',
                       '-project', 'MoltenVKPackaging.xcodeproj',
                       '-scheme', 'MoltenVK (Release)',
                       'build'])
