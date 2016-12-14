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
        'libjpeg-turbo-selector.gyp:libjpeg-turbo-selector',
        'etc1.gyp:libetc1',
        'libpng.gyp:libpng',
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
        '../src/images/SkWEBPImageEncoder.cpp',
        '../src/images/SkJPEGImageEncoder.cpp',
        '../src/images/SkPNGImageEncoder.cpp',

        '../src/images/SkImageEncoder.cpp',
        '../src/images/SkJPEGWriteUtility.cpp',

        '../src/ports/SkImageEncoder_CG.cpp',
        '../src/ports/SkImageEncoder_WIC.cpp',
      ],
      'conditions': [
        [ 'skia_os == "win"', {
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
        },{ #else if skia_os != mac
          'sources!': [
            '../src/ports/SkImageEncoder_CG.cpp',
          ],
        }],
        [ 'skia_os == "android"', {
          'include_dirs': [
             '../src/utils',
          ],
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
