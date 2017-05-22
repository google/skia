# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

DEPS = [
  'isolate',
  'recipe_engine/path',
]


def RunSteps(api):
  api.isolate.isolate_tests(api.path['checkout'].join('out', 'Release'))


def GenTests(api):
  yield api.test('basic')
