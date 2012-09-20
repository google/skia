# Port-specific Skia library code.
{
  'targets': [
    {
      'target_name': 'ports',
      'type': 'static_library',
      'dependencies': [
        'core.gyp:core',
        'sfnt.gyp:sfnt',
        'utils.gyp:utils',
      ],
      'include_dirs': [
        '../include/images',
        '../include/effects',
        '../include/ports',
        '../include/xml',
        '../src/core',
        '../src/utils',
      ],
      'sources': [
        '../src/ports/SkDebug_stdio.cpp',
        '../src/ports/SkDebug_win.cpp',
        '../src/ports/SkFontDescriptor.h',
        '../src/ports/SkFontDescriptor.cpp',
        '../src/ports/SkFontHost_sandbox_none.cpp',
        '../src/ports/SkFontHost_win.cpp',
        '../src/ports/SkFontHost_win_dw.cpp',
        '../src/ports/SkGlobalInitialization_default.cpp',
        '../src/ports/SkThread_win.cpp',

        '../src/ports/SkFontHost_tables.cpp',
        '../src/ports/SkMemory_malloc.cpp',
        '../src/ports/SkOSFile_stdio.cpp',
        '../src/ports/SkTime_Unix.cpp',
        '../src/ports/SkTime_win.cpp',
        '../src/ports/SkXMLParser_empty.cpp',
      ],
      'conditions': [
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris"]', {
          'conditions': [
            [ 'skia_nacl', {
              'defines': [
                'SK_CAN_USE_DLOPEN=0',
              ],
              'sources': [
                '../src/ports/SkFontHost_none.cpp',
              ],
            }, {
              'defines': [
                #The font host requires at least FreeType 2.3.0 at runtime.
                'SK_FONTHOST_FREETYPE_RUNTIME_VERSION=0x020300',
                'SK_CAN_USE_DLOPEN=1',
              ],
              'sources': [
                '../src/ports/SkFontHost_FreeType.cpp',
                '../src/ports/SkFontHost_FreeType_common.cpp',
                '../src/ports/SkFontHost_linux.cpp',
              ],
              'link_settings': {
                'libraries': [
                  '-lfreetype',
                  '-ldl',
                ],
              },
            }],
          ],
          'sources': [
            '../src/ports/SkThread_pthread.cpp',
          ],
        }],
        [ 'skia_os == "mac"', {
          'include_dirs': [
            '../include/utils/mac',
            '../third_party/freetype/include/**',
          ],
          'sources': [
            '../src/ports/SkFontHost_mac_coretext.cpp',
            '../src/utils/mac/SkStream_mac.cpp',
#            '../src/ports/SkFontHost_FreeType.cpp',
#            '../src/ports/SkFontHost_FreeType_common.cpp',
#            '../src/ports/SkFontHost_freetype_mac.cpp',
            '../src/ports/SkThread_pthread.cpp',
          ],
          'sources!': [
            '../src/ports/SkFontHost_tables.cpp',
          ],
        }],
        [ 'skia_os == "ios"', {
          'include_dirs': [
            '../include/utils/ios',
            '../include/utils/mac',
          ],
          'sources': [
            '../src/ports/SkFontHost_mac_coretext.cpp',
            '../src/utils/mac/SkStream_mac.cpp',
            '../src/ports/SkThread_pthread.cpp',
          ],
          'sources!': [
            '../src/ports/SkFontHost_tables.cpp',
          ],
        }],
        [ 'skia_os == "win"', {
          'include_dirs': [
            'config/win',
            '../src/utils/win',
          ],
          'conditions': [
            [ 'skia_directwrite', {
                'sources!': [
                  '../src/ports/SkFontHost_win.cpp',
                ],
              }, { # else !skia_directwrite
                'sources!': [
                  '../src/ports/SkFontHost_win_dw.cpp',
                ],
              }],
          ],
          'sources!': [ # these are used everywhere but windows
            '../src/ports/SkDebug_stdio.cpp',
            '../src/ports/SkTime_Unix.cpp',
          ],
        }, { # else !win
          'sources!': [
            '../src/ports/SkDebug_win.cpp',
            '../src/ports/SkFontHost_win.cpp',
            '../src/ports/SkFontHost_win_dw.cpp',
            '../src/ports/SkThread_win.cpp',
            '../src/ports/SkTime_win.cpp',
          ],
        }],
        [ 'skia_os == "android"', {
          'defines': [
            #Android provides at least FreeType 2.4.0 at runtime.
            'SK_FONTHOST_FREETYPE_RUNTIME_VERSION=0x020400',
            #Skia should not use dlopen on Android.
            'SK_CAN_USE_DLOPEN=0',
          ],
          'sources!': [
            '../src/ports/SkDebug_stdio.cpp',
          ],
          'sources': [
            '../src/ports/SkDebug_android.cpp',
            '../src/ports/SkThread_pthread.cpp',
            '../src/ports/SkFontHost_android.cpp',
            '../src/ports/SkFontHost_FreeType.cpp',
            '../src/ports/SkFontHost_FreeType_common.cpp',
            '../src/ports/FontHostConfiguration_android.cpp',
            #TODO: include the ports/SkImageRef_ashmem.cpp for non-NDK builds
          ],
          'dependencies': [
             'android_deps.gyp:ft2',
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

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
