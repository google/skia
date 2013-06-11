# This GYP file stores the dependencies necessary to build Skia on the Chrome OS
# platform. The OS doesn't provide many stable libraries as part of the
# distribution so we have to build a few of them ourselves.

{
  'variables': {
    'skia_warnings_as_errors': 0,
  },
  'targets': [
    {
      'target_name': 'gif',
      'type': 'static_library',
      'sources': [
        '../third_party/externals/gif/dgif_lib.c',
        '../third_party/externals/gif/gifalloc.c',
        '../third_party/externals/gif/gif_err.c',
      ],
      'include_dirs': [
        '../third_party/externals/gif',
      ],
      'cflags': [
        '-Wno-format',
        '-DHAVE_CONFIG_H',
      ],
      'cflags!': [
        '-Wall',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/externals/gif',
        ],
      }
    },
  ]
}
