# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

DEPS = [
  'isolate',
]


def RunSteps(api):
  api.isolate.run_isolated('run_isolated', 'isolate_hash', ['some', 'args'])


def GenTests(api):
  yield api.test('basic')
