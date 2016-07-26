# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import default_flavor

"""CMake flavor utils, used for building Skia with CMake."""

class CMakeFlavorUtils(default_flavor.DefaultFlavorUtils):
  def compile(self, target):
    """Build Skia with CMake.  Ignores `target`."""
    cmake_build = self._skia_api.skia_dir.join('cmake', 'cmake_build')
    self._skia_api.run(self._skia_api.m.step, 'cmake_build', cmd=[cmake_build],
                       cwd=self._skia_api.m.path['checkout'])
