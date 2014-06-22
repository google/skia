# GYP file to build flag parser
#
{
  'targets': [
    {
      'target_name': 'flags',
      'type': 'static_library',
      'sources': [
        '../tools/flags/SkCommandLineFlags.cpp',
        '../tools/flags/SkCommandLineFlags.h',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../tools/flags',
        ],
      }
    },
  ],
}
