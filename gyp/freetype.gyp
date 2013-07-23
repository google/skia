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
      'sources': [
        # base components (required)
        '../third_party/externals/freetype/src/base/ftsystem.c',
        '../third_party/externals/freetype/src/base/ftinit.c',
        '../third_party/externals/freetype/src/base/ftdebug.c',
        '../third_party/externals/freetype/src/base/ftbase.c',

        '../third_party/externals/freetype/src/base/ftbbox.c',       # recommended, see <freetype/ftbbox.h>
        '../third_party/externals/freetype/src/base/ftglyph.c',      # recommended, see <freetype/ftglyph.h>

        '../third_party/externals/freetype/src/base/ftbitmap.c',     # optional, see <freetype/ftbitmap.h>
        '../third_party/externals/freetype/src/base/ftfstype.c',     # optional
        '../third_party/externals/freetype/src/base/ftgasp.c',       # optional, see <freetype/ftgasp.h>
        '../third_party/externals/freetype/src/base/ftlcdfil.c',     # optional, see <freetype/ftlcdfil.h>
        '../third_party/externals/freetype/src/base/ftmm.c',         # optional, see <freetype/ftmm.h>
        '../third_party/externals/freetype/src/base/ftpatent.c',     # optional
        '../third_party/externals/freetype/src/base/ftstroke.c',     # optional, see <freetype/ftstroke.h>
        '../third_party/externals/freetype/src/base/ftsynth.c',      # optional, see <freetype/ftsynth.h>
        '../third_party/externals/freetype/src/base/fttype1.c',      # optional, see <freetype/t1tables.h>
        '../third_party/externals/freetype/src/base/ftwinfnt.c',     # optional, see <freetype/ftwinfnt.h>
        '../third_party/externals/freetype/src/base/ftxf86.c',       # optional, see <freetype/ftxf86.h>

        # font drivers (optional; at least one is needed)
        '../third_party/externals/freetype/src/cff/cff.c',           # CFF/OpenType font driver
        '../third_party/externals/freetype/src/sfnt/sfnt.c',         # SFNT files support (TrueType & OpenType)
        '../third_party/externals/freetype/src/truetype/truetype.c', # TrueType font driver

        # rasterizers (optional; at least one is needed for vector formats)
        '../third_party/externals/freetype/src/raster/raster.c',     # monochrome rasterizer
        '../third_party/externals/freetype/src/smooth/smooth.c',     # anti-aliasing rasterizer

        # auxiliary modules (optional)
        '../third_party/externals/freetype/src/autofit/autofit.c',   # auto hinting module
        '../third_party/externals/freetype/src/pshinter/pshinter.c', # PS hinting module
        '../third_party/externals/freetype/src/psnames/psnames.c',   # PostScript glyph names support
      ],
      'include_dirs': [
        '../third_party/freetype/include_overrides',
        '../third_party/externals/freetype/internal',
        '../third_party/externals/freetype/builds',
        '../third_party/externals/freetype/include',
        '../third_party/externals/freetype',
      ],
      'cflags': [
        '-DFT2_BUILD_LIBRARY',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/freetype/include_overrides',
          '../third_party/externals/freetype/include',
        ],
      },
      'conditions': [
        [ 'skia_warnings_as_errors', {
          'cflags!': [
            '-Werror',
          ],
        }],
        [ 'skia_os == "mac"', {
          'sources': [
            '../third_party/externals/freetype/src/base/ftmac.c',        # only on the Macintosh
          ],
        }],
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
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
