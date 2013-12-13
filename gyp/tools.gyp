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
        'bench_pictures',
        'filter',
        'lua_pictures',
        'bbh_shootout',
        'lua_app',
        'pinspect',
        'render_pdfs',
        'render_pictures',
        'skdiff',
        'skpdiff',
        'skhello',
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
        '../tools/flags/SkCommandLineFlags.cpp',
      ],
      'include_dirs': [
        '../tools/flags',
        '../src/core/', # needed for SkTLList.h
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
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
      ],
      'dependencies': [
        'flags.gyp:flags',
        'gm.gyp:gm_expectations',
        'jsoncpp.gyp:jsoncpp',
        'skia_lib.gyp:skia_lib',
        'utils.gyp:utils',
      ],
    },

    {
      'target_name': 'lua_app',
      'type': 'executable',
      'sources': [
        '../tools/lua/lua_app.cpp',
        '../src/utils/SkLua.cpp',
      ],
      'dependencies': [
        'effects.gyp:effects',
        'images.gyp:images',
        'lua.gyp:lua',
        'pdf.gyp:pdf',
        'ports.gyp:ports',
        'skia_lib.gyp:skia_lib',
        'utils.gyp:utils',
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
        'utils.gyp:utils',
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
        '../bench/SkBenchLogger.h',
        '../bench/SkBenchLogger.cpp',
        '../bench/TimerData.h',
        '../bench/TimerData.cpp',
        '../tools/bench_pictures_main.cpp',
        '../tools/PictureBenchmark.cpp',
      ],
      'include_dirs': [
        '../src/core/',
        '../bench',
        '../src/lazy/',
      ],
      'dependencies': [
        'bench.gyp:bench_timer',
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:picture_utils',
        'tools.gyp:picture_renderer',
      ],
    },
    {
      'target_name': 'picture_renderer',
      'type': 'static_library',
      'sources': [
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
          # needed for JSON headers used within PictureRenderer.h
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
          },
        ],
      ],
    },
    {
      'target_name': 'render_pdfs',
      'type': 'executable',
      'sources': [
        '../tools/render_pdfs_main.cpp',
        '../tools/PdfRenderer.cpp',
        '../tools/PdfRenderer.h',
      ],
      'include_dirs': [
        '../src/pipe/utils/',
        '../src/utils/',
      ],
      'dependencies': [
        'pdf.gyp:pdf',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:picture_utils',
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
        '../bench/TimerData.h',
        '../bench/TimerData.cpp',
      ],
      'dependencies': [
        'bench.gyp:bench_timer',
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
        '../tools/path_utils.h',
        '../tools/path_utils.cpp',
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
  ],
}
