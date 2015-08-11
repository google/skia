# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This is a copy of ../third_party/externals/libjpeg/libjpeg.gyp , modified
# such that all source paths point into that directory.
# See http://code.google.com/p/skia/issues/detail?id=543 ('wrap libjpeg.gyp
# from Chrome's libjpeg port, rather than making our own copy') for a better
# long-term solution.

{
  'variables': {
    'use_system_libjpeg%': 0,
    'skia_warnings_as_errors': 0,
  },
  'conditions': [
    ['skia_os == "android"', {
      'targets': [
        {
          'target_name': 'libjpeg',
          'type': 'none',
          'dependencies': [
            'android_deps.gyp:jpeg',
          ],
          'export_dependent_settings': [
            'android_deps.gyp:jpeg',
          ],
        },
      ],
    }, { # skia_os != android
      'targets': [
        {
          'target_name': 'libjpeg',
          'type': 'none',
          'dependencies': [
            'libjpeg-turbo.gyp:*',
          ],
          'export_dependent_settings': [
            'libjpeg-turbo.gyp:*',
          ],
        },
      ],
    }],
  ],
}
