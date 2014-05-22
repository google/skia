{
  'variables': {
    'skia_warnings_as_errors': 0,
  },
  'targets': [
  {
    'target_name': 'libetc1',
    'type': 'static_library',
    'include_dirs' : [
      '../third_party/etc1'
    ],
    'sources': [
      '../third_party/etc1/etc1.cpp',
    ],
    'direct_dependent_settings': {
      'include_dirs': [
        '../third_party/etc1',
      ],
    },
  }],
}
