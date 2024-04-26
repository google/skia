# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = "PY3"

DEPS = [
  'checkout',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'run',
  'vars',
]


def RunSteps(api):
  api.vars.setup()

  bot_update = True
  if 'NoDEPS' in api.properties['buildername']:
    bot_update = False

  checkout_root = api.checkout.default_checkout_root

  if bot_update:
    api.checkout.bot_update(
        checkout_root=checkout_root)
  else:
    api.checkout.git(checkout_root=api.path.start_dir)
  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)


def GenTests(api):

  builder = 'Build-Debian10-Clang-x86_64-Release-NoDEPS'
  yield (
      api.test(builder) +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     patch_issue=456789,
                     patch_set=12,
                     patch_ref='refs/changes/89/456789/12',
                     patch_repo='https://skia.googlesource.com/skia.git',
                     patch_storage='gerrit') +
      api.path.exists(api.path.start_dir.join('skp_output'))
  )

  buildername = 'Build-Debian10-Clang-x86_64-Release'
  yield (
      api.test('cross_repo_trybot') +
      api.properties(
          repository='https://skia.googlesource.com/parent_repo.git',
          buildername=buildername,
          path_config='kitchen',
          swarm_out_dir='[SWARM_OUT_DIR]',
          revision='abc123',
          patch_issue=456789,
          patch_set=12,
          patch_ref='refs/changes/89/456789/12',
          patch_repo='https://skia.googlesource.com/skia.git',
          patch_storage='gerrit')
  )
  yield (
      api.test('trybot') +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     patch_issue=456789,
                     patch_set=12,
                     patch_ref='refs/changes/89/456789/12',
                     patch_repo='https://skia.googlesource.com/skia.git',
                     patch_storage='gerrit',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
