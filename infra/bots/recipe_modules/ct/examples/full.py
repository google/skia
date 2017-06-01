# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


DEPS = [
  'ct',
  'recipe_engine/path',
]


def RunSteps(api):
  api.ct.download_swarming_skps(
      'All', '0', 'abc123',
      api.path['start_dir'].join('skps'),
      start_range=100,
      num_skps=100)


def GenTests(api):
  yield api.test('test')
  yield (
      api.test('failed_gsutil') +
      api.step_data('gsutil cp', retcode=1)
  )
