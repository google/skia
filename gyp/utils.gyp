# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Gyp for utils.
{
  'targets': [
    {
      'target_name': 'utils',
      'product_name': 'skia_utils',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'core.gyp:*',
        'etc1.gyp:libetc1',
      ],
      'includes': [
        'utils.gypi',
      ],
      'include_dirs': [
        '../include/effects',
        '../include/gpu',
        '../include/images',
        '../include/pathops',
        '../include/private',
        '../include/utils',
        '../include/utils/mac',
        '../src/core',
        '../src/gpu',
        '../src/image',
        '../src/opts',
        '../src/utils',
        '../src/utils/win',
      ],
      'sources': [
        'utils.gypi', # Makes the gypi appear in IDEs (but does not modify the build).
      ],
      'conditions': [
        [ 'skia_os == "mac"', {
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/AGL.framework',
            ],
          },
        }],
        [ 'skia_os in ["mac", "ios"]', {
          'direct_dependent_settings': {
            'include_dirs': [
              '../include/utils/mac',
            ],
          },
        },{ #else if 'skia_os != "mac"'
          'include_dirs!': [
            '../include/utils/mac',
          ],
          'sources!': [
            '../include/utils/mac/SkCGUtils.h',
            '../src/utils/mac/SkCreateCGImageRef.cpp',
          ],
        }],
        [ 'skia_os == "win"', {
          'direct_dependent_settings': {
            'include_dirs': [
              '../src/utils/win',
            ],
          },
          'sources!': [
            '../src/utils/SkThreadUtils_pthread.cpp',
            '../src/utils/SkThreadUtils_pthread.h',
          ],
        },{ #else if 'skia_os != "win"'
          'include_dirs!': [
            '../src/utils/win',
          ],
          'sources/': [ ['exclude', '_win.(h|cpp)$'],],
          'sources!': [
            '../src/utils/win/SkAutoCoInitialize.h',
            '../src/utils/win/SkAutoCoInitialize.cpp',
            '../src/utils/win/SkDWrite.h',
            '../src/utils/win/SkDWrite.cpp',
            '../src/utils/win/SkDWriteFontFileStream.cpp',
            '../src/utils/win/SkDWriteFontFileStream.h',
            '../src/utils/win/SkDWriteGeometrySink.cpp',
            '../src/utils/win/SkDWriteGeometrySink.h',
            '../src/utils/win/SkHRESULT.h',
            '../src/utils/win/SkHRESULT.cpp',
            '../src/utils/win/SkIStream.h',
            '../src/utils/win/SkIStream.cpp',
            '../src/utils/win/SkTScopedComPtr.h',
          ],
        }],
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/utils',
          '../src/utils',
        ],
      },
    },
  ],
}
