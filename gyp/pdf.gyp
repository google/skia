{
  'includes': [
    'common.gypi',
  ],
  'targets': [
    {
      'target_name': 'pdf',
      'type': 'static_library',
      'include_dirs': [
        '../include/config',
        '../include/core',
        '../include/pdf',
        '../src/core', # needed to get SkGlyphCache.h and SkTextFormatParams.h
      ],
      'sources': [
        '../include/pdf/SkBitSet.h',
        '../include/pdf/SkPDFCatalog.h',
        '../include/pdf/SkPDFDevice.h',
        '../include/pdf/SkPDFDocument.h',
        '../include/pdf/SkPDFFont.h',
        '../include/pdf/SkPDFFormXObject.h',
        '../include/pdf/SkPDFGraphicState.h',
        '../include/pdf/SkPDFImage.h',
        '../include/pdf/SkPDFPage.h',
        '../include/pdf/SkPDFShader.h',
        '../include/pdf/SkPDFStream.h',
        '../include/pdf/SkPDFTypes.h',
        '../include/pdf/SkPDFUtils.h',

        '../src/pdf/SkBitSet.cpp',
        '../src/pdf/SkPDFCatalog.cpp',
        '../src/pdf/SkPDFDevice.cpp',
        '../src/pdf/SkPDFDocument.cpp',
        '../src/pdf/SkPDFFont.cpp',
        '../src/pdf/SkPDFFontImpl.h',
        '../src/pdf/SkPDFFormXObject.cpp',
        '../src/pdf/SkPDFGraphicState.cpp',
        '../src/pdf/SkPDFImage.cpp',
        '../src/pdf/SkPDFPage.cpp',
        '../src/pdf/SkPDFShader.cpp',
        '../src/pdf/SkPDFStream.cpp',
        '../src/pdf/SkPDFTypes.cpp',
        '../src/pdf/SkPDFUtils.cpp',
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
      'dependencies': [
        'zlib.gyp:zlib',
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
