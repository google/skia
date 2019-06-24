# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe which runs the Skia infra tests.


DEPS = [
  'checkout',
  'infra',
  'recipe_engine/context',
  'recipe_engine/properties',
  'recipe_engine/step',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  checkout_root = api.checkout.default_checkout_root
  api.checkout.bot_update(checkout_root=checkout_root)

  # Run the infra tests.
  repo_name = api.properties['repository'].split('/')[-1]
  if repo_name.endswith('.git'):
    repo_name = repo_name[:-len('.git')]
  repo_root = checkout_root.join(repo_name)
  infra_tests = repo_root.join('infra', 'bots', 'infra_tests.py')

  # Merge the default environment with the Go environment.
  env = {}
  env.update(api.infra.go_env)
  for k, v in api.vars.default_env.iteritems():
    # The PATH variable gets merged; all others get replaced.
    if k == 'PATH':
      # This works because the value for PATH in go_env and default_env includes
      # the '%(PATH)s' placeholder.
      env[k] = env[k] % {k: v}
    else:
      env[k] = v

  with api.context(cwd=checkout_root.join(repo_name), env=env):
    # Unfortunately, the recipe tests are flaky due to file removal on Windows.
    # Run multiple attempts.
    last_exc = None
    for _ in range(3):
      try:
        api.step('infra_tests', cmd=['python', '-u', infra_tests])
        break
      except api.step.StepFailure as e:  # pragma: nocover
        last_exc = e
    else:  # pragma: nocover
      raise last_exc

def GenTests(api):
  yield (
      api.test('infra_tests') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests_Win',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
