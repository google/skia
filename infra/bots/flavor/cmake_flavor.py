#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import default_flavor
import os


"""CMake flavor utils, used for building Skia with CMake."""


class CMakeFlavorUtils(default_flavor.DefaultFlavorUtils):
  def compile(self, target):
    """Build Skia with CMake.  Ignores `target`."""
    cmake_build = os.path.join(self._bot_info.skia_dir, 'cmake', 'cmake_build')
    self._bot_info.run([cmake_build, target])
