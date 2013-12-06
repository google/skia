# conditions used in both common.gypi and skia.gyp in chromium
#
{
  'defines': [
    'SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=<(skia_static_initializers)',
    'SK_SUPPORT_GPU=<(skia_gpu)',
    'SK_SUPPORT_OPENCL=<(skia_opencl)',
    'SK_DISTANCEFIELD_FONTS=<(skia_distancefield_fonts)',
  ],
  'conditions' : [
    [ 'skia_os == "win"',
      {
        'defines': [
          'SK_BUILD_FOR_WIN32',
          '_CRT_SECURE_NO_WARNINGS',
          'GR_GL_FUNCTION_TYPE=__stdcall',
        ],
        'msvs_cygwin_shell': 0,
        'msvs_settings': {
          'VCCLCompilerTool': {
            'WarningLevel': '3',
            'ProgramDataBaseFileName': '$(OutDir)\\$(ProjectName).pdb',
            'DebugInformationFormat': '3',
            'ExceptionHandling': '0',
            'AdditionalOptions': [ '/MP', ],
          },
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'OpenGL32.lib',
              'usp10.lib',

              # Prior to gyp r1584, the following were included automatically.
              'kernel32.lib',
              'gdi32.lib',
              'winspool.lib',
              'comdlg32.lib',
              'advapi32.lib',
              'shell32.lib',
              'ole32.lib',
              'oleaut32.lib',
              'user32.lib',
              'uuid.lib',
              'odbc32.lib',
              'odbccp32.lib',
              'DelayImp.lib',
            ],
          },
        },
        'configurations': {
          'Debug': {
            'msvs_settings': {
              'VCCLCompilerTool': {
                'DebugInformationFormat': '4', # editAndContiue (/ZI)
                'Optimization': '0',           # optimizeDisabled (/Od)
                'PreprocessorDefinitions': ['_DEBUG'],
                'RuntimeLibrary': '3',         # rtMultiThreadedDebugDLL (/MDd)
                'RuntimeTypeInfo': 'false',      # /GR-
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
                'Optimization': '<(skia_release_optimization_level)',
                'WholeProgramOptimization': 'true', #/GL
               # Changing the floating point model requires rebaseling gm images
               #'FloatingPointModel': '2',          # fast (/fp:fast)
                'FavorSizeOrSpeed': '1',            # speed (/Ot)
                'PreprocessorDefinitions': ['NDEBUG'],
                'RuntimeLibrary': '2',              # rtMultiThreadedDLL (/MD)
                'EnableEnhancedInstructionSet': '2',# /arch:SSE2
                'RuntimeTypeInfo': 'false',         # /GR-
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
        'conditions' : [
          [ 'skia_arch_width == 64', {
            'msvs_configuration_platform': 'x64',
          }],
          [ 'skia_arch_width == 32', {
            'msvs_configuration_platform': 'Win32',
          }],
          [ 'skia_warnings_as_errors', {
            'msvs_settings': {
              'VCCLCompilerTool': {
                'WarnAsError': 'true',
                'AdditionalOptions': [
                  '/we4189', # initialized but unused var warning
                ],
              },
            },
          }],
          [ 'skia_win_exceptions', {
            'msvs_settings': {
              'VCCLCompilerTool': {
                'AdditionalOptions': [
                  '/EHsc',
                ],
              },
            },
          }],
        ],
      },
    ],

    # The following section is common to linux + derivatives and android
    [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "nacl", "chromeos", "android"]',
      {
        'cflags': [
          '-g',
          '-fno-exceptions',
          '-fstrict-aliasing',

          '-Wall',
          '-Wextra',
          '-Winit-self',
          '-Wpointer-arith',

          '-Wno-unused-parameter',
          '-Wno-c++11-extensions',
        ],
        'cflags_cc': [
          '-fno-rtti',
          '-Wnon-virtual-dtor',
        ],
        'conditions': [
          [ 'skia_warnings_as_errors', {
            'cflags': [
              '-Werror',
            ],
          }],
          [ 'skia_arch_type == "arm" and arm_thumb == 1', {
            'cflags': [
              '-mthumb',
            ],
            # The --fix-cortex-a8 switch enables a link-time workaround for
            # an erratum in certain Cortex-A8 processors.  The workaround is
            # enabled by default if you target the ARM v7-A arch profile.
            # It can be enabled otherwise by specifying --fix-cortex-a8, or
            # disabled unconditionally by specifying --no-fix-cortex-a8.
            #
            # The erratum only affects Thumb-2 code.
            'conditions': [
              [ 'arm_version < 7', {
                'ldflags': [
                  '-Wl,--fix-cortex-a8',
                ],
              }],
            ],
          }],
          [ 'skia_arch_type == "arm" and arm_version >= 7', {
            'cflags': [
              '-march=armv7-a',
            ],
            'ldflags': [
              '-march=armv7-a',
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
              [ 'skia_os != "chromeos"', {
                'cflags': [
                  '-mfloat-abi=softfp',
                ],
              }],
            ],
          }],
        ],
      },
    ],


    [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "nacl", "chromeos"]',
      {
        'defines': [
          'SK_SAMPLES_FOR_X',
          'SK_BUILD_FOR_UNIX',
        ],
        'configurations': {
          'Coverage': {
            'cflags': ['--coverage'],
            'ldflags': ['--coverage'],
          },
          'Debug': {
          },
          'Release': {
            'cflags': [
              '-O<(skia_release_optimization_level)',
            ],
            'defines': [ 'NDEBUG' ],
          },
        },
        'conditions' : [
          [ 'skia_shared_lib', {
            'cflags': [
              '-fPIC',
            ],
            'defines': [
              'SKIA_DLL',
              'SKIA_IMPLEMENTATION=1',
            ],
          }],
          [ 'skia_os == "nacl"', {
            'defines': [
              'SK_BUILD_FOR_NACL',
            ],
            'link_settings': {
              'libraries': [
                '-lppapi',
                '-lppapi_cpp',
                '-lnosys',
                '-pthread',
              ],
            },
          }, { # skia_os != "nacl"
            'link_settings': {
              'ldflags': [
                '-lstdc++',
                '-lm',
              ],
            },
          }],
          [ 'skia_os != "chromeos"', {
            'conditions': [
              [ 'skia_arch_width == 64 and skia_arch_type == "x86"', {
                'cflags': [
                  '-m64',
                ],
                'ldflags': [
                  '-m64',
                ],
              }],
              [ 'skia_arch_width == 32 and skia_arch_type == "x86"', {
                'cflags': [
                  '-m32',
                ],
                'ldflags': [
                  '-m32',
                ],
              }],
            ],
          }],
          # Enable asan, tsan, etc.
          [ 'skia_sanitizer', {
            'cflags': [
              '-fsanitize=<(skia_sanitizer)',
            ],
            'ldflags': [
              '-fsanitize=<(skia_sanitizer)',
            ],
            'conditions' : [
              [ 'skia_sanitizer == "thread"', {
                'defines': [ 'DYNAMIC_ANNOTATIONS_ENABLED=1' ],
                'cflags': [ '-fPIC' ],
                'target_conditions': [
                  [ '_type == "executable"', {
                    'cflags': [ '-fPIE' ],
                    'ldflags': [ '-pie' ],
                  }],
                ],
              }],
            ],
          }],
          [ 'skia_clang_build', {
            'cflags': [
              # Extra warnings we like but that only Clang knows about.
              '-Wstring-conversion',
            ],
          }],
          [ 'skia_keep_frame_pointer', {
            'cflags': [ '-fno-omit-frame-pointer' ],
          }],
        ],
      },
    ],

    [ 'skia_os == "mac"',
      {
        'variables': {
          'mac_sdk%': '<!(python <(DEPTH)/tools/find_mac_sdk.py 10.6)',
        },
        'defines': [
          'SK_BUILD_FOR_MAC',
        ],
        'conditions' : [
          [ 'skia_arch_width == 64', {
            'xcode_settings': {
              'ARCHS': ['x86_64'],
            },
          }],
          [ 'skia_arch_width == 32', {
            'xcode_settings': {
              'ARCHS': ['i386'],
            },
          }],
          [ 'skia_warnings_as_errors', {
            'xcode_settings': {
              'OTHER_CPLUSPLUSFLAGS': [
                '-Werror',
                '-Wall',
                '-Wextra',
                '-Wno-unused-parameter',
              ],
            },
          }],
# This old compiler is really bad at figuring out when things are uninitialized, so ignore it.
          [ '<(mac_sdk)==10.6', {
            'xcode_settings': {
              'OTHER_CPLUSPLUSFLAGS': [
                '-Wno-uninitialized',
              ],
            },
          }],
        ],
        'configurations': {
          'Coverage': {
            'xcode_settings': {
               'GCC_OPTIMIZATION_LEVEL': '0',
               'GCC_GENERATE_TEST_COVERAGE_FILES': 'YES',
               'GCC_INSTRUMENT_PROGRAM_FLOW_ARCS' : 'YES',
            },
          },
          'Debug': {
            'xcode_settings': {
              'GCC_OPTIMIZATION_LEVEL': '0',
            },
          },
          'Release': {
            'xcode_settings': {
              'GCC_OPTIMIZATION_LEVEL': '<(skia_release_optimization_level)',
            },
            'defines': [ 'NDEBUG' ],
          },
        },
        'xcode_settings': {
          'GCC_SYMBOLS_PRIVATE_EXTERN': 'NO',
          'conditions': [
            [ 'skia_osx_sdkroot==""', {
              'SDKROOT': 'macosx<(mac_sdk)',  # -isysroot
            }, {
              'SDKROOT': '<(skia_osx_sdkroot)',  # -isysroot
            }],
           ],
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
          'OTHER_CPLUSPLUSFLAGS': [
            '-mssse3',
            '-fvisibility=hidden',
            '-fvisibility-inlines-hidden',
          ],
        },
      },
    ],

    [ 'skia_os == "ios"',
      {
        'defines': [
          'SK_BUILD_FOR_IOS',
        ],
        'conditions' : [
          [ 'skia_warnings_as_errors', {
            'xcode_settings': {
              'OTHER_CPLUSPLUSFLAGS': [
                '-Werror',
              ],
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
              'GCC_OPTIMIZATION_LEVEL': '<(skia_release_optimization_level)',
            },
            'defines': [ 'NDEBUG' ],
          },
        },
        'xcode_settings': {
          'ARCHS': ['armv6', 'armv7'],
          'CODE_SIGNING_REQUIRED': 'NO',
          'CODE_SIGN_IDENTITY[sdk=iphoneos*]': '',
          'IPHONEOS_DEPLOYMENT_TARGET': '<(ios_sdk_version)',
          'SDKROOT': 'iphoneos',
          'TARGETED_DEVICE_FAMILY': '1,2',
          'OTHER_CPLUSPLUSFLAGS': [
            '-fvisibility=hidden',
            '-fvisibility-inlines-hidden',
          ],
          'GCC_THUMB_SUPPORT': 'NO',
        },
      },
    ],

    [ 'skia_os == "android"',
      {
        'defines': [
          'SK_BUILD_FOR_ANDROID',
          'SK_FONTHOST_DOES_NOT_USE_FONTMGR',
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
          '-fuse-ld=gold',
        ],
        'conditions': [
          [ 'skia_shared_lib', {
            'cflags': [
              '-fPIC',
            ],
            'defines': [
              'SKIA_DLL',
              'SKIA_IMPLEMENTATION=1',
            ],
          }],
          [ 'skia_profile_enabled == 1', {
            'cflags': ['-g', '-fno-omit-frame-pointer', '-marm', '-mapcs'],
          }],
        ],
      },
    ],

    # We can POD-style initialization of static mutexes to avoid generating
    # static initializers if we're using a pthread-compatible thread interface.
    [ 'skia_os != "win"', {
      'defines': [
        'SK_USE_POSIX_THREADS',
      ],
    }],
  ], # end 'conditions'
  # The Xcode SYMROOT must be at the root. See build/common.gypi in chromium for more details
  'xcode_settings': {
    'SYMROOT': '<(DEPTH)/xcodebuild',
  },
}
