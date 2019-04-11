#!/usr/bin/env python
# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os

# TODO(benjaminwagner): This file probably shouldn't be in HOME.
SSH_MACHINE_FILE = os.path.expanduser('~/ssh_machine.json')

with open(SSH_MACHINE_FILE, 'r') as f:
  print f.read()
