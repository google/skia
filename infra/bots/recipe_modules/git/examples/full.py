# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = "PY3"

DEPS = [
  'git',
  'recipe_engine/platform',
  'recipe_engine/step',
]


def RunSteps(api):
  api.step('1', cmd=['git', 'status'])
  with api.git.env():
    api.step('2', cmd=['git', 'status'])


def GenTests(api):
  yield api.test('test')
  yield api.test('test-win') + api.platform('win', 64)
