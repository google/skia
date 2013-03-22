{
  'variables': {
    'skia_warnings_as_errors': 0,
  },
  'targets': [
    {
      'target_name': 'cityhash',
      'type': 'static_library',
      'standalone_static_library': 1,
      'include_dirs': [
        '../include/config',
        '../include/core',
        '../src/utils/cityhash',
        '../third_party/externals/cityhash/src',
      ],
      'sources': [
        '../third_party/externals/cityhash/src/city.cc',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/externals/cityhash/src',
        ],
      },
      'conditions': [
        [ 'skia_os == "android"', {
          'cflags!': [
            '-Wall',
          ],
        }],
      ],
    },
  ],
}
