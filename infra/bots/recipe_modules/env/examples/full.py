# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = "PY3"

DEPS = [
  'env',
  'recipe_engine/context',
  'recipe_engine/step',
]


def RunSteps(api):
  api.step('1', cmd=['echo', 'hi'])
  with api.env({'MYVAR': 'myval'}):
    api.step('2', cmd=['echo', 'hi'])

  path = 'mypath:%(PATH)s'
  with api.context(env={'PATH': path}):
    api.step('3', cmd=['echo', 'hi'])
    with api.env({'PATH': '%(PATH)s:otherpath'}):
      api.step('4', cmd=['echo', 'hi'])


def GenTests(api):
  yield api.test('test')
