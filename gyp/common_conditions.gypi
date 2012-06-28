# conditions used in both common.gypi and skia.gyp in chromium
#
{
  'conditions' : [

    ['skia_os == "win"',
      {
        'defines': [
          'SK_BUILD_FOR_WIN32',
          'SK_IGNORE_STDINT_DOT_H',
          '_CRT_SECURE_NO_WARNINGS',
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
                'DebugInformationFormat': '4', # editAndContiue (/ZI)
                'ProgramDataBaseFileName': '$(OutDir)\\$(ProjectName).pdb',
                'Optimization': '0',           # optimizeDisabled (/Od)
                'PreprocessorDefinitions': ['_DEBUG'],
                'RuntimeLibrary': '3',         # rtMultiThreadedDebugDLL (/MDd)
                'ExceptionHandling': '0',
                'RuntimeTypeInfo': 'false',      # /GR-
                'WarningLevel': '3',             # level3 (/W3)
              },
              'VCLinkerTool': {
                'GenerateDebugInformation': 'true', # /DEBUG
                'LinkIncremental': '2',             # /INCREMENTAL
              },
            },
          },
          'Release': {
            'msvs_settings': {
              'VCCLCompilerTool': {
                'DebugInformationFormat': '3',      # programDatabase (/Zi)
                'ProgramDataBaseFileName': '$(OutDir)\\$(ProjectName).pdb',
                'Optimization': '3',                # full (/Ox)
                'WholeProgramOptimization': 'true', #/GL
               # Changing the floating point model requires rebaseling gm images
               #'FloatingPointModel': '2',          # fast (/fp:fast)
                'FavorSizeOrSpeed': '1',            # speed (/Ot)
                'PreprocessorDefinitions': ['NDEBUG'],
                'RuntimeLibrary': '2',              # rtMultiThreadedDLL (/MD)
                'ExceptionHandling': '0',
                'EnableEnhancedInstructionSet': '2',# /arch:SSE2
                'RuntimeTypeInfo': 'false',         # /GR-
                'WarningLevel': '3',                # level3 (/W3)
              },
              'VCLinkerTool': {
                'GenerateDebugInformation': 'true', # /DEBUG
                'LinkTimeCodeGeneration': '1',      # useLinkTimeCodeGeneration /LTCG
              },
              'VCLibrarianTool': {
                'LinkTimeCodeGeneration': 'true',   # useLinkTimeCodeGeneration /LTCG
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
          'SK_USE_COLOR_LUMINANCE',
          'SK_GAMMA_APPLY_TO_A8',
        ],
        'configurations': {
          'Debug': {
            'cflags': ['-g']
          },
          'Release': {
            'cflags': ['-O3'],
            'defines': [ 'NDEBUG' ],
          },
        },
        'cflags': [
          # TODO(tony): Enable -Werror once all the strict-aliasing problems
          # are fixed.
          #'-Werror',
          '-Wall',
          '-Wextra',
          '-Wno-unused',
          # suppressions below here were added for clang
          '-Wno-unused-parameter',
          '-Wno-c++11-extensions'
        ],
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
        'conditions' : [
          ['skia_arch_width == 64', {
            'xcode_settings': {
              'ARCHS': 'x86_64',
            },
          }],
          ['skia_arch_width == 32', {
            'xcode_settings': {
              'ARCHS': 'i386',
            },
          }],
        ],
        'configurations': {
          'Debug': {
            'xcode_settings': {
              'GCC_OPTIMIZATION_LEVEL': '0',
            },
          },
          'Release': {
            'xcode_settings': {
              'GCC_OPTIMIZATION_LEVEL': '3',
            },
            'defines': [ 'NDEBUG' ],
          },
        },
        'xcode_settings': {
          'GCC_SYMBOLS_PRIVATE_EXTERN': 'NO',
          'SYMROOT': '<(DEPTH)/xcodebuild',
          'SDKROOT': 'macosx10.6',
# trying to get this to work, but it needs clang I think...
#          'WARNING_CFLAGS': '-Wexit-time-destructors',
          'CLANG_WARN_CXX0X_EXTENSIONS': 'NO',
          'GCC_WARN_64_TO_32_BIT_CONVERSION': 'YES',
          'GCC_WARN_ABOUT_DEPRECATED_FUNCTIONS': 'YES',
          'GCC_WARN_ABOUT_INVALID_OFFSETOF_MACRO': 'YES',
          'GCC_WARN_ABOUT_MISSING_NEWLINE': 'YES',
          'GCC_WARN_ABOUT_MISSING_PROTOTYPES': 'YES',
          'GCC_WARN_ABOUT_POINTER_SIGNEDNESS': 'YES',
          'GCC_WARN_ABOUT_RETURN_TYPE': 'YES',
          'GCC_WARN_ALLOW_INCOMPLETE_PROTOCOL': 'YES',
          'GCC_WARN_INITIALIZER_NOT_FULLY_BRACKETED': 'YES',
          'GCC_WARN_MISSING_PARENTHESES': 'YES',
          'GCC_WARN_PROTOTYPE_CONVERSION': 'YES',
          'GCC_WARN_SIGN_COMPARE': 'YES',
          'GCC_WARN_TYPECHECK_CALLS_TO_PRINTF': 'YES',
          'GCC_WARN_UNKNOWN_PRAGMAS': 'YES',
          'GCC_WARN_UNUSED_FUNCTION': 'YES',
          'GCC_WARN_UNUSED_LABEL': 'YES',
          'GCC_WARN_UNUSED_VALUE': 'YES',
          'GCC_WARN_UNUSED_VARIABLE': 'YES',
          'OTHER_CPLUSPLUSFLAGS': '-mssse3 -fvisibility=hidden -fvisibility-inlines-hidden',
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
    
    ['skia_os == "android"', 
      {
        'defines': [
          'SK_BUILD_FOR_ANDROID',
          'SK_BUILD_FOR_ANDROID_NDK',
          'SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=0',
        ],
        'configurations': {
          'Debug': {
            'cflags': ['-g']
          },
          'Release': {
            'cflags': ['-O2'],
            'defines': [ 'NDEBUG' ],
          },
        },
        'libraries': [
          '-lstdc++',
          '-lm',
          '-llog',
        ],
        'cflags': [
          '-fno-exceptions',
          '-fno-rtti',
        ],
        'conditions': [
          [ 'skia_arch_type == "arm"', {
            'ldflags': [
              '-Wl',
              '--fix-cortex-a8',
            ],
          }],
          [ 'skia_arch_type == "arm" and arm_thumb == 1', {
            'cflags': [
              '-mthumb',
            ],
          }],
          [ 'skia_arch_type == "arm" and armv7 == 1', {
            'variables': {
              'arm_neon_optional%': 0,
            },
            'defines': [
              '__ARM_ARCH__=7',
            ],
            'cflags': [
              '-march=armv7-a',
              '-mfloat-abi=softfp',
            ],
            'conditions': [
              [ 'arm_neon == 1', {
                'defines': [
                  '__ARM_HAVE_NEON',
                ],
                'cflags': [
                  '-mfpu=neon',
                ],
              }],
              [ 'arm_neon_optional == 1', {
                'defines': [
                  '__ARM_HAVE_OPTIONAL_NEON_SUPPORT',
                ],
              }],
            ],
         }],
        ], 
      },
    ],

    # We can POD-style initialization of static mutexes to avoid generating
    # static initializers if we're using a pthread-compatible thread interface.
    [ 'skia_os != "win"', {
      'defines': [
        'SK_USE_POSIX_THREADS'
      ],
    }],

  ], # end 'conditions'
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
