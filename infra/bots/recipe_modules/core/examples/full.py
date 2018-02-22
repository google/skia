# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


DEPS = [
  'core',
  'recipe_engine/path',
  'recipe_engine/properties',
]


def RunSteps(api):
  bot_update = True
  if 'NoDEPS' in api.properties['buildername']:
    bot_update = False
  api.core.setup(bot_update=bot_update)


def GenTests(api):
  buildername = 'Build-Win-Clang-x86_64-Release-Vulkan'
  yield (
      api.test('test') +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.properties(patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=buildername,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      )
    )

  buildername = 'Build-Win-Clang-x86_64-Release-ParentRevision'
  yield (
      api.test('parent_revision') +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
      api.test('parent_revision_trybot') +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     patch_issue=500,
                     patch_set=1,
                     patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=buildername,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      )
  )

  buildername = ('Test-Win10-Clang-NUC6i5SYK-GPU-IntelIris540-x86_64-' +
                 'Debug-All-ANGLE')
  yield (
      api.test('no_persistent_checkout') +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.properties(patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=buildername,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      )
    )

  buildername = 'Build-Debian9-GCC-x86_64-Release-PDFium'
  yield (
      api.test('pdfium_trybot') +
      api.properties(
          repository='https://skia.googlesource.com/skia.git',
          buildername=buildername,
          path_config='kitchen',
          swarm_out_dir='[SWARM_OUT_DIR]',
          revision='abc123',
          patch_issue=500,
          patch_set=1,
          patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=buildername,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      ) +
      api.path.exists(
          api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
      )
  )

  buildername = 'Build-Debian9-GCC-x86_64-Release-Flutter_Android'
  yield (
      api.test('flutter_trybot') +
      api.properties(
          repository='https://skia.googlesource.com/skia.git',
          buildername=buildername,
          path_config='kitchen',
          swarm_out_dir='[SWARM_OUT_DIR]',
          revision='abc123',
          patch_issue=500,
          patch_set=1,
          patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=buildername,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      ) +
      api.path.exists(
          api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
      )
  )

  builder = 'Housekeeper-Weekly-RecreateSKPs'
  yield (
      api.test(builder) +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(api.path['start_dir'].join('skp_output'))
  )

  builder = 'Build-Debian9-Clang-x86_64-Release-NoDEPS'
  yield (
      api.test(builder) +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     patch_issue=500,
                     patch_repo='https://skia.googlesource.com/skia.git',
                     patch_set=1,
                     patch_storage='gerrit') +
      api.path.exists(api.path['start_dir'].join('skp_output'))
  )

  buildername = 'Build-Debian9-GCC-x86_64-Release'
  yield (
      api.test('cross_repo_trybot') +
      api.properties(
          repository='https://skia.googlesource.com/parent_repo.git',
          buildername=buildername,
          path_config='kitchen',
          swarm_out_dir='[SWARM_OUT_DIR]',
          revision='abc123',
          patch_issue=500,
          patch_repo='https://skia.googlesource.com/skia.git',
          patch_set=1,
          patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=buildername,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      ) +
      api.path.exists(
          api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
      )
  )
