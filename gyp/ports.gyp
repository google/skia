# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Port-specific Skia library code.
{
  'targets': [
    {
      'target_name': 'ports',
      'product_name': 'skia_ports',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'core.gyp:*',
        'qcms.gyp:qcms',
      ],
      'export_dependent_settings': [ 'qcms.gyp:qcms', ],
      'include_dirs': [
        '../include/effects',
        '../include/client/android',
        '../include/images',
        '../include/ports',
        '../include/private',
        '../include/utils',
        '../include/utils/win',
        '../src/core',
        '../src/image',
        '../src/lazy',
        '../src/ports',
        '../src/sfnt',
        '../src/utils',
      ],
      'sources': [
        '../src/ports/SkDebug_stdio.cpp',
        '../src/ports/SkDebug_win.cpp',

        '../src/fonts/SkFontMgr_indirect.cpp',
        '../src/fonts/SkRemotableFontMgr.cpp',
        '../src/ports/SkFontHost_win.cpp',
        '../src/ports/SkFontMgr_android_factory.cpp',
        '../src/ports/SkFontMgr_custom_directory_factory.cpp',
        '../src/ports/SkFontMgr_custom_embedded_factory.cpp',
        '../src/ports/SkFontMgr_fontconfig_factory.cpp',
        '../src/ports/SkFontMgr_win_dw.cpp',
        '../src/ports/SkFontMgr_win_dw_factory.cpp',
        '../src/ports/SkFontMgr_win_gdi_factory.cpp',
        '../src/ports/SkRemotableFontMgr_win_dw.cpp',
        '../src/ports/SkScalerContext_win_dw.cpp',
        '../src/ports/SkScalerContext_win_dw.h',
        '../src/ports/SkTypeface_win_dw.cpp',
        '../src/ports/SkTypeface_win_dw.h',

        '../src/ports/SkGlobalInitialization_default.cpp',
        '../src/ports/SkMemory_malloc.cpp',
        '../src/ports/SkOSEnvironment.h',
        '../src/ports/SkOSEnvironment.cpp',
        '../src/ports/SkOSFile_posix.cpp',
        '../src/ports/SkOSFile_stdio.cpp',
        '../src/ports/SkOSFile_win.cpp',
        '../src/ports/SkOSLibrary.h',
        '../src/ports/SkOSLibrary_posix.cpp',
        '../src/ports/SkOSLibrary_win.cpp',
        '../src/ports/SkDiscardableMemory_none.cpp',
        '../src/ports/SkTLS_pthread.cpp',
        '../src/ports/SkTLS_win.cpp',

        '../include/ports/SkFontConfigInterface.h',
        '../include/ports/SkFontMgr.h',
        '../include/ports/SkFontMgr_android.h',
        '../include/ports/SkFontMgr_custom.h',
        '../include/ports/SkFontMgr_fontconfig.h',
        '../include/ports/SkFontMgr_indirect.h',
        '../include/ports/SkRemotableFontMgr.h',
      ],
      'sources/': [
        ['exclude', 'SkFontMgr_.+_factory\\.cpp$'],
      ],
      'conditions': [
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "android"]', {
          'sources': [
            '../src/ports/SkFontHost_FreeType.cpp',
            '../src/ports/SkFontHost_FreeType_common.cpp',
            '../src/ports/SkFontMgr_android.cpp',
            '../src/ports/SkFontMgr_android_parser.cpp',
            '../src/ports/SkFontMgr_custom.cpp',
          ],
          'dependencies': [
            'freetype.gyp:freetype',
          ],
          'conditions': [
            [ 'skia_android_framework', {
              'link_settings': { 'libraries': [ '-lexpat' ] },
            }, {
              'link_settings': { 'libraries': [ '-ldl' ] },
              'dependencies': [ 'expat.gyp:expat' ],
            }],
            [ 'skia_embedded_fonts', {
              'variables': {
                'embedded_font_data_identifier': 'sk_fonts',
                'fonts_to_include': [
                  '../resources/fonts/Funkster.ttf',
                ],
              },
              'sources/': [['include', '../src/ports/SkFontMgr_custom_embedded_factory.cpp']],
              'actions': [{
                'action_name': 'generate_embedded_font_data',
                'inputs': [
                  '../tools/embed_resources.py',
                  '<@(fonts_to_include)',
                ],
                'outputs': [
                  '<(SHARED_INTERMEDIATE_DIR)/ports/fonts/fonts.cpp',
                ],
                'action': ['python', '../tools/embed_resources.py',
                                     '--align', '4',
                                     '--name', '<(embedded_font_data_identifier)',
                                     '--input', '<@(fonts_to_include)',
                                     '--output', '<@(_outputs)',
                ],
                'message': 'Generating <@(_outputs)',
                'process_outputs_as_sources': 1,
              }],
              'defines': [
                'SK_EMBEDDED_FONTS=<(embedded_font_data_identifier)',
              ],
            }, 'skia_no_fontconfig', {
              'sources/': [['include', '../src/ports/SkFontMgr_custom_directory_factory.cpp']],
            }, 'skia_os == "android"', {
              'sources/': [['include', '../src/ports/SkFontMgr_android_factory.cpp']],
            }, {
              'link_settings': {
                'libraries': [
                  '-lfontconfig',
                ],
              },
              'sources': [
                '../src/ports/SkFontMgr_fontconfig.cpp',
                '../src/ports/SkFontConfigInterface_direct.cpp',
                '../src/ports/SkFontConfigInterface_direct_factory.cpp',
              ],
              'sources/': [['include', '../src/ports/SkFontMgr_fontconfig_factory.cpp']],
            }]
          ],
        }],
        [ 'skia_os == "mac"', {
          'include_dirs': [
            '../include/utils/mac',
          ],
          'sources': [
            '../src/ports/SkFontHost_mac.cpp',
            '../src/utils/mac/SkStream_mac.cpp',

            '../src/ports/SkImageGeneratorCG.cpp',
          ],
        }],
        [ 'skia_os == "ios"', {
          'include_dirs': [
            '../include/utils/ios',
            '../include/utils/mac',
          ],
          'sources': [
            '../src/ports/SkFontHost_mac.cpp',
            '../src/utils/mac/SkStream_mac.cpp',

            '../src/ports/SkImageGeneratorCG.cpp',
          ],
        }],
        [ 'skia_os == "win"', {
          'include_dirs': [
            'config/win',
            '../src/utils/win',
          ],
          'sources!': [ # these are used everywhere but windows
            '../src/ports/SkDebug_stdio.cpp',
            '../src/ports/SkOSFile_posix.cpp',
            '../src/ports/SkOSLibrary_posix.cpp',
            '../src/ports/SkTLS_pthread.cpp',
          ],
          'sources': [
            '../src/ports/SkImageGeneratorWIC.cpp',
          ],
          'conditions': [
            #    when we build for win, we only want one of these default files
            [ 'skia_gdi', {
              'sources/': [['include', '../src/ports/SkFontMgr_win_gdi_factory.cpp']],
            }, { # normally default to direct write
              'sources/': [['include', '../src/ports/SkFontMgr_win_dw_factory.cpp']],
            }],
          ],
          'link_settings': {
            'libraries': [
              '-lwindowscodecs.lib',
            ],
          },
        }, { # else !win
          'sources!': [
            '../src/ports/SkDebug_win.cpp',
            '../src/ports/SkFontHost_win.cpp',
            '../src/ports/SkFontMgr_win_dw.cpp',
            '../src/ports/SkOSFile_win.cpp',
            '../src/ports/SkOSLibrary_win.cpp',
            '../src/ports/SkRemotableFontMgr_win_dw.cpp',
            '../src/ports/SkTLS_win.cpp',
            '../src/ports/SkScalerContext_win_dw.cpp',
            '../src/ports/SkScalerContext_win_dw.h',
            '../src/ports/SkTypeface_win_dw.cpp',
            '../src/ports/SkTypeface_win_dw.h',
          ],
        }],
        [ 'skia_os == "android"', {
          'sources!': [
            '../src/ports/SkDebug_stdio.cpp',
          ],
          'sources': [
            '../src/ports/SkDebug_android.cpp',
          ],
        }],
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/ports',
        ],
      },
    },
  ],
}
