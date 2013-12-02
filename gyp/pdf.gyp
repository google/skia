{
  'targets': [
    {
      'target_name': 'pdf',
      'product_name': 'skia_pdf',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'zlib.gyp:zlib',
      ],
      'includes': [
        'pdf.gypi',
      ],
      'include_dirs': [
        '../include/pdf',
        '../src/core', # needed to get SkGlyphCache.h and SkTextFormatParams.h
        '../src/pdf',
        '../src/utils', # needed to get SkBitSet.h
      ],
      'sources': [
        'pdf.gypi', # Makes the gypi appear in IDEs (but does not modify the build).

        '../src/doc/SkDocument_PDF.cpp', # Chromium does use this file
      ],
      # This section makes all targets that depend on this target
      # #define SK_SUPPORT_PDF and have access to the pdf header files.
      'direct_dependent_settings': {
        'defines': [
          'SK_SUPPORT_PDF',
        ],
        'include_dirs': [
          '../include/pdf',
        ],
      },
    },
  ],
}
