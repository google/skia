{
  'targets': [
    {
      'target_name': 'xps',
      'product_name': 'skia_xps',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'sfnt.gyp:sfnt',
      ],
      'include_dirs': [
        '../include/device/xps',
        '../include/utils/win',
        '../src/core', # needed to get SkGlyphCache.h
        '../src/utils', # needed to get SkBitSet.h
      ],
      'sources': [
        '../include/device/xps/SkConstexprMath.h',
        '../include/device/xps/SkXPSDevice.h',

        '../src/device/xps/SkXPSDevice.cpp',
      ],
      'conditions': [
        [ 'skia_os == "win"', {
          'link_settings': {
            'libraries': [
              '-lt2embed.lib',
              '-lfontsub.lib',
            ],
          },
        },{ #else if 'skia_os != "win"'
          'include_dirs!': [
            '../include/utils/win',
          ],
          'sources!': [
            '../include/device/xps/SkXPSDevice.h',

            '../src/device/xps/SkXPSDevice.cpp',
          ],
        }],
      ],
      # This section makes all targets that depend on this target
      # #define SK_SUPPORT_XPS and have access to the xps header files.
      'direct_dependent_settings': {
        'conditions': [
          [ 'skia_os == "win"', {
            'defines': [
              'SK_SUPPORT_XPS',
            ],
          }],
        ],
        'include_dirs': [
          '../include/device/xps',
          '../src/utils', # needed to get SkBitSet.h
        ],
      },
    },
  ],
}
