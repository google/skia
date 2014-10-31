# GYP file to build various tools.
#
# To build on Linux:
#  ./gyp_skia tools.gyp && make tools
#
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      # Build all executable targets defined below.
      'target_name': 'tools',
      'type': 'none',
      'dependencies': [
        'bbh_shootout',
        'bench_pictures',
        'dump_record',
        'filter',
        'gpuveto',
        'lua_app',
        'lua_pictures',
        'pinspect',
        'render_pdfs',
        'render_pictures',
        'skdiff',
        'skhello',
        'skpdiff',
        'skpinfo',
        'skpmaker',
        'skimage',
        'test_image_decoder',
      ],
      'conditions': [
        ['skia_shared_lib',
          {
            'dependencies': [
              'sklua', # This can only be built if skia is built as a shared library
            ],
          },
        ],
      ],
    },
    {  # This would go in gm.gyp, but it's also used by skimage below.
      'target_name': 'gm_expectations',
      'type': 'static_library',
      'include_dirs' : [ '../src/utils/' ],
      'sources': [
        '../gm/gm_expectations.cpp',
      ],
      'dependencies': [
        'jsoncpp.gyp:jsoncpp',
        'sk_tool_utils',
        'skia_lib.gyp:skia_lib',
      ],
      'direct_dependent_settings': {
        'include_dirs': [ '../gm/' ],
      },
    },
    {
      'target_name': 'crash_handler',
        'type': 'static_library',
        'sources': [ '../tools/CrashHandler.cpp' ],
        'dependencies': [ 'skia_lib.gyp:skia_lib' ],
        'direct_dependent_settings': {
          'include_dirs': [ '../tools' ],
        },
        'all_dependent_settings': {
          'msvs_settings': {
            'VCLinkerTool': {
              'AdditionalDependencies': [ 'Dbghelp.lib' ],
            }
          },
        }
    },
    {
      'target_name': 'resources',
      'type': 'static_library',
      'sources': [ '../tools/Resources.cpp' ],
      'dependencies': [
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
      ],
      'direct_dependent_settings': {
        'include_dirs': [ '../tools', ],
      },
    },
    {
      'target_name': 'sk_tool_utils',
      'type': 'static_library',
      'sources': [
        '../tools/sk_tool_utils.cpp',
        '../tools/sk_tool_utils_font.cpp',
      ],
      'include_dirs': [
        '../src/fonts',
      ],
      'dependencies': [
        'resources',
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
      ],
      'direct_dependent_settings': {
        'include_dirs': [ '../tools', ],
      },
    },
    {
      'target_name' : 'timer',
      'type': 'static_library',
      'sources': [
        '../tools/timer/Timer.cpp',
        '../tools/timer/TimerData.cpp',
      ],
      'include_dirs': [
        '../src/core',
        '../src/gpu',
      ],
      'direct_dependent_settings': {
        'include_dirs': ['../tools/timer'],
      },
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'jsoncpp.gyp:jsoncpp',
      ],
      'conditions': [
        ['skia_gpu == 1', {
          'sources': [ '../tools/timer/GpuTimer.cpp' ],
        }],
        [ 'skia_os in ["mac", "ios"]', {
          'sources': [ '../tools/timer/SysTimer_mach.cpp' ],
        }],
        [ 'skia_os == "win"', {
          'sources': [ '../tools/timer/SysTimer_windows.cpp' ],
        }],
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "android", "chromeos"]', {
          'sources': [ '../tools/timer/SysTimer_posix.cpp' ],
        }],
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "chromeos"]', {
          'link_settings': { 'libraries': [ '-lrt' ] },
        }],
      ],
    },
    {
      'target_name': 'skdiff',
      'type': 'executable',
      'sources': [
        '../tools/skdiff.cpp',
        '../tools/skdiff.h',
        '../tools/skdiff_html.cpp',
        '../tools/skdiff_html.h',
        '../tools/skdiff_main.cpp',
        '../tools/skdiff_utils.cpp',
        '../tools/skdiff_utils.h',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
      'target_name': 'skpdiff',
      'type': 'executable',
      'sources': [
        '../tools/skpdiff/skpdiff_main.cpp',
        '../tools/skpdiff/SkDiffContext.cpp',
        '../tools/skpdiff/SkImageDiffer.cpp',
        '../tools/skpdiff/SkPMetric.cpp',
        '../tools/skpdiff/skpdiff_util.cpp',
      ],
      'include_dirs': [
        '../src/core/', # needed for SkTLList.h
        '../tools/',    # needed for picture_utils::replace_char
      ],
      'dependencies': [
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:picture_utils',
      ],
      'cflags': [
        '-O3',
      ],
      'conditions': [
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "chromeos"]', {
          'link_settings': {
            'libraries': [
              '-lrt',
            ],
          },
        }],
        ['skia_opencl', {
          'sources': [
            '../tools/skpdiff/SkCLImageDiffer.cpp',
            '../tools/skpdiff/SkDifferentPixelsMetric_opencl.cpp',
          ],
          'conditions': [
            [ 'skia_os == "mac"', {
              'link_settings': {
                'libraries': [
                  '$(SDKROOT)/System/Library/Frameworks/OpenCL.framework',
                ]
              }
            }, {
              'link_settings': {
                'libraries': [
                  '-lOpenCL',
                ],
              },
            }],
          ],
        }, { # !skia_opencl
          'sources': [
            '../tools/skpdiff/SkDifferentPixelsMetric_cpu.cpp',
          ],
        }],
      ],
    },
    {
      'target_name': 'skpmaker',
      'type': 'executable',
      'sources': [
        '../tools/skpmaker.cpp',
      ],
      'include_dirs': [
        '../src/core',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
      'target_name': 'skimagediff',
      'type': 'executable',
      'sources': [
        '../tools/skdiff.cpp',
        '../tools/skdiff.h',
        '../tools/skdiff_html.cpp',
        '../tools/skdiff_html.h',
        '../tools/skdiff_image.cpp',
        '../tools/skdiff_utils.cpp',
        '../tools/skdiff_utils.h',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
      'target_name': 'skhello',
      'type': 'executable',
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
      'conditions': [
        [ 'skia_os == "nacl"', {
          'sources': [
            '../platform_tools/nacl/src/nacl_hello.cpp',
          ],
        }, {
          'sources': [
            '../tools/skhello.cpp',
          ],
          'dependencies': [
            'flags.gyp:flags',
            'pdf.gyp:pdf',
          ],
        }],
      ],
    },
    {
      'target_name': 'skimage',
      'type': 'executable',
      'sources': [
        '../tools/skimage_main.cpp',
      ],
      'include_dirs': [
        # For SkBitmapHasher.h
        '../src/utils/',
        '../tools/',
      ],
      'dependencies': [
        'gm_expectations',
        'flags.gyp:flags',
        'jsoncpp.gyp:jsoncpp',
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
      'target_name': 'skpinfo',
      'type': 'executable',
      'sources': [
        '../tools/skpinfo.cpp',
      ],
      'include_dirs': [
        '../src/core/',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
      'target_name': 'gpuveto',
      'type': 'executable',
      'sources': [
        '../tools/gpuveto.cpp',
        '../tools/LazyDecodeBitmap.cpp',
      ],
      'include_dirs': [
        '../src/core/',
        '../src/images',
        '../src/lazy',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
      'target_name': 'lua_app',
      'type': 'executable',
      'sources': [
        '../tools/lua/lua_app.cpp',
        '../src/utils/SkLua.cpp',
      ],
      'include_dirs': [
        # Lua exposes GrReduceClip which in turn requires src/core for SkTLList
        '../src/gpu/',
        '../src/core/',
      ],
      'dependencies': [
        'effects.gyp:effects',
        'images.gyp:images',
        'lua.gyp:lua',
        'pdf.gyp:pdf',
        'ports.gyp:ports',
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
      'target_name': 'lua_pictures',
      'type': 'executable',
      'sources': [
        '../tools/lua/lua_pictures.cpp',
        '../src/utils/SkLuaCanvas.cpp',
        '../src/utils/SkLua.cpp',
      ],
      'include_dirs': [
        # Lua exposes GrReduceClip which in turn requires src/core for SkTLList
        '../src/gpu/',
        '../src/core/',
      ],
      'dependencies': [
        'effects.gyp:effects',
        'flags.gyp:flags',
        'images.gyp:images',
        'lua.gyp:lua',
        'tools.gyp:picture_renderer',
        'tools.gyp:picture_utils',
        'pdf.gyp:pdf',
        'ports.gyp:ports',
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
      'target_name': 'render_pictures',
      'type': 'executable',
      'sources': [
        '../tools/render_pictures_main.cpp',
      ],
      'include_dirs': [
        '../src/core',
        '../src/images',
        '../src/lazy',
        '../src/pipe/utils/',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:picture_renderer',
        'tools.gyp:picture_utils',
      ],
    },
    {
      'target_name': 'bench_pictures',
      'type': 'executable',
      'sources': [
        '../bench/BenchLogger.cpp',
        '../bench/BenchLogger.h',
        '../tools/PictureBenchmark.cpp',
        '../tools/PictureResultsWriter.h',
        '../tools/bench_pictures_main.cpp',
      ],
      'include_dirs': [
        '../src/core/',
        '../bench',
        '../src/lazy/',
      ],
      'dependencies': [
        'timer',
        'crash_handler',
        'flags.gyp:flags',
        'jsoncpp.gyp:jsoncpp',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:picture_renderer',
        'tools.gyp:picture_utils',
      ],
      'conditions': [
        ['skia_android_framework == 1', {
          'libraries': [ '-lskia' ],
        }],
      ],
    },
    {
      'target_name': 'dump_record',
      'type': 'executable',
      'sources': [
        '../tools/dump_record.cpp',
        '../tools/DumpRecord.cpp',
        '../tools/LazyDecodeBitmap.cpp',
      ],
      'include_dirs': [
        '../src/core/',
        '../src/images',
        '../src/lazy',
      ],
      'dependencies': [
        'timer',
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
      'target_name': 'picture_renderer',
      'type': 'static_library',
      'sources': [
        '../tools/image_expectations.h',
        '../tools/image_expectations.cpp',
        '../tools/LazyDecodeBitmap.cpp',
        '../tools/PictureRenderer.h',
        '../tools/PictureRenderer.cpp',
        '../tools/PictureRenderingFlags.h',
        '../tools/PictureRenderingFlags.cpp',
        '../tools/CopyTilesRenderer.h',
        '../tools/CopyTilesRenderer.cpp',
        '../src/pipe/utils/SamplePipeControllers.h',
        '../src/pipe/utils/SamplePipeControllers.cpp',
      ],
      'include_dirs': [
        '../src/core',
        '../src/images',
        '../src/lazy',
        '../src/pipe/utils/',
        '../src/utils/',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          # needed for JSON headers used within image_expectations.h
          '../third_party/externals/jsoncpp-chromium/overrides/include/',
          '../third_party/externals/jsoncpp/include/',
        ],
      },
      'dependencies': [
        'flags.gyp:flags',
        'jsoncpp.gyp:jsoncpp',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:picture_utils',
      ],
      'conditions': [
        ['skia_gpu == 1',
          {
            'include_dirs' : [
              '../src/gpu',
            ],
            'dependencies': [
              'gputest.gyp:skgputest',
            ],
          },
        ],
      ],
    },
    {
      'target_name': 'render_pdfs',
      'type': 'executable',
      'sources': [
        '../tools/render_pdfs_main.cpp',
      ],
      'include_dirs': [
        '../src/core',
        '../src/pipe/utils/',
        '../src/utils/',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'pdf.gyp:pdf',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:picture_utils',
        'tools.gyp:proc_stats',
      ],
      'conditions': [
        ['skia_win_debuggers_path and skia_os == "win"',
          {
            'dependencies': [
              'tools.gyp:win_dbghelp',
            ],
          },
        ],
        # VS static libraries don't have a linker option. We must set a global
        # project linker option, or add it to each executable.
        ['skia_win_debuggers_path and skia_os == "win" and '
         'skia_arch_width == 64',
          {
            'msvs_settings': {
              'VCLinkerTool': {
                'AdditionalDependencies': [
                  '<(skia_win_debuggers_path)/x64/DbgHelp.lib',
                ],
              },
            },
          },
        ],
        ['skia_win_debuggers_path and skia_os == "win" and '
         'skia_arch_width == 32',
          {
            'msvs_settings': {
              'VCLinkerTool': {
                'AdditionalDependencies': [
                  '<(skia_win_debuggers_path)/DbgHelp.lib',
                ],
              },
            },
          },
        ],
      ],
    },
    {
      'target_name': 'picture_utils',
      'type': 'static_library',
      'sources': [
        '../tools/picture_utils.cpp',
        '../tools/picture_utils.h',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../tools/',
        ],
      },
    },
    {
      'target_name': 'pinspect',
      'type': 'executable',
      'sources': [
        '../tools/pinspect.cpp',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:picture_renderer',
      ],
    },
    {
      'target_name': 'bbh_shootout',
      'type': 'executable',
      'include_dirs': [
        '../bench',
        '../tools/'
      ],
      'sources': [
        '../tools/bbh_shootout.cpp',

        # Bench code:
      ],
      'dependencies': [
        'timer',
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:picture_renderer',
        'tools.gyp:picture_utils',
      ],
    },
    {
      'target_name': 'filter',
      'type': 'executable',
      'include_dirs' : [
        '../src/core',
        '../src/utils/debugger',
      ],
      'sources': [
        '../tools/filtermain.cpp',
        '../src/utils/debugger/SkDrawCommand.h',
        '../src/utils/debugger/SkDrawCommand.cpp',
        '../src/utils/debugger/SkDebugCanvas.h',
        '../src/utils/debugger/SkDebugCanvas.cpp',
        '../src/utils/debugger/SkObjectParser.h',
        '../src/utils/debugger/SkObjectParser.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'tools.gyp:picture_utils',
      ],
    },
    {
      'target_name': 'test_image_decoder',
      'type': 'executable',
      'sources': [
        '../tools/test_image_decoder.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
      'target_name': 'proc_stats',
      'type': 'static_library',
      'sources': [
        '../tools/ProcStats.h',
        '../tools/ProcStats.cpp',
      ],
      'direct_dependent_settings': {
        'include_dirs': [ '../tools', ],
      },
    },
  ],
  'conditions': [
    ['skia_shared_lib',
      {
        'targets': [
          {
            'target_name': 'sklua',
            'product_name': 'skia',
            'product_prefix': '',
            'product_dir': '<(PRODUCT_DIR)/',
            'type': 'shared_library',
            'sources': [
              '../src/utils/SkLuaCanvas.cpp',
              '../src/utils/SkLua.cpp',
            ],
            'include_dirs': [
              # Lua exposes GrReduceClip which in turn requires src/core for SkTLList
              '../src/gpu/',
              '../src/core/',
              '../third_party/lua/src/',
            ],
            'dependencies': [
              'lua.gyp:lua',
              'pdf.gyp:pdf',
              'skia_lib.gyp:skia_lib',
            ],
            'conditions': [
              ['skia_os != "win"',
                {
                  'ldflags': [
                    '-Wl,-rpath,\$$ORIGIN,--enable-new-dtags',
                  ],
                },
              ],
            ],
          },
        ],
      },
    ],
    ['skia_win_debuggers_path and skia_os == "win"',
      {
        'targets': [
          {
            'target_name': 'win_dbghelp',
            'type': 'static_library',
            'defines': [
              'SK_CDB_PATH="<(skia_win_debuggers_path)"',
            ],
            'sources': [
              '../tools/win_dbghelp.h',
              '../tools/win_dbghelp.cpp',
            ],
          },
        ],
      },
    ],
    ['skia_os == "win"',
      {
        'targets': [
          {
            'target_name': 'win_lcid',
            'type': 'executable',
            'sources': [
              '../tools/win_lcid.cpp',
            ],
          },
        ],
      },
    ],
    ['skia_os == "mac"',
      {
        'targets': [
          {
            'target_name': 'create_test_font',
            'type': 'executable',
            'sources': [
              '../tools/create_test_font.cpp',
            ],
            'include_dirs': [
              '../src/core',
            ],
            'dependencies': [
              'flags.gyp:flags',
              'skia_lib.gyp:skia_lib',
              'resources',
            ],
          },
        ],
      },
    ],
  ],
}
