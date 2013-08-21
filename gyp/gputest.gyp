{
  'targets': [
    {
      'target_name': 'skgputest',
      'product_name': 'skia_skgputest',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
      'include_dirs': [
        '../include/gpu',
        '../include/utils',
        '../src/core',
        '../src/gpu',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../src/gpu',
        ],
      },
      'sources': [
        '<(skia_src_path)/gpu/GrTest.cpp',
        '<(skia_src_path)/gpu/GrTest.h',
      ],
      'defines': [
        'GR_IMPLEMENTATION=1',
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
