# The minimal set of static libraries for basic Skia functionality.
{
  'variables': {
    'component_libs': [
      'core.gyp:core',
      'opts.gyp:opts',
      'ports.gyp:ports',
      'utils.gyp:utils',
    ],
    'conditions': [
      [ 'skia_arch_type == "x86" and skia_os != "android"', {
        'component_libs': [
          'opts.gyp:opts_ssse3',
        ],
      }],
      [ 'arm_neon == 1', {
        'component_libs': [
          'opts.gyp:opts_neon',
        ],
      }],
      [ 'skia_gpu', {
        'component_libs': [
          'gpu.gyp:gr',
          'gpu.gyp:skgr',
        ],
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'skia_base_libs',
      'type': 'none',
      'dependencies': [
        '<@(component_libs)',
      ],
      'export_dependent_settings': [
        '<@(component_libs)',
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
