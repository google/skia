# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe which runs the Skia infra tests.


DEPS = [
  'core',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'vars',
]


UPDATE_GO_ATTEMPTS = 5


def RunSteps(api):
  api.vars.setup()
  api.core.checkout_steps()

  # Attempt to update go dependencies. This fails flakily sometimes, so perform
  # multiple attempts.
  gopath = api.vars.checkout_root.join('gopath')
  env = {'GOPATH': gopath}
  name = 'update go pkgs'
  for attempt in xrange(UPDATE_GO_ATTEMPTS):
    step_name = name
    if attempt > 0:
      step_name += ' (attempt %d)' % (attempt + 1)
    try:
      api.step(step_name,
               cmd=['go', 'get', '-u', 'go.skia.org/infra/...'],
               env=env)
      break
    except api.step.StepFailure:
      if attempt == UPDATE_GO_ATTEMPTS - 1:
        raise

  # Run the infra tests.
  infra_tests = api.vars.skia_dir.join(
      'infra', 'bots', 'infra_tests.py')
  api.step('infra_tests',
           cmd=['python', infra_tests],
           cwd=api.vars.skia_dir,
           env=env)


def GenTests(api):
  yield (
      api.test('infra_tests') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests',
                     mastername='client.skia.fyi',
                     slavename='dummy-slave',
                     buildnumber=5,
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
    api.test('failed_one_update') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests',
                     mastername='client.skia.fyi',
                     slavename='dummy-slave',
                     buildnumber=5,
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
    api.step_data('update go pkgs', retcode=1)
  )

  yield (
    api.test('failed_all_updates') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests',
                     mastername='client.skia.fyi',
                     slavename='dummy-slave',
                     buildnumber=5,
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
    api.step_data('update go pkgs', retcode=1) +
    api.step_data('update go pkgs (attempt 2)', retcode=1) +
    api.step_data('update go pkgs (attempt 3)', retcode=1) +
    api.step_data('update go pkgs (attempt 4)', retcode=1) +
    api.step_data('update go pkgs (attempt 5)', retcode=1)
  )
