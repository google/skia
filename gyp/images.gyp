# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# GYP file for images project.
{
  'targets': [
    {
      'target_name': 'images',
      'product_name': 'skia_images',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'core.gyp:*',
        'giflib.gyp:giflib',
        'libjpeg.gyp:*',
        'etc1.gyp:libetc1',
        'ktx.gyp:libSkKTX',
        'libwebp.gyp:libwebp',
        'utils.gyp:utils',
      ],
      'include_dirs': [
        '../include/images',
        '../include/private',
        '../src/lazy',
        # for access to SkErrorInternals.h
        '../src/core/',
        # for access to SkImagePriv.h
        '../src/image/',
      ],
      'sources': [
        '../include/images/SkDecodingImageGenerator.h',
        '../include/images/SkForceLinking.h',
        '../src/images/SkJpegUtility.h',
        '../include/images/SkMovie.h',
        '../include/images/SkPageFlipper.h',

        '../src/images/bmpdecoderhelper.cpp',
        '../src/images/bmpdecoderhelper.h',

        '../src/images/SkDecodingImageGenerator.cpp',
        '../src/images/SkForceLinking.cpp',
        '../src/images/SkImageDecoder.cpp',
        '../src/images/SkImageDecoder_FactoryDefault.cpp',
        '../src/images/SkImageDecoder_FactoryRegistrar.cpp',

        # If decoders are added/removed to/from (all/individual)
        # platform(s), be sure to update SkForceLinking.cpp
        # so the right decoders will be forced to link.

        # IMPORTANT: The build order of the SkImageDecoder_*.cpp files
        # defines the order image decoders are tested when decoding a
        # stream. The last decoder is the first one tested, so the .cpp
        # files should be in listed in order from the least likely to be
        # used, to the most likely (jpeg and png should be the last two
        # for instance.) As a result, they are deliberately not in
        # alphabetical order.
        '../src/images/SkImageDecoder_wbmp.cpp',
        '../src/images/SkImageDecoder_pkm.cpp',
        '../src/images/SkImageDecoder_ktx.cpp',
        '../src/images/SkImageDecoder_astc.cpp',
        '../src/images/SkImageDecoder_libbmp.cpp',
        '../src/images/SkImageDecoder_libgif.cpp',
        '../src/images/SkImageDecoder_libico.cpp',
        '../src/images/SkImageDecoder_libwebp.cpp',
        '../src/images/SkImageDecoder_libjpeg.cpp',
        '../src/images/SkImageDecoder_libpng.cpp',

        '../src/images/SkImageEncoder.cpp',
        '../src/images/SkImageEncoder_Factory.cpp',
        '../src/images/SkImageEncoder_argb.cpp',
        '../src/images/SkJpegUtility.cpp',
        '../src/images/SkMovie.cpp',
        '../src/images/SkMovie_gif.cpp',
        '../src/images/SkPageFlipper.cpp',
        '../src/images/SkScaledBitmapSampler.cpp',
        '../src/images/SkScaledBitmapSampler.h',

        '../src/ports/SkImageGenerator_skia.cpp',

        '../src/ports/SkImageDecoder_CG.cpp',
        '../src/ports/SkImageDecoder_WIC.cpp',
      ],
      'conditions': [
        [ 'skia_os == "win"', {
          'sources!': [
            '../src/images/SkImageDecoder_FactoryDefault.cpp',
            '../src/images/SkImageDecoder_libgif.cpp',
            '../src/images/SkImageDecoder_libpng.cpp',
            '../src/images/SkMovie_gif.cpp',
          ],
          'dependencies!': [
            'giflib.gyp:giflib'
          ],
          'link_settings': {
            'libraries': [
              '-lwindowscodecs.lib',
            ],
          },
        },{ #else if skia_os != win
          'sources!': [
            '../src/ports/SkImageDecoder_WIC.cpp',
          ],
        }],
        [ 'skia_os in ["mac", "ios"]', {
          'sources!': [
            '../src/images/SkImageDecoder_FactoryDefault.cpp',
            '../src/images/SkImageDecoder_libpng.cpp',
            '../src/images/SkImageDecoder_libgif.cpp',
            '../src/images/SkMovie_gif.cpp',
          ],
        },{ #else if skia_os != mac
          'sources!': [
            '../src/ports/SkImageDecoder_CG.cpp',
          ],
        }],
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris"]', {
          'export_dependent_settings': [
            'libpng.gyp:libpng',
          ],
          'dependencies': [
            'libpng.gyp:libpng',
          ],
          # end libpng stuff
        }],
        [ 'skia_os == "android"', {
          'include_dirs': [
             '../src/utils',
          ],
          'dependencies': [
             'android_deps.gyp:png',
          ],
          'conditions': [
            [ 'skia_android_framework == 0', {
              'export_dependent_settings': [
                'android_deps.gyp:png',
                'libjpeg.gyp:*'
              ],
            }, {
              # The android framework disables these decoders as they are of little use to
              # Java applications that can't take advantage of the compressed formats.
              'sources!': [
                '../src/images/SkImageDecoder_pkm.cpp',
                '../src/images/SkImageDecoder_ktx.cpp',
                '../src/images/SkImageDecoder_astc.cpp',
              ],
            }],
          ],
        }],
        [ 'skia_os == "chromeos"', {
          'dependencies': [
            'libpng.gyp:libpng',
          ],
        }],
        [ 'skia_os == "ios"', {
           'include_dirs': [
             '../include/utils/mac',
           ],
        }],
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/images',
        ],
      },
    },
  ],
}
