# conditions used in both common.gypi and skia.gyp in chromium
#
{
  'conditions' : [

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
            'AdditionalOptions': [ '/MP' ],
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
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
