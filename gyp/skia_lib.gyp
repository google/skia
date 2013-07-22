# The minimal set of static libraries for basic Skia functionality.

{
  'variables': {
    'component_libs': [
      'core.gyp:core',
      'effects.gyp:effects',
      'images.gyp:images',
      'opts.gyp:opts',
      'ports.gyp:ports',
      'sfnt.gyp:sfnt',
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
          'gpu.gyp:skgpu',
        ],
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'skia_lib',
      'conditions': [
        [ 'skia_shared_lib', {
          'conditions': [
            [ 'skia_os == "android"', {
              # The name skia will confuse the linker on android into using the system's libskia.so
              # instead of the one packaged with the apk. We simply choose a different name to fix
              # this.
              'product_name': 'skia_android',
            }, {
              'product_name': 'skia',
            }],
          ],
          'type': 'shared_library',
        }, {
          'type': 'none',
        }],
      ],
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
