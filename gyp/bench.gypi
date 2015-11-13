# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'include_dirs': [
    '../bench/subset',
    '../bench',
    '../include/private',
    '../src/core',
    '../src/effects',
    '../src/gpu',
    '../src/utils',
  ],
  'sources': [ '<!@(python find.py ../bench "*.cpp")' ],

  'dependencies': [
    'etc1.gyp:libetc1',
    'skia_lib.gyp:skia_lib',
    'tools.gyp:resources',
    'tools.gyp:sk_tool_utils',
  ],
  'conditions': [
    ['skia_gpu == 1', {
      'include_dirs': [ '../src/gpu' ],
      'dependencies': [ 'gputest.gyp:skgputest' ],
    }],
    ['not skia_android_framework', {
        'sources!': [ '../bench/nanobenchAndroid.cpp' ],
    }],
  ],
}
