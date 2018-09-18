# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# TODO(borenet): This module was copied from build.git and heavily modified to
# remove dependencies on other modules in build.git.  It belongs in a different
# repo. Remove this once it has been moved.


from recipe_engine.recipe_api import Property

DEPS = [
  'isolate',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'swarming_client',
]

PROPERTIES = {
  'always_use_exparchive': Property(
    kind=bool, help="Force usage of exparchive.", default=False),
}


def RunSteps(api, always_use_exparchive):
  # 'isolate_tests' step needs swarming checkout.
  api.swarming_client.checkout('master')

  # Code coverage for isolate_server property.
  api.isolate.isolate_server = 'https://isolateserver-dev.appspot.com'
  assert api.isolate.isolate_server == 'https://isolateserver-dev.appspot.com'

  # That would read a list of files to search for, generated in GenTests.
  step_result = api.step('read test spec', ['cat'], stdout=api.json.output())
  expected_targets = step_result.stdout

  build_path = api.isolate.package_repo_resource()
  # Generates code coverage for find_isolated_tests corner cases.
  # TODO(vadimsh): This step doesn't actually make any sense when the recipe
  # is running for real via run_recipe.py.
  api.isolate.find_isolated_tests(build_path, expected_targets)

  # Code coverage for 'isolate_tests'. 'isolated_test' doesn't support discovery
  # of isolated targets in build directory, so skip if 'expected_targets' is
  # None.
  if expected_targets is not None:
    api.isolate.isolate_tests(
        build_path, expected_targets,
        always_use_exparchive=always_use_exparchive)


def GenTests(api):
  def make_test(
          name,
          expected_batcharchive_targets,
          expected_exparchive_targets,
          discovered_targets):

    if expected_batcharchive_targets or expected_exparchive_targets:
      all_expected_targets = (
          (expected_batcharchive_targets or []) +
          (expected_exparchive_targets or []))
    else:
      all_expected_targets = None

    missing = set(all_expected_targets or []) - set(discovered_targets or [])
    output = (
        api.test(name) +
        api.step_data(
            'read test spec',
            stdout=api.json.output(all_expected_targets)) +
        api.override_step_data(
            'find isolated tests',
            api.isolate.output_json(discovered_targets))
    )

    # See comment around 'if expected_targets is not None' above.
    if all_expected_targets:
      for target in sorted(expected_exparchive_targets):
        output += api.override_step_data(
            'isolate %s' % target,
            api.isolate.output_json([target], missing))

      if expected_batcharchive_targets:
        output += api.override_step_data(
            'isolate tests',
            api.isolate.output_json(expected_batcharchive_targets, missing))

    return output

  # Expected targets == found targets.
  yield make_test(
      'basic', ['test1', 'test2'], [], ['test1', 'test2'])
  # No expectations, just discovering what's there returned by default mock.
  yield make_test(
      'discover', None, None, None)
  # Found more than expected.
  yield make_test(
      'extra', ['test1', 'test2'], [], ['test1', 'test2', 'extra_test'])
  # Didn't find something.
  yield (
      make_test('missing', ['test1', 'test2'], [], ['test1']) +
      api.properties.generic(buildername='Windows Swarm Test'))
  # No expectations, and nothing has been found, produces warning.
  yield make_test('none', None, None, [])
  # Test the `exparchive` cases
  # Only exparchive
  yield make_test(
      'exparchive', [], ['test_exparchive'], ['test_exparchive'])
  yield make_test(
      'exparchive-miss', [], ['test_exparchive'], [])
  yield make_test(
      'exparchive-multi',
      [],
      ['test1_exparchive', 'test2_exparchive'],
      ['test1_exparchive', 'test2_exparchive'])
  yield make_test(
      'exparchive-multi-miss',
      [],
      ['test1_exparchive', 'test2_exparchive'],
      ['test1_exparchive'])
  # Mixed
  yield make_test(
      'exparchive-batch',
      ['test1', 'test2'],
      ['test_exparchive'],
      ['test1', 'test2', 'test_exparchive'])
  yield make_test(
      'exparchive-batch-bmiss',
      ['test1', 'test2'],
      ['test_exparchive'],
      ['test1', 'test_exparchive'])
  yield make_test(
      'exparchive-batch-emiss',
      ['test1', 'test2'],
      ['test_exparchive'],
      ['test1', 'test2'])
  # Use force-exparchive
  yield make_test(
      'always-use-exparchive',
      [],
      ['test_exparchive', 'test1', 'test2'],
      ['test_exparchive', 'test1', 'test2']) + api.properties(
          always_use_exparchive=True)
