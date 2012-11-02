# Common entry point for all Skia executables running in NaCl
{
  'targets': [
    {
      'target_name': 'nacl_interface',
      'type': 'static_library',
      'dependencies': [
        'skia_base_libs.gyp:skia_base_libs',
      ],
      'include_dirs': [
        # For SkThreadUtils.h
        '../src/utils',
      ],
      'sources': [
        '../../nacl/src/nacl_interface.cpp',
      ],
    },
  ],
}
