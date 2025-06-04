# Copyright 2025 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
  'recipe_engine/properties',
  'vars',
  'xcode',
]


def RunSteps(api):
  api.vars.setup()
  api.xcode.install()


def GenTests(api):
  yield api.test('xcode-test') + api.properties(
    buildername='Build-Mac-Clang-arm64-Debug-iOS',
    repository='https://skia.googlesource.com/skia.git',
    revision='abc123',
    path_config='kitchen',
    patch_set=2,
    swarm_out_dir='[SWARM_OUT_DIR]',
  )
