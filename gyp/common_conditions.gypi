# conditions used in both common.gypi and skia.gyp in chromium
#
{
  'defines': [
    'SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=<(skia_static_initializers)',
    'SK_SUPPORT_GPU=<(skia_gpu)',
    'SK_SUPPORT_OPENCL=<(skia_opencl)',
    'SK_FORCE_DISTANCE_FIELD_TEXT=<(skia_force_distance_field_text)',
  ],
  'conditions' : [
    ['skia_pic', {
     'cflags': [
       '-fPIC',
     ],
     'conditions' : [
      # FIXME: The reason we don't do this on Android is due to the way
      # we build the executables/skia_launcher on Android. See
      # https://codereview.chromium.org/406613003/diff/1/gyp/common_conditions.gypi#newcode455
      ['skia_os != "android"', {
       'target_conditions': [
         [ '_type == "executable"', {
           'cflags': [ '-fPIE' ],
           'ldflags': [ '-pie' ],
         }],
       ],
      }],
     ],
    }],

    # As of M35, Chrome requires SSE2 on x86 (and SSSE3 on Mac).
    [ 'skia_arch_type == "x86"', {
      'cflags': [
        '-msse2',
        '-mfpmath=sse',
      ],
    }],

    [ 'skia_os == "win"',
      {
        'defines': [
          'SK_BUILD_FOR_WIN32',
          '_CRT_SECURE_NO_WARNINGS',
          'GR_GL_FUNCTION_TYPE=__stdcall',
        ],
        'msvs_disabled_warnings': [
            4275,  # An exported class was derived from a class that was not exported
            4345,  # This is an FYI about a behavior change from long ago. Chrome stifles it too.
            4355,  # 'this' used in base member initializer list. Off by default in newer compilers.
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
            'LargeAddressAware': 2,  # 2 means "Yes, please let me use more RAM on 32-bit builds."
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
              },
            },
          },
        },
        'conditions' : [
          # Gyp's ninja generator depends on these specially named
          # configurations to build 64-bit on Windows.
          # See http://skbug.com/2348
          #
          # We handle the 64- vs 32-bit variations elsewhere, so I think it's
          # OK for us to just make these inherit non-archwidth-specific
          # configurations without modification.
          #
          # See http://skbug.com/2442 : These targets cause problems in the
          # MSVS build, so only include them if gyp is generating a ninja build.
          [ '"ninja" in "<!(echo %GYP_GENERATORS%)"', {
            'configurations': {
              'Debug_x64': {
                'inherit_from': ['Debug'],
                'msvs_settings': {
                  'VCCLCompilerTool': {
                     # /ZI is not supported on 64bit
                    'DebugInformationFormat': '3', # programDatabase (/Zi)
                  },
                },
              },
              'Release_x64': {
                'inherit_from': ['Release'],
                'msvs_settings': {
                  'VCCLCompilerTool': {
                     # Don't specify /arch. SSE2 is implied by 64bit and specifying it warns.
                    'EnableEnhancedInstructionSet': '0', #
                  },
                },
              },
              'Release_Developer_x64': {
                'inherit_from': ['Release_Developer'],
                'msvs_settings': {
                  'VCCLCompilerTool': {
                     # Don't specify /arch. SSE2 is implied by 64bit and specifying it warns.
                    'EnableEnhancedInstructionSet': '0', #
                  },
                },
              },
            },
          }],
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
          [ 'skia_win_ltcg', {
            'configurations': {
              'Release': {
                'msvs_settings': {
                  'VCCLCompilerTool': {
                    'WholeProgramOptimization': 'true', #/GL
                  },
                  'VCLinkerTool': {
                    'LinkTimeCodeGeneration': '1',      # useLinkTimeCodeGeneration /LTCG
                  },
                  'VCLibrarianTool': {
                    'LinkTimeCodeGeneration': 'true',   # useLinkTimeCodeGeneration /LTCG
                  },
                },
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
          '-Wsign-compare',

          '-Wno-unused-parameter',
        ],
        'cflags_cc': [
          '-std=c++11',
          '-fno-rtti',
          '-Wnon-virtual-dtor',
          '-Wno-invalid-offsetof',  # GCC <4.6 is old-school strict about what is POD.
        ],
        'conditions': [
          [ 'skia_os != "chromeos"', {
            'conditions': [
              [ 'skia_arch_type == "x86_64" and not skia_android_framework', {
                'cflags': [
                  '-m64',
                ],
                'ldflags': [
                  '-m64',
                ],
              }],
              [ 'skia_arch_type == "x86" and not skia_android_framework', {
                'cflags': [
                  '-m32',
                ],
                'ldflags': [
                  '-m32',
                ],
              }],
            ],
          }],
          [ 'skia_warnings_as_errors', {
            'cflags': [
              '-Werror',
            ],
          }],
          # For profiling; reveals some costs, exaggerates others (e.g. trivial setters & getters).
          [ 'skia_disable_inlining', {
            'cflags': [
              '-fno-inline',
              '-fno-default-inline',
              '-finline-limit=0',
              '-fno-omit-frame-pointer',
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
                  'SK_ARM_HAS_NEON',
                ],
                'cflags': [
                  '-mfpu=neon',
                ],
              }],
              [ 'arm_neon_optional == 1', {
                'defines': [
                  'SK_ARM_HAS_OPTIONAL_NEON',
                ],
              }],
              [ 'skia_os != "chromeos" and skia_os != "linux"', {
                'cflags': [
                  '-mfloat-abi=softfp',
                ],
              }],
            ],
          }],
          [ 'skia_arch_type == "mips"', {
            'cflags': [
              '-EL',
            ],
            'conditions': [
              [ 'mips_arch_variant == "mips32r2"', {
                'cflags': [
                  '-march=mips32r2',
                ],
                'conditions': [
                  [ 'mips_dsp == 1', {
                    'cflags': [
                      '-mdsp',
                    ],
                    'defines': [
                      'SK_MIPS_HAS_DSP',
                    ],
                  }],
                  [ 'mips_dsp == 2', {
                    'cflags': [
                      '-mdspr2',
                    ],
                    'defines': [
                      'SK_MIPS_HAS_DSP',
                      'SK_MIPS_HAS_DSPR2',
                    ],
                  }],
                ],
              }],
            ],
          }],
        ],
      },
    ],

    [ 'skia_os == "nacl"', {
      # NaCl compiler is GCC 4.4, which is too old to understand 'c++11', so call it '0x'.
      # NaCl's newlib needs gnu++ mode to see snprintf, vsnprintf, etc in C++11 mode.
      'cflags_cc!': [ '-std=c++11' ],
      'cflags_cc' : [ '-std=gnu++0x' ],
    }],

    ['skia_android_framework', {
      'includes' : [
        'skia_for_android_framework_defines.gypi',
      ],
      'cflags': [
        # Skia does not enforce this usage pattern so we disable it here to avoid
        # unecessary log spew when building
        '-Wno-unused-parameter',

        # Android's -D_FORTIFY_SOURCE=2 extensions are incompatibile with SkString.
        # Revert to -D_FORTIFY_SOURCE=1
        '-U_FORTIFY_SOURCE',
        '-D_FORTIFY_SOURCE=1',

        # We can't use the skia_shared_library gyp setting because we need to
        # isolate this define to Skia sources. CFLAGS are local to Android.mk
        # and ensures that this define is not exported to clients of the library
        '-DSKIA_IMPLEMENTATION=1',
      ],
      # Remove flags which are either unnecessary or problematic for the
      # Android framework build. Many of these flags are removed simply because
      # they were not previously in the Android framework makefile, and we did
      # did not intend to add them when generating the makefile.
      # TODO (scroggo): Investigate whether any of these flags are actually
      # needed/would be beneficial.
      'cflags!': [
        # Android has one makefile, used for both debugging (after manual
        # modification) and release. Turn off debug info by default.
        '-g',
        '-march=armv7-a',
        '-mthumb',
        '-mfpu=neon',
        '-mfloat-abi=softfp',
        '-fno-exceptions',
        '-fstrict-aliasing',
        # Remove flags to turn on warnings, since most people building Android
        # are not focused on Skia and do not need the extra warning info.
        '-Wall',
        '-Wextra',
        '-Winit-self',
        '-Wpointer-arith',
        '-Wsign-compare',
      ],
      'cflags_cc!': [
        '-fno-rtti',
        '-Wnon-virtual-dtor',
      ],
      'defines': [
        'DCT_IFAST_SUPPORTED',
        # using freetype's embolden allows us to adjust fake bold settings at
        # draw-time, at which point we know which SkTypeface is being drawn
        'SK_USE_FREETYPE_EMBOLDEN',
        'SK_SFNTLY_SUBSETTER "sample/chromium/font_subsetter.h"',
        # When built as part of the system image we can enable certian non-NDK
        # compliant optimizations.
        'SK_BUILD_FOR_ANDROID_FRAMEWORK',
        # Optimizations for chromium (m30)
        'GR_GL_CUSTOM_SETUP_HEADER "gl/GrGLConfig_chrome.h"',
        'IGNORE_ROT_AA_RECT_OPT',
        'SK_DEFAULT_FONT_CACHE_LIMIT   (768 * 1024)',
        'SK_DEFAULT_GLOBAL_DISCARDABLE_MEMORY_POOL_SIZE (512 * 1024)',
        'SK_IGNORE_ETC1_SUPPORT',
        # We can't use the skia_shared_library gyp setting because we need expose
        # this define globally and the the implemention define as a cflag.
        'SKIA_DLL',
        'SK_OVERRIDE override',
        # Defines from skia_for_android_framework_defines.gypi
        '<@(skia_for_android_framework_defines)',
      ],
    }],

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
            'defines': [
              'SKIA_DLL',
              'SKIA_IMPLEMENTATION=1',
            ],
          }],
          [ 'skia_os == "nacl"', {
            'defines': [
              'SK_BUILD_FOR_NACL',
            ],
            'variables': {
              'nacl_sdk_root': '<!(echo ${NACL_SDK_ROOT})',
            },
            'link_settings': {
              'libraries': [
                '-lppapi',
                '-lppapi_cpp',
                '-lnosys',
                '-pthread',
              ],
              'ldflags': [
                '-L<(nacl_sdk_root)/lib/newlib_x86_<(skia_arch_width)/Release',
                '-L<(nacl_sdk_root)/ports/lib/newlib_x86_<(skia_arch_width)/Release',
              ],
            },
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
                'defines': [ 'THREAD_SANITIZER' ],
              }],
              [ 'skia_sanitizer == "undefined"', {
                'cflags_cc!': ['-fno-rtti'],
              }],
            ],
          }],
          [ 'skia_clang_build', {
            'cflags_cc': [
                '-Wno-unknown-warning-option',  # Allows unknown warnings.
                '-Wno-deprecated',              # From Qt, via debugger (older Clang).
                '-Wno-deprecated-register',     # From Qt, via debugger (newer Clang).
            ],
            'cflags': [
                # Extra warnings we like but that only Clang knows about.
                '-Wstring-conversion',
            ],
            'cflags!': [
                '-mfpmath=sse',  # Clang doesn't need to be told this, and sometimes gets confused.
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
        'defines': [ 'SK_BUILD_FOR_MAC' ],
        'configurations': {
          'Coverage': {
            'xcode_settings': {
               'GCC_OPTIMIZATION_LEVEL': '0',
               'GCC_GENERATE_TEST_COVERAGE_FILES': 'YES',
               'GCC_INSTRUMENT_PROGRAM_FLOW_ARCS' : 'YES',
            },
          },
          'Debug': {
            'xcode_settings': { 'GCC_OPTIMIZATION_LEVEL': '0' },
          },
          'Release': {
            'xcode_settings': { 'GCC_OPTIMIZATION_LEVEL': '<(skia_release_optimization_level)', },
            'defines': [ 'NDEBUG' ],
          },
        },
        'xcode_settings': {
          'conditions': [
            [ 'skia_warnings_as_errors', { 'GCC_TREAT_WARNINGS_AS_ERRORS': 'YES' }],
            [ 'skia_arch_width == 32', { 'ARCHS': ['i386']   }],
            [ 'skia_arch_width == 64', { 'ARCHS': ['x86_64'] }],
            [ 'skia_osx_deployment_target==""', {
              'MACOSX_DEPLOYMENT_TARGET': '10.6', # -mmacos-version-min, passed in env to ld.
            }, {
              'MACOSX_DEPLOYMENT_TARGET': '<(skia_osx_deployment_target)',
            }],
          ],
          'CLANG_CXX_LANGUAGE_STANDARD':               'c++11',
          'GCC_ENABLE_SUPPLEMENTAL_SSE3_INSTRUCTIONS': 'YES',  # -mssse3
          'GCC_SYMBOLS_PRIVATE_EXTERN':                'NO',   # -fvisibility=hidden
          'GCC_INLINES_ARE_PRIVATE_EXTERN':            'NO',   # -fvisibility-inlines-hidden
          'GCC_CW_ASM_SYNTAX':                         'NO',   # remove -fasm-blocks
          'GCC_ENABLE_PASCAL_STRINGS':                 'NO',   # remove -mpascal-strings
          'GCC_WARN_ABOUT_INVALID_OFFSETOF_MACRO':     'NO',   # -Wno-invalid-offsetof
          'WARNING_CFLAGS': [
            '-Wall',
            '-Wextra',
            '-Winit-self',
            '-Wpointer-arith',
            '-Wsign-compare',

            '-Wno-unused-parameter',
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
          'ARCHS': ['armv7'],
          'CODE_SIGNING_REQUIRED': 'NO',
          'CODE_SIGN_IDENTITY[sdk=iphoneos*]': '',
          'IPHONEOS_DEPLOYMENT_TARGET': '<(ios_sdk_version)',
          'SDKROOT': 'iphoneos',
          'TARGETED_DEVICE_FAMILY': '1,2',
          'GCC_WARN_ABOUT_INVALID_OFFSETOF_MACRO': 'NO',   # -Wno-invalid-offsetof
          'OTHER_CPLUSPLUSFLAGS': [
            '-std=c++0x',
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

          # Android Text Tuning
          'SK_GAMMA_EXPONENT=1.4',
          'SK_GAMMA_CONTRAST=0.0',
        ],
        # Android defines a fixed gamma exponent instead of using SRGB
        'defines!': [
          'SK_GAMMA_SRGB',
        ],
        'configurations': {
          'Debug': {
            'cflags': ['-g']
          },
          'Release': {
            'cflags': ['-O2'],
          },
        },
        'libraries': [
          '-llog',
        ],
        'cflags': [
          '-fuse-ld=gold',
        ],
        'conditions': [
          [ '"x86" in skia_arch_type', {
            'cflags': [
              '-mssse3',
            ],
          }],
          [ 'skia_android_framework', {
            'cflags!': [
              '-fuse-ld=gold',
              '-mssse3',
            ],
          }],
          [ 'skia_shared_lib', {
            'defines': [
              'SKIA_DLL',
              'SKIA_IMPLEMENTATION=1',
              # Needed until we fix skbug.com/2440.
              'SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG',
            ],
          }],
          [ 'skia_profile_enabled == 1', {
            'cflags': ['-g', '-fno-omit-frame-pointer', '-marm', '-mapcs'],
          }],
        ],
      },
    ],

    [ 'skia_moz2d', {
      'defines': [
        # add flags here (e.g. SK_SUPPORT_LEGACY_...) needed by moz2d
      ],
    }],

  ], # end 'conditions'
  # The Xcode SYMROOT must be at the root. See build/common.gypi in chromium for more details
  'xcode_settings': {
    'SYMROOT': '<(DEPTH)/xcodebuild',
  },
}
