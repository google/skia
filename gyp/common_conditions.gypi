# conditions used in both common.gypi and skia.gyp in chromium
#
{
  'defines': [
    'SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=<(skia_static_initializers)',
    'SK_SUPPORT_GPU=<(skia_gpu)',
    'SK_SUPPORT_OPENCL=<(skia_opencl)',
    'SK_FORCE_DISTANCEFIELD_FONTS=<(skia_force_distancefield_fonts)',
  ],
  'conditions' : [
    [ 'skia_arch_type == "arm64"', {
      'cflags': [
        '-ffp-contract=off',
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
            4345,  # This is an FYI about a behavior change from long ago.  Chrome stifles it too.
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
              },
              'Release_x64': {
                'inherit_from': ['Release'],
              },
              'Release_Developer_x64': {
                'inherit_from': ['Release_Developer'],
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
        ],
        'cflags_cc': [
          '-fno-rtti',
          '-Wnon-virtual-dtor',
        ],
        'conditions': [
          [ 'skia_android_framework==0', {
            'cflags': [
              # This flag is not supported by Android build system.
              '-Wno-c++11-extensions',
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
                  }],
                  [ 'mips_dsp == 2', {
                    'cflags': [
                      '-mdspr2',
                    ],
                    'defines': [
                      '__MIPS_HAVE_DSPR2',
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
        # This flag is not supported by Android build system.
        '-Wno-c++11-extensions',
        '-fno-exceptions',
        '-fstrict-aliasing',
        # Remove flags to turn on warnings, since most people building Android
        # are not focused on Skia and do not need the extra warning info.
        '-Wall',
        '-Wextra',
        '-Winit-self',
        '-Wpointer-arith',
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
        # Disable this check because it is too strict for some chromium-specific
        # subclasses of SkPixelRef. See bug: crbug.com/171776.
        'SK_DISABLE_PIXELREF_LOCKCOUNT_BALANCE_CHECK',
        'SkLONGLONG int64_t',
        'SK_DEFAULT_FONT_CACHE_LIMIT   (768 * 1024)',
        'SK_ATOMICS_PLATFORM_H "../../src/ports/SkAtomics_sync.h"',
        'SK_MUTEX_PLATFORM_H "../../src/ports/SkMutex_pthread.h"',
        # Still need to switch Android to the new name for N32.
        'kNative_8888_SkColorType kN32_SkColorType',
        'SK_SUPPORT_LEGACY_BLURMASKFILTER_STYLE',
        # Needed until we fix skbug.com/2440.
        'SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG',
        # Transitional, for deprecated SkCanvas::SaveFlags methods.
        'SK_ATTR_DEPRECATED=SK_NOTHING_ARG1',
        'SK_SUPPORT_LEGACY_SHADER_LOCALMATRIX',
        'SK_DEFAULT_GLOBAL_DISCARDABLE_MEMORY_POOL_SIZE (512 * 1024)',
        'SK_IGNORE_ETC1_SUPPORT',
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
              [ 'skia_sanitizer == "undefined"', {
                'cflags': [ '-fPIC' ],
                'cflags_cc!': ['-fno-rtti'],
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
                '-Wno-uninitialized',  # Disabled because we think GCC 4.2 is bad at this.
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
            [ 'skia_osx_deployment_target==""', {
              'MACOSX_DEPLOYMENT_TARGET': '10.6', # -mmacos-version-min, passed in environment to ld.
            }, {
              'MACOSX_DEPLOYMENT_TARGET': '<(skia_osx_deployment_target)',
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
          'ARCHS': ['armv7'],
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
          [ 'skia_android_framework', {
            'libraries!': [
              '-lstdc++',
              '-lm',
            ],
            'cflags!': [
              '-fuse-ld=gold',
            ],
          }],
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

    [ 'skia_moz2d', {
      'defines': [
        'SK_SUPPORT_LEGACY_LAYERRASTERIZER_API=1',
        'SK_SUPPORT_LEGACY_COMPATIBLEDEVICE_CONFIG=1',
        'SK_SUPPORT_LEGACY_GETTOTALCLIP=1',
      ],
    }],

    [ 'skia_build_json_writer', {
      'defines': [
        'SK_BUILD_JSON_WRITER',
      ]
    }],

  ], # end 'conditions'
  # The Xcode SYMROOT must be at the root. See build/common.gypi in chromium for more details
  'xcode_settings': {
    'SYMROOT': '<(DEPTH)/xcodebuild',
  },
}
