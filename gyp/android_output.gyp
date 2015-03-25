# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# GYP file to send android SkDebug to stdout in addition to logcat. To enable,
# include this project as a dependency.
{
  'targets': [
    {
      'target_name': 'android_output',
      'type': 'static_library',
      'sources': [
        '../tools/AndroidSkDebugToStdOut.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
    },
  ],
}
