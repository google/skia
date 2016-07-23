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
        'libjpeg-turbo-selector.gyp:libjpeg-turbo-selector',
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
        '../include/images/SkForceLinking.h',
        '../include/images/SkMovie.h',

        '../src/images/SkForceLinking.cpp',
        '../src/images/SkMovie_FactoryDefault.cpp',

        # If encoders are added/removed to/from (all/individual)
        # platform(s), be sure to update SkForceLinking.cpp
        # so the right decoders will be forced to link.

        '../src/images/SkKTXImageEncoder.cpp',
        '../src/images/SkWEBPImageEncoder.cpp',
        '../src/images/SkJPEGImageEncoder.cpp',
        '../src/images/SkPNGImageEncoder.cpp',

        '../src/images/SkImageEncoder.cpp',
        '../src/images/SkImageEncoder_Factory.cpp',
        '../src/images/SkARGBImageEncoder.cpp',
        '../src/images/SkJPEGWriteUtility.cpp',
        '../src/images/SkMovie.cpp',
        '../src/images/SkGIFMovie.cpp',

        '../src/ports/SkImageEncoder_CG.cpp',
        '../src/ports/SkImageEncoder_WIC.cpp',
      ],
      'conditions': [
        [ 'skia_os == "win"', {
          'sources!': [
            '../src/images/SkPNGImageEncoder.cpp',
            '../src/images/SkGIFMovie.cpp',
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
            '../src/ports/SkImageEncoder_WIC.cpp',
          ],
        }],
        [ 'skia_os in ["mac", "ios"]', {
          'sources!': [
            '../src/images/SkPNGImageEncoder.cpp',
            '../src/images/SkGIFMovie.cpp',
          ],
        },{ #else if skia_os != mac
          'sources!': [
            '../src/ports/SkImageEncoder_CG.cpp',
          ],
        }],
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris"]', {
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
            'libpng.gyp:libpng',
          ],
          'conditions': [
            [ 'skia_android_framework == 1', {
              # The android framework disables these decoders as they are of little use to
              # Java applications that can't take advantage of the compressed formats.
              'sources!': [
                '../src/images/SkKTXImageEncoder.cpp',
              ],
            }],
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
