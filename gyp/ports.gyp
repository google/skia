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
      ],
      'include_dirs': [
        '../include/effects',
        '../include/images',
        '../include/ports',
        '../include/utils',
        '../include/utils/win',
        '../include/xml',
        '../src/core',
        '../src/lazy',
        '../src/ports',
        '../src/sfnt',
        '../src/utils',
      ],
      'sources': [
        '../src/ports/SkDebug_nacl.cpp',
        '../src/ports/SkDebug_stdio.cpp',
        '../src/ports/SkDebug_win.cpp',

        '../src/ports/SkFontHost_win.cpp',
        '../src/ports/SkFontHost_win_dw.cpp',
        '../src/ports/SkFontMgr_default_gdi.cpp',
        '../src/ports/SkFontMgr_default_dw.cpp',

        '../src/ports/SkGlobalInitialization_default.cpp',
        '../src/ports/SkMemory_malloc.cpp',
        '../src/ports/SkOSFile_posix.cpp',
        '../src/ports/SkOSFile_stdio.cpp',
        '../src/ports/SkOSFile_win.cpp',
        '../src/ports/SkDiscardableMemory_none.cpp',
        '../src/ports/SkPurgeableMemoryBlock_none.cpp',
       #'../src/ports/SkThread_none.cpp',
        '../src/ports/SkThread_pthread.cpp',
        '../src/ports/SkThread_win.cpp',
        '../src/ports/SkTime_Unix.cpp',
        '../src/ports/SkTime_win.cpp',
       #'../src/ports/SkTLS_none.cpp',
        '../src/ports/SkTLS_pthread.cpp',
        '../src/ports/SkTLS_win.cpp',
        '../src/ports/SkXMLParser_empty.cpp',
      ],
      'conditions': [
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "chromeos", "nacl", "android"]', {
          'sources': [
            '../src/ports/SkFontHost_FreeType.cpp',
            '../src/ports/SkFontHost_FreeType_common.cpp',
          ],
          'dependencies': [
            'freetype.gyp:freetype',
          ],
        }],
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "chromeos"]', {
          'link_settings': {
            'libraries': [
              '-lfontconfig',
              '-ldl',
            ],
          },
          'sources': [
            '../src/fonts/SkFontMgr_fontconfig.cpp',
            '../src/ports/SkFontHost_fontconfig.cpp',
            '../src/ports/SkFontConfigInterface_direct.cpp',
          ],
        }],
        [ 'skia_os == "nacl"', {
          'sources': [
            '../src/ports/SkFontHost_linux.cpp',
          ],
          'sources!': [
            '../src/ports/SkDebug_stdio.cpp',
          ],
        }, {
          'sources!': [
            '../src/ports/SkDebug_nacl.cpp',
          ],
        }],
        [ 'skia_os == "mac"', {
          'include_dirs': [
            '../include/utils/mac',
          ],
          'sources': [
            '../src/ports/SkFontHost_mac.cpp',
            '../src/ports/SkPurgeableMemoryBlock_mac.cpp',
            '../src/utils/mac/SkStream_mac.cpp',
          ],
          'sources!': [
            '../src/ports/SkPurgeableMemoryBlock_none.cpp',
            '../src/ports/SkFontHost_tables.cpp',
          ],
        }],
        [ 'skia_os == "ios"', {
          'include_dirs': [
            '../include/utils/ios',
            '../include/utils/mac',
          ],
          'sources': [
            '../src/ports/SkFontHost_mac.cpp',
            '../src/ports/SkPurgeableMemoryBlock_mac.cpp',
            '../src/utils/mac/SkStream_mac.cpp',
          ],
          'sources!': [
            '../src/ports/SkPurgeableMemoryBlock_none.cpp',
            '../src/ports/SkFontHost_tables.cpp',
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
            '../src/ports/SkThread_pthread.cpp',
            '../src/ports/SkTime_Unix.cpp',
            '../src/ports/SkTLS_pthread.cpp',
          ],
          'conditions': [
            #    when we build for win, we only want one of these default files
            [ 'skia_directwrite', {
              'sources!': [
                '../src/ports/SkFontMgr_default_gdi.cpp',
              ],
            }, { # else gdi
              'sources!': [
                '../src/ports/SkFontMgr_default_dw.cpp',
              ],
            }],
          ],
        }, { # else !win
          'sources!': [
            '../src/ports/SkDebug_win.cpp',
            '../src/ports/SkFontHost_win.cpp',
            '../src/ports/SkFontHost_win_dw.cpp',
            '../src/ports/SkFontMgr_default_gdi.cpp',
            '../src/ports/SkFontMgr_default_dw.cpp',
            '../src/ports/SkOSFile_win.cpp',
            '../src/ports/SkThread_win.cpp',
            '../src/ports/SkTime_win.cpp',
            '../src/ports/SkTLS_win.cpp',
          ],
        }],
        [ 'skia_os == "android"', {
          'sources!': [
            '../src/ports/SkDebug_stdio.cpp',
            '../src/ports/SkDiscardableMemory_none.cpp',
            '../src/ports/SkPurgeableMemoryBlock_none.cpp',
          ],
          'sources': [
            '../src/ports/SkDebug_android.cpp',
            '../src/ports/SkDiscardableMemory_ashmem.cpp',
            '../src/ports/SkFontConfigInterface_android.cpp',
            '../src/ports/SkFontConfigParser_android.cpp',
            '../src/ports/SkFontHost_fontconfig.cpp',
            '../src/ports/SkPurgeableMemoryBlock_android.cpp',
          ],
          'dependencies': [
             'android_deps.gyp:expat',
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
