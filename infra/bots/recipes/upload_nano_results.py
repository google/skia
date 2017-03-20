# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for uploading nanobench results.


DEPS = [
  'recipe_engine/properties',
  'skia-recipes/upload_nano_results',
]


def RunSteps(api):
  api.upload_nano_results.run()


def GenTests(api):
  yield (
    api.test('upload') +
    api.properties(buildername='Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug',
                   revision='abc123',
                   path_config='kitchen')
  )
