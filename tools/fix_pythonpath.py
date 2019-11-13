#!/usr/bin/env python
# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Add the checkout root to sys.path, provide mechanisms for adding others."""


import os
import sys


CHECKOUT_ROOT = os.path.realpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)), os.pardir))


def add_to_pythonpath(path):
  """Add the given directory to PYTHONPATH."""
  sys.path.append(path)


add_to_pythonpath(CHECKOUT_ROOT)

