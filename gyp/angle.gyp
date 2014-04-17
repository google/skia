# ANGLE is the Windows-specific translator from OGL ES 2.0 to D3D 9

{
  'conditions': [
    [ 'skia_angle', {
      'target_defaults': {
        'include_dirs': [
          '$(DXSDK_DIR)/Include',
        ],
        'msvs_settings': {
          'VCLinkerTool': {
            'conditions': [
              [ 'skia_arch_width == 32 ', {
                'AdditionalLibraryDirectories': [
                  '$(DXSDK_DIR)/Lib/x86',
                ],
              },{
                'AdditionalLibraryDirectories': [
                  '$(DXSDK_DIR)/Lib/x64',
                ],
              }],
            ],
          },
        },
        'defines': [
          'NOMINMAX',
        ],
        'defines/': [
          ['exclude', 'ANGLE_PRELOADED_D3DCOMPILER_MODULE_NAMES'],
        ],
      },
      'variables': {
        'component': 'static_library',
        'skia_warnings_as_errors': 0,
      },
      'includes': [
        '../third_party/externals/angle/src/build_angle.gypi',
      ],
    }],
  ],
  'targets': [
    {
      'target_name': 'angle',
      'type': 'none',
      'conditions': [
        [ 'skia_angle', {
          'direct_dependent_settings': {
            'include_dirs': [
              '../third_party/externals/angle/include',
            ],
          },
        }],
      ],
    },
  ],
}
