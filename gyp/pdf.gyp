# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# This file builds the PDF backend.
{
  'targets': [
    {
      'target_name': 'nopdf',
      'type': 'static_library',
      'dependencies': [ 'skia_lib.gyp:skia_lib', ],
      'sources': [ '<(skia_src_path)/doc/SkDocument_PDF_None.cpp', ],
      'defines': [ 'SK_SUPPORT_PDF=0', ],
    },
    {
      'target_name': 'pdf',
      'product_name': 'skia_pdf',
      'type': 'static_library',
      'standalone_static_library': 1,
      'variables': { 'skia_pdf_use_sfntly%': 1, },
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'zlib.gyp:zlib',
      ],
      'includes': [
        'pdf.gypi',
      ],
      'include_dirs': [
        '../include/private',
        '../src/core', # needed to get SkGlyphCache.h and SkTextFormatParams.h
        '../src/pdf',
        '../src/image',
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
        [ 'skia_pdf_generate_pdfa', { 'defines': ['SK_PDF_GENERATE_PDFA'] } ],
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
        'defines': [ 'SK_SUPPORT_PDF=1', ],
        'include_dirs': [
          '../include/core',  # SkDocument.h
        ],
      },
    },
  ],
}
