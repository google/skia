{
  'includes': [
    'common.gypi',
  ],
  'targets': [
    {
      'target_name': 'sfnt',
      'type': 'static_library',
      'dependencies': [
        'core.gyp:core',
      ],
      'include_dirs': [
        '../src/sfnt',
      ],
      'sources': [
        '../src/sfnt/SkIBMFamilyClass.h',
        '../src/sfnt/SkOTTableTypes.h',
        '../src/sfnt/SkOTTable_head.h',
        '../src/sfnt/SkOTTable_hhea.h',
        '../src/sfnt/SkOTTable_OS_2.h',
        '../src/sfnt/SkOTTable_OS_2_V0.h',
        '../src/sfnt/SkOTTable_OS_2_V1.h',
        '../src/sfnt/SkOTTable_OS_2_V2.h',
        '../src/sfnt/SkOTTable_OS_2_V3.h',
        '../src/sfnt/SkOTTable_OS_2_V4.h',
        '../src/sfnt/SkOTTable_OS_2_VA.h',
        '../src/sfnt/SkOTTable_post.h',
        '../src/sfnt/SkPanose.h',
        '../src/sfnt/SkPreprocessorSeq.h',
        '../src/sfnt/SkTypedEnum.h',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../src/sfnt',
        ],
      },
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
