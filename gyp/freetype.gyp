{
  'targets': [
    {
      'target_name': 'freetype',
      'type': 'none',
      'conditions': [
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "chromeos"]', {
          'direct_dependent_settings': {
            'include_dirs' : [
              '/usr/include/freetype2',
            ],
            'link_settings': {
              'libraries': [
                '-lfreetype',
              ],
              'defines': [
                #The font host requires at least FreeType 2.3.0 at runtime.
                'SK_FONTHOST_FREETYPE_RUNTIME_VERSION=0x020300',\
                'SK_CAN_USE_DLOPEN=1',
              ],
            }
          },
        }],
        [ 'skia_os in ["android", "nacl"]', {
          'dependencies': [
            'freetype_static'
          ],
          'export_dependent_settings': [
            'freetype_static'
          ],
          'direct_dependent_settings': {
            'defines': [
              # Both Android and NaCl provide at least FreeType 2.4.0
              'SK_FONTHOST_FREETYPE_RUNTIME_VERSION=0x020400',
              'SK_CAN_USE_DLOPEN=0',
            ],
          },
        }],
      ],
    },
    {
      'target_name': 'freetype_static',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        # we are dependent upon PNG for color emoji glyphs
        'images.gyp:images'
      ],
      'includes': [
        # common freetype sources needed for both the base Skia build and the
        # libpoppler build for testing only
        'freetype.gypi',
      ],
      'include_dirs': [
        '../third_party/freetype/include_overrides',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/freetype/include_overrides',
        ],
      },
      'conditions': [
        [ 'skia_os == "android"', {
          # These flags are used by the Android OS.  They are probably overkill
          # for Skia, but we add them for consistency.
          'cflags': [
            '-W',
            '-Wall',
            '-fPIC',
            '-DPIC',
            '-DDARWIN_NO_CARBON',
            '-DFT2_BUILD_LIBRARY',
            '-O2',
          ],
        }],
      ],
    },
    {
      'target_name': 'freetype_poppler',
      'type': 'static_library',
      'standalone_static_library': 1,
      'includes': [
        'freetype.gypi',
      ],
      'sources': [
        # additional components used by poppler
        '../third_party/externals/freetype/src/base/ftbdf.c',
        '../third_party/externals/freetype/src/base/ftpfr.c',

        '../third_party/externals/freetype/src/bdf/bdf.c',
        '../third_party/externals/freetype/src/cid/type1cid.c',
        '../third_party/externals/freetype/src/pcf/pcf.c',
        '../third_party/externals/freetype/src/pfr/pfr.c',
        '../third_party/externals/freetype/src/psaux/psaux.c',
        '../third_party/externals/freetype/src/type1/type1.c',
        '../third_party/externals/freetype/src/type42/type42.c',
        '../third_party/externals/freetype/src/winfonts/winfnt.c',

        '../third_party/externals/freetype/src/gzip/ftgzip.c',
        '../third_party/externals/freetype/src/lzw/ftlzw.c',
      ],
    },
  ],
}
