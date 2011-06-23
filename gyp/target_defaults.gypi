{
  'target_defaults': {
    'defines': [
      'SK_CAN_USE_FLOAT',
    ],

    'variables': {
      'skia_scalar%': 'float',
    },
    # Allow override in GYP_DEFINES environment variable.
    'skia_scalar%': '<(skia_scalar)',

    'configurations': {
      'Debug': {
        'defines': [
          'SK_DEBUG',
          'GR_DEBUG=1',
        ],
        'msvs_settings': {
           'VCLinkerTool': {
             'GenerateDebugInformation': 'true',
           },
           'VCCLCompilerTool': {
             #0 Disabled (/Od)
             #1 Minimize Size (/O1)
             #2 Maximize Speed (/O2)
             #3 Full Optimization (/Ox)
             'Optimization': '0',
           },
         },
      },
      'Release': {
        'defines': [
          'SK_RELEASE',
          'GR_RELEASE=1',
        ],
      },
    },
    'conditions': [
      [ 'skia_scalar == "float"',
        {
          'defines': [
            'SK_SCALAR_IS_FLOAT',
          ],
        }, { # else, skia_scalar != "float"
          'defines': [
            'SK_SCALAR_IS_FIXED',
          ],
        }
      ],
      [ 'OS == "linux" or OS == "freebsd" or OS == "openbsd" or OS == "solaris"', {
        'include_dirs' : [
          '/usr/include/freetype2',
        ],
      }],
      [ 'OS == "mac"', {
        'defines': [
          'SK_BUILD_FOR_MAC',
        ],
      }],
      [ 'OS == "win"', {
        'defines': [
          'SK_BUILD_FOR_WIN32',
          'SK_IGNORE_STDINT_DOT_H',
        ],
      }],
      [ 'OS == "linux"', {
        'defines': [
          'SK_SAMPLES_FOR_X',
          'SK_BUILD_FOR_UNIX',
        ],
      }],
    ],
    'direct_dependent_settings': {
      'conditions': [
        [ 'OS == "mac"', {
          'defines': [
            'SK_BUILD_FOR_MAC',
          ],
        }],
        [ 'OS == "win"', {
          'defines': [
            'SK_BUILD_FOR_WIN32',
          ],
        }],
      ],
    },
  },
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
