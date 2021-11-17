# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = "PY2+3"

DEPS = [
  'doxygen',
  'recipe_engine/path',
  'recipe_engine/properties',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  api.doxygen.generate_and_upload(api.path['start_dir'])


def GenTests(api):
  yield (
      api.test('doxygen') +
      api.properties(buildername='Housekeeper-PerCommit',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
