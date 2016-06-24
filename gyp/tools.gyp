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
        'chrome_fuzz',
        'dump_record',
        'get_images_from_skps',
        'gpuveto',
        'imgblur',
        'imgslice',
        'lua_app',
        'lua_pictures',
        'pinspect',
        'skdiff',
        'skhello',
        'skpinfo',
        'skpmaker',
        'test_public_includes',
        'using_skia_and_harfbuzz',
        'visualize_color_gamut',
        'whitelist_typefaces',
      ],
      'conditions': [
        ['skia_mesa and skia_os in ["linux", "mac"]',
          { 'dependencies': [ 'fiddle_build_test' ] }
        ],
        ['skia_shared_lib',
          {
            'dependencies': [
              'sklua', # This can only be built if skia is built as a shared library
            ],
          },
        ],
        [ 'skia_os == "android"',
          {
            'dependencies': [
               # Build this by default to catch compile errors more quickly, since
               # the only other time this code is exercised in the android framework
              'android_utils',
            ],
          },
        ],
      ],
    },
    {
      'target_name': 'android_utils',
      'type': 'static_library',
      'dependencies': [
        'core.gyp:core',
      ],
      'sources': [
        '../tools/android/SkAndroidSDKCanvas.h',
        '../tools/android/SkAndroidSDKCanvas.cpp',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../tools/android',
        ],
      },
    },
    {
        'target_name': 'dump_record',
        'type': 'executable',
        'sources': [
            '../tools/dump_record.cpp',
            '../tools/DumpRecord.cpp',
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
        '../tools/random_parse_path.cpp',
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
      'target_name': 'get_images_from_skps',
      'type': 'executable',
      'sources': [
        '../tools/get_images_from_skps.cpp',
      ],
      'include_dirs': [
        '../src/core',
        '../include/private',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'jsoncpp.gyp:jsoncpp',
        'skia_lib.gyp:skia_lib',
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
      'target_name': 'picture_utils',
      'type': 'static_library',
      'sources': [
        '../tools/picture_utils.cpp',
        '../tools/picture_utils.h',
      ],
      'include_dirs': [
          '../src/core/',
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
      'target_name': 'url_data_manager',
      'type': 'static_library',
      'sources': [
        '../tools/UrlDataManager.h',
        '../tools/UrlDataManager.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
      'include_dirs': [
         '../include/private',
         '../src/core',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/private',
          '../tools',
        ],
      },
    },
    {
      'target_name': 'using_skia_and_harfbuzz',
      'type': 'executable',
      'sources': [ '../tools/using_skia_and_harfbuzz.cpp' ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'pdf.gyp:pdf',
        'harfbuzz.gyp:harfbuzz',
      ],
      'cflags': [ '-w', ],
      'msvs_settings': { 'VCCLCompilerTool': { 'WarningLevel': '0', }, },
      'xcode_settings': { 'WARNING_CFLAGS': [ '-w', ], },
    },
    {
      'target_name': 'visualize_color_gamut',
      'type': 'executable',
      'sources': [
        '../tools/visualize_color_gamut.cpp',
      ],
      'include_dirs': [
        '../src/core',
        '../include/private',
        '../tools',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'resources',
        'skia_lib.gyp:skia_lib',
      ],
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
      'target_name': 'thermal_manager',
      'type': 'static_library',
      'sources': [
        '../tools/ThermalManager.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
      'direct_dependent_settings': {
        'include_dirs': [ '../tools', ],
      },
    },
    {
      'target_name': 'test_public_includes',
      'type': 'static_library',
      # Ensure that our public headers don't have unused params so that clients
      # (e.g. Android) that include us can build with these warnings enabled
      'cflags!': [ '-Wno-unused-parameter' ],
      'variables': {
        'includes_to_test': [
          '<(skia_include_path)/android',
          '<(skia_include_path)/animator',
          '<(skia_include_path)/c',
          '<(skia_include_path)/codec',
          '<(skia_include_path)/config',
          '<(skia_include_path)/core',
          '<(skia_include_path)/effects',
          '<(skia_include_path)/gpu',
          '<(skia_include_path)/images',
          '<(skia_include_path)/pathops',
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
          '<(skia_include_path)/views/SkOSWindow_Android.h',
          '<(skia_include_path)/views/SkOSWindow_iOS.h',
          '<(skia_include_path)/views/SkOSWindow_Mac.h',
          '<(skia_include_path)/views/SkOSWindow_SDL.h',
          '<(skia_include_path)/views/SkOSWindow_Unix.h',
          '<(skia_include_path)/views/SkOSWindow_Win.h',
          '<(skia_include_path)/views/SkWindow.h',
          '<(skia_include_path)/gpu/vk',
        ],
        'output_file' : [
          '<(INTERMEDIATE_DIR)/test_public_includes.cpp',
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
          ],
          'outputs': [
            '<@(output_file)',
            # Force the script to always run so that we pick up when files have
            # been deleted.
            'filename_that_does_not_exists_but_forces_rebuild.txt',
          ],
          'action': ['python', '../tools/generate_includes_cpp.py',
                               '--ignore', '<(paths_to_ignore)',
                               '<@(output_file)', '<@(includes_to_test)'],
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
    ['skia_mesa and skia_os in ["linux", "mac"]',
      {
        'targets': [
          {
            'target_name': 'fiddle_build_test',
            'type': 'executable',
            'sources': [
              '../tools/fiddle/draw.cpp',
              '../tools/fiddle/fiddle_main.cpp',
              '../tools/fiddle/fiddle_main.h',
            ],
            'dependencies': [
              'skia_lib.gyp:skia_lib',
              'pdf.gyp:pdf',
              'gputest.gyp:osmesa',
            ],
            'defines': [ 'FIDDLE_BUILD_TEST' ],
          },
        ],
      },
    ],
  ],
}
