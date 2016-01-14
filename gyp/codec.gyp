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
        'libjpeg-turbo-selector.gyp:libjpeg-turbo-selector',
        'libpng.gyp:libpng',
        'libwebp.gyp:libwebp',
      ],
      'cflags':[
        # FIXME: This gets around a longjmp warning. See
        # http://build.chromium.org/p/client.skia.compile/builders/Build-Ubuntu-GCC-x86_64-Release-Trybot/builds/113/steps/build%20most/logs/stdio
        '-Wno-clobbered -Wno-error',
      ],
      'include_dirs': [
        '../include/codec',
        '../include/private',
        '../src/codec',
        '../src/core',
        '../src/utils',
      ],
      'sources': [
        '../src/codec/SkAndroidCodec.cpp',
        '../src/codec/SkBmpCodec.cpp',
        '../src/codec/SkBmpMaskCodec.cpp',
        '../src/codec/SkBmpRLECodec.cpp',
        '../src/codec/SkBmpStandardCodec.cpp',
        '../src/codec/SkCodec.cpp',
        '../src/codec/SkCodec_libpng.cpp',
        '../src/codec/SkGifCodec.cpp',
        '../src/codec/SkIcoCodec.cpp',
        '../src/codec/SkJpegCodec.cpp',
        '../src/codec/SkJpegDecoderMgr.cpp',
        '../src/codec/SkJpegUtility_codec.cpp',
        '../src/codec/SkMaskSwizzler.cpp',
        '../src/codec/SkMasks.cpp',
        '../src/codec/SkSampler.cpp',
        '../src/codec/SkSampledCodec.cpp',
        '../src/codec/SkSwizzler.cpp',
        '../src/codec/SkWbmpCodec.cpp',
        '../src/codec/SkWebpAdapterCodec.cpp',
        '../src/codec/SkWebpCodec.cpp',

        '../src/codec/SkCodecImageGenerator.cpp',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/codec',
        ],
      },
      'defines': [
        'TURBO_HAS_SKIP',
      ],
    },
  ],
}
