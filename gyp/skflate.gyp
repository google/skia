# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Target for including SkFlate.
{
  'targets': [
    {
      'target_name': 'skflate',
      'type': 'static_library',
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'zlib.gyp:zlib',
      ],
      'sources': [ '../src/core/SkFlate.cpp' ],
    },
  ],
}
