# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Example recipe w/ coverage.


DEPS = [
  'recipe_engine/properties',
  'upload_nano_results',
]


def RunSteps(api):
  api.upload_nano_results.run()


def GenTests(api):
  builder = 'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug'
  yield (
    api.test('normal_bot') +
    api.properties(buildername=builder,
                   gs_bucket='skia-perf',
                   revision='abc123',
                   path_config='kitchen')
  )

  builder = 'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-Trybot'
  yield (
    api.test('trybot') +
    api.properties(buildername=builder,
                   gs_bucket='skia-perf',
                   revision='abc123',
                   path_config='kitchen',
                   issue='12345',
                   patchset='1002')
  )

  yield (
      api.test('recipe_with_gerrit_patch') +
      api.properties(
          buildername=builder,
          gs_bucket='skia-perf',
          revision='abc123',
          path_config='kitchen',
          patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=builder,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      )
  )
