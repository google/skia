# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


DEPS = [
  'builder_name_schema',
]


def RunSteps(api):
  name = 'Build-Ubuntu-Clang-x64-Release-Android'
  d = api.builder_name_schema.DictForBuilderName(name)
  got = api.builder_name_schema.MakeBuilderName(**d)
  assert got == name


def GenTests(api):
  yield api.test('test')
