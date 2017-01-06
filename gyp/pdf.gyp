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
      'sources': [ '<(skia_src_path)/pdf/SkDocument_PDF_None.cpp', ],
    },
    {
      'target_name': 'pdf',
      'product_name': 'skia_pdf',
      'type': 'static_library',
      'standalone_static_library': 1,
      'variables': {
        'skia_pdf_use_sfntly%': 1,
      },
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'zlib.gyp:zlib',
      ],
      'include_dirs': [
        '../include/private',
        '../src/core', # needed to get SkGlyphCache.h and SkTextFormatParams.h
        '../src/image',
        '../src/utils', # needed to get SkBitSet.h
      ],
      'sources': [
        '<!@(python read_gni.py ../gn/pdf.gni skia_pdf_sources)',
      ],
      'conditions': [
        [ 'skia_pdf_use_sfntly and not skia_android_framework and \
           skia_os in ["win", "android", "linux", "mac"]',
          {
            'dependencies': [ 'sfntly.gyp:sfntly' ],
            'defines': [ 'SK_PDF_USE_SFNTLY' ],
          }
        ],
        [ 'skia_android_framework', {
            # Add SFTNLY support for PDF (which in turns depends on ICU)
            'include_dirs': [ 'external/sfntly/cpp/src' ],
            'defines': [ 'SK_PDF_USE_SFNTLY' ],
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
