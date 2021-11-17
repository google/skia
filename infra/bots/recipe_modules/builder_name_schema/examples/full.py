# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = "PY2+3"

DEPS = [
  'builder_name_schema',
]


def RunSteps(api):
  names = [
    'Build-Debian10-Clang-x64-Release-Android',
    'Upload-Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-Shard_12-Coverage',
  ]
  for name in names:
    d = api.builder_name_schema.DictForBuilderName(name)
    got = api.builder_name_schema.MakeBuilderName(**d)
    assert got == name

  # Failures.
  try:
    api.builder_name_schema.MakeBuilderName(role='nope')
  except ValueError:
    pass

  try:
    api.builder_name_schema.MakeBuilderName(compiler='Build', os='ab')
  except ValueError:
    pass

  try:
    api.builder_name_schema.MakeBuilderName(role='Build', bogus='BOGUS')
  except ValueError:
    pass

  try:
    api.builder_name_schema.MakeBuilderName(
        role='Build',
        os='Debian10',
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
        'Build-Debian10-Clang-x64-Release-Android-Bogus')
  except ValueError:
    pass

  try:
    api.builder_name_schema.DictForBuilderName(
        'Bogus-Debian10-Clang-x64-Release-Android')
  except ValueError:
    pass

  try:
    api.builder_name_schema.MakeBuilderName(role='Upload')
  except ValueError:
    pass

  try:
    m = {
      'role': 'Upload',
      'sub-role-1': 'fake',
    }
    api.builder_name_schema.MakeBuilderName(**m)
  except ValueError:
    pass

  try:
    api.builder_name_schema.MakeBuilderName(
        role='Build',
        os='Debian10',
        compiler='Clang',
        target_arch='x64',
        configuration='Release',
        extra_config='Android',
        extra_extra_config='Bogus',
    )
  except ValueError:
    pass

def GenTests(api):
  yield api.test('test')
