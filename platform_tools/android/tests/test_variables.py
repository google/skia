#!/usr/bin/python

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Common variables for tests.
"""

import os

# Find this file so we can find the python files to test.
SCRIPT_DIR = os.path.dirname(__file__)
ANDROID_DIR = os.path.normpath(os.path.join(SCRIPT_DIR, os.pardir))

# Path to gyp_to_android.
BIN_DIR = os.path.join(ANDROID_DIR, 'bin')

# Path to generator files.
GYP_GEN_DIR = os.path.join(ANDROID_DIR, 'gyp_gen')

ANDROID_MK = 'Android.mk'
