# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# conditions used in both common.gypi and skia.gyp in chromium
#
{
  'defines': [
    'SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=<(skia_static_initializers)',
    'SK_SUPPORT_GPU=<(skia_gpu)',
    'SK_FORCE_DISTANCE_FIELD_TEXT=<(skia_force_distance_field_text)',
    
    # Indicate that all dependency libraries are present.  Clients that
    # are missing some of the required decoding libraries may choose
    # not to define these.  This will disable some decoder and encoder
    # features.
    'SK_HAS_GIF_LIBRARY',
    'SK_HAS_JPEG_LIBRARY',
    'SK_HAS_PNG_LIBRARY',
    'SK_HAS_WEBP_LIBRARY',

    # Temporarily test against the QCMS library.
    'SK_TEST_QCMS',
  ],
  'conditions' : [
    [ 'skia_is_bot', {
      'defines': [ 'SK_IS_BOT' ],
    }],
    [ 'skia_codec_decodes_raw', {
      'defines': [
        'SK_CODEC_DECODES_RAW',
      ],
    }],
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
          '_HAS_EXCEPTIONS=0',
          'WIN32_LEAN_AND_MEAN',
          'NOMINMAX',
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
          # See https://bug.skia.org/2348
          #
          # We handle the 64- vs 32-bit variations elsewhere, so I think it's
          # OK for us to just make these inherit non-archwidth-specific
          # configurations without modification.
          #
          # See https://bug.skia.org/2442 : These targets cause problems in the
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
          [ 'skia_arch_type == "x86_64"', {
            'msvs_configuration_platform': 'x64',
          }],
          [ 'skia_arch_type == "x86"', {
            'msvs_configuration_platform': 'Win32',
          }],
          [ 'skia_warnings_as_errors', {
            'msvs_settings': {
              'VCCLCompilerTool': {
                'WarnAsError': 'true',
                'AdditionalOptions': [
                  '/we4189', # initialized but unused var warning
                  '/we4238', # taking address of rvalue
                  '/we4239', # assigning rvalues to non-const lvalues
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
    [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "android"]',
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
          '-Wvla',

          '-Wno-unused-parameter',
        ],
        'cflags_cc': [
          '-std=c++11',
          '-fno-rtti',
          '-fno-threadsafe-statics',
          '-Wnon-virtual-dtor',
        ],
        'ldflags': [ '-rdynamic' ],
        'conditions': [
          [ 'skia_fast', { 'cflags': [ '<@(skia_fast_flags)' ] }],
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
          [ 'skia_arch_type == "arm" and arm_version >= 7', {
            'cflags': [
              '-march=armv7-a',
              '-mthumb',
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
              [ 'skia_os != "linux"', {
                'cflags': [
                  '-mfloat-abi=softfp',
                ],
              }],
            ],
          }],
          [ '"mips" in skia_arch_type', {
            'target_conditions': [
              [ '_toolset == "target"', {
                'cflags' : ['-EL'],
                'conditions' : [
                  [ 'mips_arch_variant == "mips32r2"', {
                    'cflags': [ '-march=mips32r2' ],
                    'conditions': [
                      [ 'mips_dsp == 1', { 'cflags': [ '-mdsp'   ] }],
                      [ 'mips_dsp == 2', { 'cflags': [ '-mdspr2' ] }],
                    ],
                  }],
                ],
              }],
            ],
          }],
        ],
      },
    ],

    ['skia_android_framework', {
      'cflags': [
        # Skia does not enforce this usage pattern so we disable it here to avoid
        # unecessary log spew when building
        '-Wno-unused-parameter',

        # Android's -D_FORTIFY_SOURCE=2 extensions are incompatibile with SkString.
        # Revert to -D_FORTIFY_SOURCE=1
        '-U_FORTIFY_SOURCE',
        '-D_FORTIFY_SOURCE=1',

        # We can't use the skia_shared_lib gyp setting because we need to
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
        'SK_DEFAULT_FONT_CACHE_LIMIT   (768 * 1024)',
        'SK_DEFAULT_GLOBAL_DISCARDABLE_MEMORY_POOL_SIZE (512 * 1024)',
        'SK_IGNORE_ETC1_SUPPORT',
        # We can't use the skia_shared_lib gyp setting because we need expose
        # this define globally and the the implemention define as a cflag.
        'SKIA_DLL',
        'SK_PRINT_CODEC_MESSAGES',
      ],
    }],

    ['skia_use_android_framework_defines', {
      # Add these defines when building for the Android framework, or when
      # specifically requested. These should be temporary staging defines. Any
      # permanent defines should be moved into the skia_android_framework block
      # above.
      'includes' : [
        'skia_for_android_framework_defines.gypi',
      ],
      'defines': [
        '<@(skia_for_android_framework_defines)',
      ],
    }],

    [ 'skia_use_sdl == 1',
      {
        'defines': [ 'SK_USE_SDL' ],
    }],

    [ 'skia_dump_stats == 1',
      {
        'defines': [ 'SK_DUMP_STATS'],
    }],

    [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris"]',
      {
        'defines': [
          'SK_SAMPLES_FOR_X',
          'SK_BUILD_FOR_UNIX',
        ],
        'configurations': {
          'Coverage': {
            'conditions': [
              [ 'skia_clang_build', {
                'cflags': ['-fprofile-instr-generate', '-fcoverage-mapping'],
                'ldflags': ['-fprofile-instr-generate', '-fcoverage-mapping'],
              }, {
                'cflags': ['--coverage'],
                'ldflags': ['--coverage'],
              }],
            ],
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
          # Enable asan, tsan, etc.
          [ 'skia_sanitizer', {
            'cflags_cc!': [ '-fno-rtti' ],                        # vptr needs rtti
            'cflags': [
              '-fsanitize=<(skia_sanitizer)',                     # Turn on sanitizers.
              '-fno-sanitize-recover=<(skia_sanitizer)',          # Make any failure fatal.
              '-fsanitize-blacklist=<(skia_sanitizer_blacklist)', # Compile in our blacklist.
              '-include <(skia_sanitizer_blacklist)',             # Make every .cpp depend on it.
            ],
            'ldflags': [ '-fsanitize=<(skia_sanitizer)' ],
            'conditions' : [
              [ 'skia_sanitizer == "thread"', {
                'defines': [ 'THREAD_SANITIZER' ],
              }],
              [ 'skia_sanitizer == "memory"', {
                'cflags': [
                    '-O1',
                    '-fsanitize-memory-track-origins',
                ],
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
        'conditions': [
            # ANGLE for mac hits -Wunneeded-internal-declaration if this isn't set.
            [ 'skia_angle', { 'defines': [ 'YY_NO_INPUT' ], } ],
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
            'xcode_settings': { 'GCC_OPTIMIZATION_LEVEL': '0' },
          },
          'Release': {
            'xcode_settings': { 'GCC_OPTIMIZATION_LEVEL': '<(skia_release_optimization_level)', },
            'defines': [ 'NDEBUG' ],
          },
        },
        'xcode_settings': {
          'conditions': [
            [ 'skia_fast', { 'WARNING_CFLAGS': [ '<@(skia_fast_flags)' ] } ],
            [ 'skia_warnings_as_errors', { 'GCC_TREAT_WARNINGS_AS_ERRORS': 'YES' }],
            [ 'skia_arch_type == "x86"', { 'ARCHS': ['i386']   }],
            [ 'skia_arch_type == "x86_64"', { 'ARCHS': ['x86_64'] }],
            [ 'skia_osx_deployment_target==""', {
              'MACOSX_DEPLOYMENT_TARGET': '10.7', # -mmacos-version-min, passed in env to ld.
            }, {
              'MACOSX_DEPLOYMENT_TARGET': '<(skia_osx_deployment_target)',
            }],
            [ 'skia_sanitizer', {
              'GCC_ENABLE_CPP_RTTI': 'YES',                         # vptr needs rtti
              'OTHER_CFLAGS': [
                '-fsanitize=<(skia_sanitizer)',                     # Turn on sanitizers.
                '-fno-sanitize-recover=<(skia_sanitizer)',          # Make any failure fatal.
                '-fsanitize-blacklist=<(skia_sanitizer_blacklist)', # Compile in our blacklist.
                '-include <(skia_sanitizer_blacklist)',             # Make every .cpp depend on it.
              ],
              # We want to pass -fsanitize=... to our final link call,
              # but not to libtool. OTHER_LDFLAGS is passed to both.
              # To trick GYP into doing what we want, we'll piggyback on
              # LIBRARY_SEARCH_PATHS, producing "-L/usr/lib -fsanitize=...".
              # The -L/usr/lib is redundant but innocuous: it's a default path.
              'LIBRARY_SEARCH_PATHS': [ '/usr/lib -fsanitize=<(skia_sanitizer)'],
            }],
          ],
          'CLANG_CXX_LIBRARY':                         'libc++',
          'CLANG_CXX_LANGUAGE_STANDARD':               'c++11',
          'GCC_ENABLE_CPP_EXCEPTIONS':                 'NO',   # -fno-exceptions
          'GCC_ENABLE_CPP_RTTI':                       'NO',   # -fno-rtti
          'GCC_THREADSAFE_STATICS':                    'NO',   # -fno-threadsafe-statics
          'GCC_ENABLE_SUPPLEMENTAL_SSE3_INSTRUCTIONS': 'YES',  # -mssse3
          'GCC_SYMBOLS_PRIVATE_EXTERN':                'YES',  # -fvisibility=hidden
          'GCC_INLINES_ARE_PRIVATE_EXTERN':            'YES',  # -fvisibility-inlines-hidden
          'GCC_CW_ASM_SYNTAX':                         'NO',   # remove -fasm-blocks
          'GCC_ENABLE_PASCAL_STRINGS':                 'NO',   # remove -mpascal-strings
          'WARNING_CFLAGS': [
            '-Wall',
            '-Wextra',
            '-Winit-self',
            '-Wpointer-arith',
            '-Wsign-compare',
            '-Wvla',

            '-Wno-unused-parameter',
          ],
        },
      },
    ],

    [ 'skia_os == "ios"',
      {
        'defines': [
          # When targetting iOS and using gyp to generate the build files, it is
          # not possible to select files to build depending on the architecture
          # (i.e. it is not possible to use hand optimized assembly version). In
          # that configuration, disable all optimisation.
          'SK_BUILD_FOR_IOS',
          'SK_BUILD_NO_OPTS',
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
          'IPHONEOS_DEPLOYMENT_TARGET': '<(ios_sdk_version)',
          'SDKROOT': 'iphoneos',
          'TARGETED_DEVICE_FAMILY': '1,2',

          'CLANG_CXX_LIBRARY':              'libc++',
          'CLANG_CXX_LANGUAGE_STANDARD':    'c++11',
          'GCC_ENABLE_CPP_EXCEPTIONS':      'NO',   # -fno-exceptions
          'GCC_ENABLE_CPP_RTTI':            'NO',   # -fno-rtti
          'GCC_THREADSAFE_STATICS':         'NO',   # -fno-threadsafe-statics
          'GCC_SYMBOLS_PRIVATE_EXTERN':     'YES',  # -fvisibility=hidden
          'GCC_INLINES_ARE_PRIVATE_EXTERN': 'YES',  # -fvisibility-inlines-hidden

          'GCC_THUMB_SUPPORT': 'NO',  # TODO(mtklein): why would we not want thumb?
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
          'Release': {
            'cflags': ['-O2'],
            'conditions': [
              [ 'skia_clang_build', {
                'cflags!': ['-g'],
                'cflags': [ '-gline-tables-only' ],
              }],
            ],
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
              # Needed until we fix https://bug.skia.org/2440 .
              'SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG',
            ],
          }],
          [ 'skia_profile_enabled == 1', {
            'cflags': ['-g', '-fno-omit-frame-pointer', '-marm', '-mapcs'],
          }],
          [ 'skia_clang_build', {
            'cflags': [
                '-Wno-unknown-warning-option', # Allows unknown warnings
                # These flags that are on by default for only the android
                # toolchain and no other platforms.
                '-Wno-tautological-compare',
                '-Wno-unused-command-line-argument',
            ],
          }],
        ],
      },
    ],

    [ 'skia_moz2d', {
      'defines': [
        # add flags here (e.g. SK_SUPPORT_LEGACY_...) needed by moz2d
      ],
    }],

    [ 'skia_command_buffer and skia_os == "linux"', {
      'ldflags': [
          '-Wl,-rpath,\$$ORIGIN/lib',
      ],
    }],

    [ 'skia_command_buffer and skia_os == "mac"', {
      'xcode_settings': {
          'LD_RUNPATH_SEARCH_PATHS': ['@executable_path/.'],
      },
    }],

  ], # end 'conditions'
  # The Xcode SYMROOT must be at the root. See build/common.gypi in chromium for more details
  'xcode_settings': {
    'SYMROOT': '<(DEPTH)/xcodebuild',
  },
}
