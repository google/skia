# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe which runs the Skia infra tests.


DEPS = [
  'recipe_engine/context',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'core',
  'infra',
  'run',
  'vars',
]


def update_go_deps(api):
  env = api.context.env
  env.update(api.infra.go_env)
  with api.context(env=env):
    api.run.with_retry(
        api.step,
        'update go pkgs',
        5,  # Update attempts.
        cmd=[api.infra.go_exe, 'get', '-u', '-t',
             'go.skia.org/infra/fiddle/go/fiddlecli'])


def RunSteps(api):
  api.vars.setup()
  api.core.checkout_steps()
  api.infra.go_version()
  update_go_deps(api)  # inline this?

  # Run the infra tests.
  print 'hhhhhh'
  print api.vars.swarming_out_dir
  print 'Try to find bookmaker so that you can execte it!'
  print 'Make sure fiddlecli now exists!'
  print 'Sleeping for 10 mins!'
  import time
  time.sleep(600)

  """
  repo_name = api.properties['repository'].split('/')[-1]
  if repo_name.endswith('.git'):
    repo_name = repo_name[:-len('.git')]
  with api.context(cwd=api.vars.checkout_root.join(repo_name),
                   env=api.infra.go_env):
    api.step('infra_tests', cmd=['make', '-C', 'infra/bots', 'test'])
  """


def GenTests(api):
  yield (
      api.test('bookmaker') +
      api.properties(buildername='Housekeeper-PerCommit-Bookmaker',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
  """
  yield (
    api.test('failed_one_update') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
    api.step_data('update go pkgs', retcode=1)
  )

  yield (
    api.test('failed_all_updates') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
    api.step_data('update go pkgs', retcode=1) +
    api.step_data('update go pkgs (attempt 2)', retcode=1) +
    api.step_data('update go pkgs (attempt 3)', retcode=1) +
    api.step_data('update go pkgs (attempt 4)', retcode=1) +
    api.step_data('update go pkgs (attempt 5)', retcode=1)
  )
  """
