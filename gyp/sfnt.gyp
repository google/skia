{
  'targets': [
    {
      'target_name': 'sfnt',
      'product_name': 'skia_sfnt',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'core.gyp:*',
      ],
      'include_dirs': [
        '../src/sfnt',
      ],
      'sources': [
        '../src/sfnt/SkIBMFamilyClass.h',
        '../src/sfnt/SkOTTableTypes.h',
        '../src/sfnt/SkOTTable_glyf.h',
        '../src/sfnt/SkOTTable_head.h',
        '../src/sfnt/SkOTTable_hhea.h',
        '../src/sfnt/SkOTTable_loca.h',
        '../src/sfnt/SkOTTable_maxp.h',
        '../src/sfnt/SkOTTable_maxp_CFF.h',
        '../src/sfnt/SkOTTable_maxp_TT.h',
        '../src/sfnt/SkOTTable_name.h',
        '../src/sfnt/SkOTTable_OS_2.h',
        '../src/sfnt/SkOTTable_OS_2_V0.h',
        '../src/sfnt/SkOTTable_OS_2_V1.h',
        '../src/sfnt/SkOTTable_OS_2_V2.h',
        '../src/sfnt/SkOTTable_OS_2_V3.h',
        '../src/sfnt/SkOTTable_OS_2_V4.h',
        '../src/sfnt/SkOTTable_OS_2_VA.h',
        '../src/sfnt/SkOTTable_post.h',
        '../src/sfnt/SkPanose.h',
        '../src/sfnt/SkOTUtils.h',
        '../src/sfnt/SkPreprocessorSeq.h',
        '../src/sfnt/SkSFNTHeader.h',
        '../src/sfnt/SkTTCFHeader.h',
        '../src/sfnt/SkTypedEnum.h',

        '../src/sfnt/SkOTTable_name.cpp',
        '../src/sfnt/SkOTUtils.cpp',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../src/sfnt',
        ],
      },
    },
  ],
}
