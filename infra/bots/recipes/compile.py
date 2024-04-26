# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for compiling Skia when the checkout has already been done
# (e.g. repo brought in via CAS)

PYTHON_VERSION_COMPATIBILITY = "PY3"

DEPS = [
  'build',
  'checkout',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/step',
  'run',
  'vars',
]


def RunSteps(api):
  api.vars.setup()

  checkout_root = api.path.start_dir
  out_dir = api.vars.cache_dir.join(
      'work', 'skia', 'out', api.vars.builder_name, api.vars.configuration)

  try:
    api.build(checkout_root=checkout_root, out_dir=out_dir)

    # TODO(borenet): Move this out of the try/finally.
    dst = api.vars.swarming_out_dir
    api.build.copy_build_products(out_dir=out_dir, dst=dst)
  finally:
    if 'Win' in api.vars.builder_cfg.get('os', ''):
      script = api.build.resource('cleanup_win_processes.py')
      api.step(
          name='cleanup',
          cmd=['vpython3', script],
          infra_step=True)

  api.run.check_failure()


TEST_BUILDERS = [
  'Build-Win-Clang-x86-Debug',
]


def GenTests(api):
  for builder in TEST_BUILDERS:
    test = (
      api.test(builder) +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
    )
    if 'Win' in builder:
      test += api.platform('win', 64)
    yield test
