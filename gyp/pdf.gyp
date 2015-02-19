# This file builds the PDF backend.
{
  'targets': [
    {
      'target_name': 'pdf',
      'product_name': 'skia_pdf',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'skflate.gyp:skflate',
      ],
      'includes': [
        'pdf.gypi',
      ],
      'include_dirs': [
        '../src/core', # needed to get SkGlyphCache.h and SkTextFormatParams.h
        '../src/pdf',
        '../src/utils', # needed to get SkBitSet.h
      ],
      'sources': [
        'pdf.gypi', # Makes the gypi appear in IDEs (but does not modify the build).
      ],
      'conditions': [
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
      # This section makes all targets that depend on this target
      # #define SK_SUPPORT_PDF and have access to the pdf header files.
      'direct_dependent_settings': {
        'defines': [
          'SK_SUPPORT_PDF',
        ],
        'include_dirs': [
          '../include/core',  # SkDocument.h
        ],
      },
    },
  ],
}
