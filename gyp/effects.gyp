{
  'targets': [
    {
      'target_name': 'effects',
      'type': 'static_library',
      'includes': [
        'effects.gypi',
      ],
      'include_dirs': [
        '../include/effects',
        '../src/core',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/effects',
        ],
      },
      'dependencies': [
        'core.gyp:core',
      ],
      'conditions': [
        ['skia_gpu == 1', {
          'include_dirs': [
            '../src/gpu',
          ],
          'dependencies': [
            'gpu.gyp:gr',
          ],
        }],
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
