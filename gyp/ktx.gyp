# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'skia_warnings_as_errors': 0,
  },
  'targets': [
  {
    'target_name': 'libSkKTX',
    'type': 'static_library',
    'include_dirs' : [
      '../third_party/ktx',
      '../include/gpu',
      '../include/private',
      '../src/core',
      '../src/gpu',
      '../src/utils',
    ],
    'sources': [
      '../third_party/ktx/ktx.cpp',
    ],
    'dependencies': [
      'core.gyp:*',
      'etc1.gyp:libetc1',
    ],
    'direct_dependent_settings': {
      'include_dirs': [
        '../third_party/ktx',
      ],
    },
  }],
}
