# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for compiling Skia when the checkout has already been done
# (e.g. repo brought in via CAS)


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

import hashlib

def RunSteps(api):
  api.vars.setup()

  checkout_root = api.path.start_dir

  # We changed the names of these paths. To reduce disk space from the old
  # paths, we'll try to delete them as we go.
  old_path = api.vars.cache_dir.joinpath(
      'work', 'skia', 'out', api.vars.builder_name)
  # If the folder does not exist, this returns without error.
  # //skia/infra/bots/.recipe_deps/recipe_engine/recipe_modules/file/api.py
  api.file.rmtree('delete old caches %s' % old_path, old_path)

  # Use a shortened output directory path to avoid hitting path length
  # limits on Windows (250 chars)
  long_name = api.vars.builder_name + api.vars.configuration
  short_name = hashlib.md5(long_name.encode('utf-8')).hexdigest()[:6]
  out_dir = api.vars.cache_dir.joinpath(
      'work', 'skia', 'out', short_name)

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
