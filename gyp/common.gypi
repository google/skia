# Copyright 2011 The Android Open Source Project
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  # Define all variables, allowing for override in GYP_DEFINES.
  #
  # One such variable is 'skia_os', which we use instead of 'OS' throughout
  # our gyp files.  We set it automatically based on 'OS', but allow the
  # user to override it via GYP_DEFINES if they like.
  'variables': {
    'skia_scalar%': 'float',
    'skia_os%': '<(OS)',
  },
  'skia_scalar%': '<(skia_scalar)',
  'skia_os': '<(skia_os)',

  'target_defaults': {

    # Validate the 'skia_os' setting against 'OS', because only certain
    # combinations work.  You should only override 'skia_os' for certain
    # situations, like building for iOS on a Mac.
    'variables': {
      'conditions': [
        ['skia_os != OS and not (skia_os == "ios" and OS == "mac")',
          {'error': '<!(Cannot build with skia_os=<(skia_os) on OS=<(OS))'}],
      ],
    },

    'configurations': {
      'Debug': {
        'defines': [
          'SK_DEBUG',
          'GR_DEBUG=1',
        ],
      },
      'Release': {
        'defines': [
          'SK_RELEASE',
          'GR_RELEASE=1',
        ],
      },
    },

    'conditions' : [

      [ 'skia_scalar == "float"',
        {
          'defines': [
            'SK_SCALAR_IS_FLOAT',
            'SK_CAN_USE_FLOAT',
          ],
        }, { # else, skia_scalar != "float"
          'defines': [
            'SK_SCALAR_IS_FIXED',
            'SK_CAN_USE_FLOAT',  # we can still use floats along the way
          ],
        }
      ],

      ['skia_os == "win"',
        {
          'defines': [
            'SK_BUILD_FOR_WIN32',
            'SK_IGNORE_STDINT_DOT_H',
          ],
          'msvs_cygwin_shell': 0,
          'msvs_settings': {
            'VCCLCompilerTool': {
              'WarningLevel': '1',
              'WarnAsError': 'false',
              'DebugInformationFormat': '3',
              'AdditionalOptions': ['/MP',],
            },
            'VCLinkerTool': {
              'AdditionalDependencies': [
                'OpenGL32.lib',
                'usp10.lib',
              ],
            },
          },
          'configurations': {
            'Debug': {
              'msvs_settings': {
                'VCCLCompilerTool': {
                  'DebugInformationFormat': '1', # debugOldStyleInfo (/Z7)
                  'Optimization': '0',           # optimizeDisabled (/Od)
                  'PreprocessorDefinitions': ['_DEBUG'],
                  'RuntimeLibrary': '3',         # rtMultiThreadedDebugDLL (/MDd)
                },
                'VCLinkerTool': {
                  'GenerateDebugInformation': 'true',
                },
              },
            },
            'Release': {
              'msvs_settings': {
                'VCCLCompilerTool': {
                  'DebugInformationFormat': '0', # debugDisabled
                  'Optimization': '2',           # optimizeMaxSpeed (/O2)
                  'PreprocessorDefinitions': ['NDEBUG'],
                  'RuntimeLibrary': '2',         # rtMultiThreadedDLL (/MD)
                },
                'VCLinkerTool': {
                  'GenerateDebugInformation': 'false',
                },
              },
            },
          },
        },
      ],

      ['skia_os in ["linux", "freebsd", "openbsd", "solaris"]',
        {
          'defines': [
            'SK_SAMPLES_FOR_X',
            'SK_BUILD_FOR_UNIX',
          ],
          'configurations': {
            'Debug': {
              'cflags': ['-g']
            },
            'Release': {
              'cflags': ['-O2']
            },
          },
          'cflags': [ '-Wall', '-Wextra', '-Wno-unused' ],
          'include_dirs' : [
            '/usr/include/freetype2',
          ],
        },
      ],

      ['skia_os == "mac"', 
        {
          'defines': [
            'SK_BUILD_FOR_MAC',
          ],
          'configurations': {
            'Debug': {
              'xcode_settings': {
                'GCC_OPTIMIZATION_LEVEL': '0',
              },
            },
          },
          'xcode_settings': {
            'SYMROOT': '<(DEPTH)/xcodebuild',
          },
        },
      ],

      ['skia_os == "ios"', 
        {
          'defines': [
            'SK_BUILD_FOR_IOS',
          ],
          'configurations': {
            'Debug': {
              'xcode_settings': {
                'GCC_OPTIMIZATION_LEVEL': '0',
              },
            },
          },
          'xcode_settings': {
            'SYMROOT': '<(DEPTH)/xcodebuild',
          },
        },
      ],

    ], # end 'conditions'
  }, # end 'target_defaults'
}
# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
