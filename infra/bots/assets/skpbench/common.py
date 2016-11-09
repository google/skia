#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Common vars used by scripts in this directory."""


import os
import sys

FILE_DIR = os.path.dirname(os.path.abspath(__file__))
INFRA_BOTS_DIR = os.path.realpath(os.path.join(FILE_DIR, os.pardir, os.pardir))

sys.path.insert(0, INFRA_BOTS_DIR)
from assets import assets

ASSET_NAME = os.path.basename(FILE_DIR)


def run(cmd):
  """Run a command, eg. "upload" or "download". """
  assets.main([cmd, ASSET_NAME] + sys.argv[1:])
