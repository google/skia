# GYP file to build flag parser
#
{
  'targets': [
    {
      'target_name': 'flags',
      'type': 'static_library',
      'sources': [
        '../tools/SkFlags.h',
        '../tools/SkFlags.cpp',
      ],
      'dependencies': [
        'skia_base_libs.gyp:skia_base_libs',
        'core.gyp:core',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../tools/',
        ],
      }
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
