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

  # Failures.
  try:
    api.builder_name_schema.MakeBuilderName('nope')
  except ValueError:
    pass

  try:
    api.builder_name_schema.MakeBuilderName(
        role='Build', os='a%sb' % api.builder_name_schema.BUILDER_NAME_SEP)
  except ValueError:
    pass

  try:
    api.builder_name_schema.MakeBuilderName(role='Build', bogus='BOGUS')
  except ValueError:
    pass

  try:
    api.builder_name_schema.MakeBuilderName(
        role='Build',
        os='Ubuntu',
        compiler='Clang',
        target_arch='x64',
        configuration='Release',
        extra_config='A%sB' % api.builder_name_schema.BUILDER_NAME_SEP)
  except ValueError:
    pass

  try:
    api.builder_name_schema.DictForBuilderName('Build-')
  except ValueError:
    pass

  try:
    api.builder_name_schema.DictForBuilderName(
        'Build-Ubuntu-Clang-x64-Release-Android-Bogus')
  except ValueError:
    pass

  try:
    api.builder_name_schema.DictForBuilderName(
        'Bogus-Ubuntu-Clang-x64-Release-Android')
  except ValueError:
    pass


def GenTests(api):
  yield api.test('test')
