# GYP for fontconfig (
#
# This has been tested on Windows and Mac.
# This library is native to Linux, so build from source is not necessary.
#
# Additional files for building under Windows are provided here: (LGPL)
# http://comments.gmane.org/gmane.comp.fonts.fontconfig/4438

{
  'variables': {
    'skia_warnings_as_errors': 0,
  },
  'targets': [
    {
      'target_name': 'fontconfig',
      'type': 'none',
      'dependencies': [
        'libfontconfig',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/externals/fontconfig',
        ],
      },
      'conditions': [
        ['skia_os == "mac"', {
          'dependencies': [
            'fontconfig_setup',
          ],
        }],
      ],
    },
    {
      'target_name': 'libfontconfig',
      'type': 'static_library',
      'dependencies': [
        'freetype.gyp:freetype_poppler',
      ],
      'include_dirs' : [
        '../third_party/externals/fontconfig',
        '../third_party/fontconfig/config',
        '../third_party/fontconfig/config/src',
      ],
      'sources': [
        '../third_party/externals/fontconfig/src/fcatomic.c',
        '../third_party/externals/fontconfig/src/fcblanks.c',
        '../third_party/externals/fontconfig/src/fccache.c',
        '../third_party/externals/fontconfig/src/fccfg.c',
        '../third_party/externals/fontconfig/src/fccharset.c',
        '../third_party/externals/fontconfig/src/fccompat.c',
        '../third_party/externals/fontconfig/src/fcdbg.c',
        '../third_party/externals/fontconfig/src/fcdefault.c',
        '../third_party/externals/fontconfig/src/fcdir.c',
        '../third_party/externals/fontconfig/src/fcfreetype.c',
        '../third_party/externals/fontconfig/src/fcfs.c',
        '../third_party/externals/fontconfig/src/fchash.c',
        '../third_party/externals/fontconfig/src/fcinit.c',
        '../third_party/externals/fontconfig/src/fclang.c',
        '../third_party/externals/fontconfig/src/fclist.c',
        '../third_party/externals/fontconfig/src/fcmatch.c',
        '../third_party/externals/fontconfig/src/fcmatrix.c',
        '../third_party/externals/fontconfig/src/fcname.c',
        '../third_party/externals/fontconfig/src/fcobjs.c',
        '../third_party/externals/fontconfig/src/fcpat.c',
        '../third_party/externals/fontconfig/src/fcserialize.c',
        '../third_party/externals/fontconfig/src/fcstat.c',
        '../third_party/externals/fontconfig/src/fcstr.c',
        '../third_party/externals/fontconfig/src/fcxml.c',
        '../third_party/externals/fontconfig/src/ftglue.c',
      ],
      'defines': [
        'HAVE_CONFIG_H',
      ],
      'cflags': [
        '-fPIC',
      ],

      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/externals/fontconfig',
        ],
      },

      'conditions': [
        ['skia_os == "mac"', {
          'include_dirs': [
            '<(SHARED_INTERMEDIATE_DIR)/fontconfig_config',
            '../third_party/fontconfig/config/mac',
            '../third_party/fontconfig/config/mac/src',
          ],
          'dependencies': [
            'fontconfig_config_template',
          ],
          'libraries': [
            '$(SDKROOT)/usr/lib/libexpat.dylib',
          ],
          'xcode_settings': {
            'DYLIB_INSTALL_NAME_BASE': '@executable_path',
          },
        }],
        ['skia_os == "win"', {
          'include_dirs': [
            '../third_party/fontconfig/config/windows',
            '../third_party/fontconfig/config/windows/src',
          ],
          'sources!': [
            '../third_party/externals/fontconfig/src/fccompat.c',
            '../third_party/externals/fontconfig/src/fcxml.c',
          ],
          'defines': [
            # inline is not recognized  in C and has to be __inline
            'inline=__inline',
          ],
        }],
      ],
    },
  ],
  'conditions': [
    ['skia_os == "mac"', {
      'variables': {
        'cachedir': '<(PRODUCT_DIR)/fontconfig/cache',
        'configdir': '<(PRODUCT_DIR)/fontconfig/etc',
      },
      'targets': [
        {
          'target_name': 'fontconfig_config_template',
          'type': 'none',
          'actions': [{
            'action_name': 'fontconfig_config_template',
            'message': 'Generating fontconfig header',
            'inputs': [
              '../third_party/fontconfig/process-template.py',
              '../third_party/fontconfig/config/mac/config.h.template',
            ],
            'outputs': [
              '<(SHARED_INTERMEDIATE_DIR)/fontconfig_config/config.h',
            ],
            'action': [
              'python', '../third_party/fontconfig/process-template.py',
              '-i', '../third_party/fontconfig/config/mac/config.h.template',
              '-o', '<@(_outputs)',
              '-p', '@FC_CACHEDIR@', '<(cachedir)',
              '-p', '@FONTCONFIG_PATH@', '<(configdir)',
            ],
          }],
        }, {
          'target_name': 'fontconfig_setup',
          'type': 'none',
          'dependencies': [
            'fc-cache',
          ],
          'actions': [{
            'action_name': 'fontconfig_config_file',
            'message': 'Generating font.conf',
            'inputs': [
              '../third_party/fontconfig/process-template.py',
              '../third_party/externals/fontconfig/fonts.conf.in',
            ],
            'outputs': [ '<(configdir)/fonts.conf', ],
            'action': [
              'python', '../third_party/fontconfig/process-template.py',
              '-i', '../third_party/externals/fontconfig/fonts.conf.in',
              '-o', '<(configdir)/fonts.conf',
              '-p', '@FC_CACHEDIR@', '<(cachedir)',
              '-k', '@FC_DEFAULT_FONTS@', '/System/Library/Fonts',
              '-k', '@FC_FONTPATH@', '<dir>/Library/Fonts</dir>',
              '-p', '@CONFIGDIR@', '<(configdir)/conf.d',
              '-k', '@PACKAGE@', 'fontconfig',
              '-k', '@VERSION@', '2.11.0',
            ],
          }, {
            'action_name': 'populate_fontconfig_cache',
            'message':
              'Generating fontconfig cache (this will take a few minutes).',
            'inputs': [
              '<(PRODUCT_DIR)/fc-cache',
              '<(configdir)/fonts.conf',
            ],
            'outputs': [ '<(cachedir)/CACHEDIR.TAG', ],
            'action': [ '<(PRODUCT_DIR)/fc-cache', '-s', '-f', '-v', ],
          }],
        },
        {
          'target_name': 'fc-cache',
          'type': 'executable',
          'dependencies': [
            'libfontconfig',
          ],
          'libraries':[
            '$(SDKROOT)/usr/lib/libexpat.dylib',
          ],
          'sources': [
            '../third_party/externals/fontconfig/fc-cache/fc-cache.c',
          ],
        },
      ],
    }],
  ],
}
