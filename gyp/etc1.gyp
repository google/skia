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
    'target_name': 'libetc1',
    'type': 'static_library',
    'include_dirs' : [
      '../third_party/externals/android_native/opengl/include/'
    ],
    'sources': [
      '../third_party/externals/android_native/opengl/libs/ETC1/etc1.cpp'
    ],
    'direct_dependent_settings': {
      'include_dirs' : [
        '../third_party/externals/android_native/opengl/include/ETC1'
      ],
    },
  }],
}
