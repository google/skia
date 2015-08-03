# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# This file builds the PDF backend.
{
  'targets': [
    {
      'target_name': 'pdf',
      'product_name': 'skia_pdf',
      'type': 'static_library',
      'standalone_static_library': 1,
      'variables': { 'skia_pdf_use_sfntly%': 1, },
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'skflate.gyp:skflate',
      ],
      'includes': [
        'pdf.gypi',
      ],
      'include_dirs': [
        '../include/private',
        '../src/core', # needed to get SkGlyphCache.h and SkTextFormatParams.h
        '../src/pdf',
        '../src/utils', # needed to get SkBitSet.h
      ],
      'sources': [
        'pdf.gypi', # Makes the gypi appear in IDEs (but does not modify the build).
      ],
      'conditions': [
        [ 'skia_pdf_use_sfntly and not skia_android_framework and \
           skia_os in ["win", "android", "linux", "chromeos", "mac"]',
          { 'dependencies': [ 'sfntly.gyp:sfntly' ] }
        ],
        [ 'skia_android_framework', {
            # Add SFTNLY support for PDF (which in turns depends on ICU)
            'include_dirs': [
              'external/sfntly/cpp/src',
            ],
            'libraries': [
              'libsfntly.a',
              '-licuuc',
              '-licui18n',
            ],
          }
        ],
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/core',  # SkDocument.h
        ],
      },
    },
  ],
}
