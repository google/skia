# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
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
        'bitmap_region_decoder',
        'chrome_fuzz',
        'filter',
        'gpuveto',
        'imgblur',
        'imgconv',
        'imgslice',
        'lua_app',
        'lua_pictures',
        'pinspect',
        'render_pdfs',
        'skdiff',
        'skhello',
        'skpdiff',
        'skpinfo',
        'skpmaker',
        'test_image_decoder',
        'test_public_includes',
        'whitelist_typefaces',
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
      'target_name': 'bitmap_region_decoder',
      'type': 'static_library',
      'sources': [
        '../tools/android/SkBitmapRegionCanvas.cpp',
        '../tools/android/SkBitmapRegionCodec.cpp',
        '../tools/android/SkBitmapRegionDecoder.cpp',
      ],
      'include_dirs': [
        '../include/private',
        '../src/codec',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../tools/android',
        ],
      },
    },
    {
      'target_name': 'chrome_fuzz',
      'type': 'executable',
      'sources': [
        '../tools/chrome_fuzz.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
      'target_name': 'crash_handler',
        'type': 'static_library',
        'sources': [ '../tools/CrashHandler.cpp' ],
        'dependencies': [ 'skia_lib.gyp:skia_lib' ],
        'direct_dependent_settings': {
          'include_dirs': [ '../tools' ],
        },
        'conditions': [
          [ 'skia_is_bot', {
            'defines': [ 'SK_CRASH_HANDLER' ],
          }],
        ],

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
        '../include/private',
        '../src/fonts',
        '../src/core',
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
      'sources': [ '../tools/timer/Timer.cpp' ],
      'direct_dependent_settings': {
        'include_dirs': ['../tools/timer'],
      },
      'dependencies': [ 'skia_lib.gyp:skia_lib' ],
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
        '../include/private',
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
              '-pthread',
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
        '../include/private',
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
        'flags.gyp:flags',
        'pdf.gyp:pdf',
        'skia_lib.gyp:skia_lib',
      ],
      'sources': [
        '../tools/skhello.cpp',
      ],
    },
    {
      'target_name': 'skpinfo',
      'type': 'executable',
      'sources': [
        '../tools/skpinfo.cpp',
      ],
      'include_dirs': [
        '../include/private',
        '../src/core/',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
      'target_name': 'imgblur',
      'type': 'executable',
      'sources': [
        '../tools/imgblur.cpp',
      ],
      'include_dirs': [
        '../include/core',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'flags.gyp:flags_common',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:sk_tool_utils',
      ],
    },
    {
      'target_name': 'imgslice',
      'type': 'executable',
      'sources': [
        '../tools/imgslice.cpp',
      ],
      'include_dirs': [
        '../include/core',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
        'target_name': 'lazy_decode_bitmap',
        'type': 'static_library',
        'sources': [ '../tools/LazyDecodeBitmap.cpp' ],
        'include_dirs': [
            '../include/private',
            '../src/core',
            '../src/lazy',
        ],
        'dependencies': [
            'flags.gyp:flags',
            'skia_lib.gyp:skia_lib'
        ],
    },
    {
      'target_name': 'gpuveto',
      'type': 'executable',
      'sources': [
        '../tools/gpuveto.cpp',
      ],
      'include_dirs': [
        '../include/private',
        '../src/core/',
        '../src/images',
      ],
      'dependencies': [
        'lazy_decode_bitmap',
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
        '../include/private',
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
        '../include/private',
        # Lua exposes GrReduceClip which in turn requires src/core for SkTLList
        '../src/gpu/',
        '../src/core/',
      ],
      'dependencies': [
        'lazy_decode_bitmap',
        'effects.gyp:effects',
        'flags.gyp:flags',
        'images.gyp:images',
        'lua.gyp:lua',
        'tools.gyp:picture_utils',
        'pdf.gyp:pdf',
        'ports.gyp:ports',
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
      'target_name': 'picture_renderer',
      'type': 'static_library',
      'sources': [
        '../tools/PictureRenderer.h',
        '../tools/PictureRenderer.cpp',
      ],
      'include_dirs': [
        '../include/private',
        '../src/core',
        '../src/images',
        '../src/lazy',
        '../src/pipe/utils/',
        '../src/utils/',
      ],
      'dependencies': [
        'lazy_decode_bitmap',
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
            'export_dependent_settings': [
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
        '../include/private',
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
         'skia_arch_type == "x86_64"',
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
         'skia_arch_type == "x86"',
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
        'lazy_decode_bitmap',
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
      'target_name': 'imgconv',
      'type': 'executable',
      'sources': [
        '../tools/imgconv.cpp',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
      'target_name': 'filter',
      'type': 'executable',
      'include_dirs' : [
        '../include/private',
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
    {
      'target_name': 'whitelist_typefaces',
      'type': 'executable',
      'sources': [
        '../tools/whitelist_typefaces.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
      'target_name': 'test_public_includes',
      'type': 'static_library',
      # Ensure that our public headers don't have unused params so that clients
      # (e.g. Android) that include us can build with these warnings enabled
      'cflags!': [ '-Wno-unused-parameter' ],
      'variables': {
        'includes_to_test': [
          '<(skia_include_path)/animator',
          '<(skia_include_path)/c',
          '<(skia_include_path)/codec',
          '<(skia_include_path)/config',
          '<(skia_include_path)/core',
          '<(skia_include_path)/effects',
          '<(skia_include_path)/gpu',
          '<(skia_include_path)/images',
          '<(skia_include_path)/pathops',
          '<(skia_include_path)/pipe',
          '<(skia_include_path)/ports',
          '<(skia_include_path)/svg/parser',
          '<(skia_include_path)/utils',
          '<(skia_include_path)/views',
          '<(skia_include_path)/xml',
        ],
        'paths_to_ignore': [
          '<(skia_include_path)/gpu/gl/GrGLConfig_chrome.h',
          '<(skia_include_path)/ports/SkFontMgr_fontconfig.h',
          '<(skia_include_path)/ports/SkTypeface_mac.h',
          '<(skia_include_path)/ports/SkTypeface_win.h',
          '<(skia_include_path)/utils/ios',
          '<(skia_include_path)/utils/mac',
          '<(skia_include_path)/utils/win',
          '<(skia_include_path)/utils/SkDebugUtils.h',
          '<(skia_include_path)/utils/SkJSONCPP.h',
          '<(skia_include_path)/views/animated',
          '<(skia_include_path)/views/SkOSWindow_Android.h',
          '<(skia_include_path)/views/SkOSWindow_iOS.h',
          '<(skia_include_path)/views/SkOSWindow_Mac.h',
          '<(skia_include_path)/views/SkOSWindow_SDL.h',
          '<(skia_include_path)/views/SkOSWindow_Unix.h',
          '<(skia_include_path)/views/SkOSWindow_Win.h',
          '<(skia_include_path)/views/SkWindow.h',
        ],
      },
      'include_dirs': [
        '<@(includes_to_test)',
      ],
      'sources': [
        # unused_param_test.cpp is generated by the action below.
        '<(INTERMEDIATE_DIR)/test_public_includes.cpp',
      ],
      'actions': [
        {
          'action_name': 'generate_includes_cpp',
          'inputs': [
            '../tools/generate_includes_cpp.py',
            '<@(includes_to_test)',
            # This causes the gyp generator on mac to fail
            #'<@(paths_to_ignore)',
          ],
          'outputs': [
            '<(INTERMEDIATE_DIR)/test_public_includes.cpp',
          ],
          'action': ['python', '../tools/generate_includes_cpp.py',
                               '--ignore', '<(paths_to_ignore)',
                               '<@(_outputs)', '<@(includes_to_test)'],
        },
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
              '../include/private',
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
              '../include/private',
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
