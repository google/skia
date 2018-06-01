# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for uploading skiaserve to gs://skia-public-binaries.


DEPS = [
  'flavor',
  'gsutil',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'recipe_engine/time',
  'vars',
]


def RunSteps(api):
  api.vars.setup()

  if api.properties.get('patch_issue') or api.properties.get('patch_set'):
    # Do not upload skiaserve for trybots.
    return

  src = api.vars.build_dir.join('skiaserve')
  target_arch = api.vars.builder_cfg.get('target_arch')
  dest = 'gs://skia-public-binaries/skiaserve/%s/%s/' % (
      target_arch, api.properties['revision'])
  api.gsutil.cp('skiaserve', src, dest)


def GenTests(api):
  builder = 'Build-Debian9-Clang-arm-Release-Android'
  yield (
    api.test('normal_bot') +
    api.properties(buildername=builder,
                   repository='https://skia.googlesource.com/skia.git',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   revision='abc123',
                   path_config='kitchen')
  )

  yield (
    api.test('trybot') +
    api.properties(buildername=builder,
                   repository='https://skia.googlesource.com/skia.git',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   revision='abc123',
                   path_config='kitchen',
                   patch_storage='gerrit') +
    api.properties.tryserver(
        buildername=builder,
        gerrit_project='skia',
        gerrit_url='https://skia-review.googlesource.com/',
    )
  )
