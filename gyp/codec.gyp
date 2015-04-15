# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# GYP file for codec project.
{
  'targets': [
    {
      'target_name': 'codec',
      'product_name': 'skia_codec',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'core.gyp:*',
        'giflib.gyp:giflib',
        'libjpeg.gyp:libjpeg',
      ],
      'cflags':[
        # FIXME: This gets around a longjmp warning. See
        # http://build.chromium.org/p/client.skia.compile/builders/Build-Ubuntu-GCC-x86_64-Release-Trybot/builds/113/steps/build%20most/logs/stdio
        '-Wno-clobbered',
      ],
      'include_dirs': [
        '../include/codec',
        '../src/codec',
        '../src/core',
      ],
      'sources': [
        '../src/codec/SkCodec.cpp',
        '../src/codec/SkCodec_libbmp.cpp',
        '../src/codec/SkCodec_libgif.cpp',
        '../src/codec/SkCodec_libico.cpp',
        '../src/codec/SkCodec_libpng.cpp',
        '../src/codec/SkCodec_wbmp.cpp',
        '../src/codec/SkGifInterlaceIter.cpp',
        '../src/codec/SkJpegCodec.cpp',
        '../src/codec/SkJpegDecoderMgr.cpp',
        '../src/codec/SkJpegUtility.cpp',
        '../src/codec/SkMaskSwizzler.cpp',
        '../src/codec/SkMasks.cpp',
        '../src/codec/SkSwizzler.cpp',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/codec',
        ],
      },
      'conditions': [
        [ 'skia_android_framework == 1',
          {
            # TODO(djsollen): this is a temporary dependency until we can update
            # the android framework to a more recent version of libpng.
            'dependencies': [
              'libpng.gyp:libpng',
            ],
          }, {  # !skia_android_framework
            'dependencies': [
              'libpng.gyp:libpng_static',
            ],
          }
        ]
      ]
    },
  ],
}
