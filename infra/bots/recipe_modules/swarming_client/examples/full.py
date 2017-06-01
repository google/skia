# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# TODO(borenet): This module was copied from build.git and heavily modified to
# remove dependencies on other modules in build.git.  It belongs in a different
# repo. Remove this once it has been moved.


DEPS = [
  'recipe_engine/properties',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'swarming_client',
]


def RunSteps(api):
  # Code coverage for these methods.
  api.step('client path', [])
  api.step.active_result.step_text = api.swarming_client.path
  api.swarming_client.checkout()
  #api.swarming_client.checkout('master')
  api.swarming_client.query_script_version('swarming.py')
  api.swarming_client.ensure_script_version('swarming.py', (0, 4, 4))

  # Coverage for |step_test_data| argument.
  api.swarming_client.query_script_version(
      'isolate.py', step_test_data=(0, 3, 1))

  # 'master' had swarming.py at v0.4.4 at the moment of writing this example.
  assert api.swarming_client.get_script_version('swarming.py') >= (0, 4, 4)

  # Coverage for 'fail' path of ensure_script_version.
  api.swarming_client.ensure_script_version('swarming.py', (20, 0, 0))


def GenTests(api):
  yield (
      api.test('basic') +
      api.properties(parent_got_swarming_client_revision='sample_sha') +
      api.step_data(
          'swarming.py --version',
          stdout=api.raw_io.output_text('0.4.4'))
  )
