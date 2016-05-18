# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# GYP file for android specific codecs.
{
  'targets': [
    {
      'target_name': 'codec_android',
      'product_name': 'skia_codec_android',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'codec.gyp:codec',
      ],
      'include_dirs': [
        '../include/android',
        '../include/config',
        '../include/core',
        '../include/private',
        '../src/android',
        '../src/codec',
      ],
      'sources': [
        '../src/android/SkBitmapRegionCodec.cpp',
        '../src/android/SkBitmapRegionDecoder.cpp',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/android',
        ],
      },
    },
  ],
}
