# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

DEPS = [
  'isolate',
  'recipe_engine/properties',
  'recipe_engine/step',
]


def RunSteps(api):
  api.step('isolated_tests', [])
  api.step.active_result.presentation.logs['details'] = [
    'isolated_tests: %r' % api.isolate.isolated_tests
  ]


def GenTests(api):
  yield (
      api.test('basic') +
      api.properties(
          swarm_hashes={
            'base_unittests': 'ffffffffffffffffffffffffffffffffffffffff',
          }
      )
  )
