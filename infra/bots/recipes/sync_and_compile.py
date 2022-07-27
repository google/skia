# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming compile.

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
  'recipe_engine/python',
  'recipe_engine/step',
  'run',
  'vars',
]


def RunSteps(api):
  api.vars.setup()

  # Check out code.
  bot_update = True
  checkout_root = api.checkout.default_checkout_root
  checkout_flutter = False
  flutter_android = False
  ignore_trybot = False

  if 'NoDEPS' in api.properties['buildername']:
    bot_update = False
    checkout_root = api.path['start_dir']
  if 'Flutter' in api.vars.builder_name:
    checkout_root = checkout_root.join('flutter')
    checkout_flutter = True
    if 'Android' in api.vars.builder_name:
      flutter_android = True
  if 'NoPatch' in api.vars.builder_name:
    ignore_trybot = True
    checkout_root = api.path['start_dir']

  if bot_update:
    api.checkout.bot_update(
        checkout_root=checkout_root,
        checkout_flutter=checkout_flutter,
        flutter_android=flutter_android,
        ignore_trybot=ignore_trybot)

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
      src = api.path['start_dir'].join('k', 'skia')
      dst = api.path['start_dir'].join('skia')
      api.python.inline(
          name='copy Skia repository checkout from %s to %s' % (src, dst),
          program='''
import shutil
import sys
shutil.copytree(sys.argv[1], sys.argv[2], dirs_exist_ok=True)
''',
          args=[src, dst])
      api.file.rmtree('remove %s' % src, src)

  else:
    api.checkout.git(checkout_root=checkout_root)

  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)

  out_dir = checkout_root.join(
      'skia', 'out', api.vars.builder_name, api.vars.configuration)
  if 'Flutter' in api.vars.builder_name:
    out_dir = checkout_root.join('src', 'out', 'android_release')
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
      api.python.inline(
          name='cleanup',
          program='''
# [VPYTHON:BEGIN]
# wheel: <
#  name: "infra/python/wheels/psutil/${vpython_platform}"
#  version: "version:5.4.7"
# >
# [VPYTHON:END]

import psutil
for p in psutil.process_iter():
  try:
    if p.name in ('mspdbsrv.exe', 'vctip.exe', 'cl.exe', 'link.exe'):
      p.kill()
  except psutil._error.AccessDenied:
    pass
''',
          infra_step=True,
          venv=True)

  api.run.check_failure()


TEST_BUILDERS = [
  'Build-Debian10-Clang-arm-Release-Flutter_Android',
  'Build-Debian10-Clang-arm-Release-NoPatch',
  'Build-Win10-Clang-x86_64-Release-NoDEPS',
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
