# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for compiling Skia when we need to get a full Skia checkout.

PYTHON_VERSION_COMPATIBILITY = "PY3"

DEPS = [
  'build',
  'checkout',
  'infra',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/step',
  'depot_tools/gitiles',
  'run',
  'vars',
]


def RunSteps(api):
  api.vars.setup()

  # Check out code.
  bot_update = True
  checkout_root = api.checkout.default_checkout_root
  skip_patch = False
  revision = api.properties['revision']

  if 'NoDEPS' in api.properties['buildername']:
    bot_update = False
    checkout_root = api.path.start_dir
  if 'NoPatch' in api.vars.builder_name:
    skip_patch = True
    checkout_root = api.path.start_dir

    # If we are running on the CI (post submit), we want to do a diff with the
    # previous commit. To do this, we use gitiles to look up the current
    # git revision, and find its parent. In the unlikely event of there being
    # multiple parents, we pick the first one arbitrarily.
    if not api.vars.is_trybot:
      # Fetches something like
      # https://skia.googlesource.com/skia.git/+log/b44572fbfeb669998053b023f473b9c274f2f2cf?format=JSON
      #
      # https://chromium.googlesource.com/chromium/tools/depot_tools/+/dca14bc463857bd2a0fee59c86ffa289b535d5d3/recipes/recipe_modules/gitiles/api.py#75
      response, _ = api.gitiles.log(
        url = api.properties['repository'],
        ref = api.properties['revision'],
        limit = 1)
      # Response looks like:
      #     [{
      #        'parents': ['<githash>'],
      #        ...
      #     }]
      revision  = response[0]['parents'][0]

  if bot_update:
    api.checkout.bot_update(
        checkout_root=checkout_root,
        skip_patch=skip_patch,
        override_revision=revision)

    if 'NoPatch' in api.vars.builder_name:
      # The CodeSize-* family of tasks compute size diffs between the binaries produced by
      # Build-<CONFIG>-NoPatch tasks and those produced by Build-<CONFIG> tasks. Some debug strings
      # in said binaries might contain relative paths from the output directory to the sources
      # directory (e.g. "../../../dm/DM.cpp"). In order to prevent spurious deltas, we must make
      # the Build-<CONFIG>-NoPatch tasks match the paths used by Build-<CONFIG> tasks. For example,
      # Build-<CONFIG> tasks place the Skia checkout at /mnt/pd0/s/w/ir/skia, so
      # Build-<CONFIG>-NoPatch tasks must do the same.
      #
      # For some reason api.checkout.bot_update places the Skia checkout at /mnt/pd0/s/w/ir/k
      # even though we specified /mnt/pd0/s/w/ir as the checkout root. As a workaround, we manually
      # copy the Skia checkout to the intended location.
      #
      # An inline Python script is necessary here because api.file.copytree[1] does not pipe
      # through the dirs_exist_ok argument to the underlying shutil.copytree[2] call.
      #
      # [1] https://chromium.googlesource.com/infra/luci/recipes-py.git/+/cfdb92cc6933d8f72c2340233ba03b602b447507/recipe_modules/file/api.py#146
      # [2] https://docs.python.org/3/library/shutil.html#shutil.copytree
      src = api.path.start_dir.join('k', 'skia')
      dst = api.path.start_dir.join('skia')
      script = api.infra.resource('copytree.py')
      api.step(
          name='copy Skia repository checkout from %s to %s' % (src, dst),
          cmd=['python3', script, src, dst])
      api.file.rmtree('remove %s' % src, src)

  else:
    api.checkout.git(checkout_root=checkout_root)

  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)

  out_dir = checkout_root.join(
      'skia', 'out', api.vars.builder_name, api.vars.configuration)
  if 'NoPatch' in api.vars.builder_name:
    # Similarly as with the checkout root, we use the same output directory in
    # Build-<CONFIG>-NoPatch tasks as we do on Build-<CONFIG> tasks to prevent spurious deltas.
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


def GenTests(api):
  yield (
      api.test('Build-Win10-Clang-x86_64-Release-NoDEPS') +
      api.properties(buildername='Build-Win10-Clang-x86_64-Release-NoDEPS',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.platform('win', 64)
  )

  yield (
      # git revisions based off of real data
      api.test('Build-Debian10-Clang-arm-Release-NoPatch') +
      api.properties(buildername='Build-Debian10-Clang-arm-Release-NoPatch',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='b44572fbfeb669998053b023f473b9c274f2f2cf',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      # This tells recipes to use this fake data for a step with the given
      # name. Inspired by
      # https://chromium.googlesource.com/chromium/tools/depot_tools/+/dca14bc463857bd2a0fee59c86ffa289b535d5d3/recipes/recipe_modules/gitiles/examples/full.py#62
      # Even though we use the commit 6e0e0... as the "seed string", the actual
      # commit (and parent commit) returned from make_log_test_data is
      # different. Mocked commit is 188e23c7abc4b205f0f80fb345ff63ec5b716be8
      # and parent commit is d2231a340fd47b47d61d0f99a188e46e6aabba0a
      api.step_data(
          'gitiles log: b44572fbfeb669998053b023f473b9c274f2f2cf',
          api.gitiles.make_log_test_data('6e0e0a9f6cbf09078aa4730d1a0dc0aa722ddc11'),
      )
    )

  yield (
      api.test('Build-Debian10-Clang-arm-Release-NoPatch (tryjob)') +
      api.properties(buildername='Build-Debian10-Clang-arm-Release-NoPatch',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     patch_issue=123,
                     patch_set=123,
                     patch_ref=123)
    )

