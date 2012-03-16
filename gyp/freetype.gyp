{
  'targets': [
    {
      'target_name': 'skfreetype',
      'type': 'static_library',
      'sources': [
        '../third_party/freetype/src/base/ftbbox.c',
        '../third_party/freetype/src/base/ftbitmap.c',
        '../third_party/freetype/src/base/ftglyph.c',
        '../third_party/freetype/src/base/ftlcdfil.c',
        '../third_party/freetype/src/base/ftstroke.c',
        '../third_party/freetype/src/base/ftxf86.c',
        '../third_party/freetype/src/base/ftbase.c',
        '../third_party/freetype/src/base/ftsystem.c',
        '../third_party/freetype/src/base/ftinit.c',
        '../third_party/freetype/src/base/ftgasp.c',
        '../third_party/freetype/src/base/ftfstype.c',
        '../third_party/freetype/src/raster/raster.c',
        '../third_party/freetype/src/sfnt/sfnt.c',
        '../third_party/freetype/src/smooth/smooth.c',
        '../third_party/freetype/src/autofit/autofit.c',
        '../third_party/freetype/src/truetype/truetype.c',
        '../third_party/freetype/src/cff/cff.c',
        '../third_party/freetype/src/psnames/psnames.c',
        '../third_party/freetype/src/pshinter/pshinter.c',

# added for linker
        '../third_party/freetype/src/lzw/ftlzw.c',
        '../third_party/freetype/src/gzip/ftgzip.c',
        '../third_party/freetype/src/cid/type1cid.c',
        '../third_party/freetype/src/bdf/bdf.c',
        '../third_party/freetype/src/psaux/psaux.c',
        '../third_party/freetype/src/pcf/pcf.c',
        '../third_party/freetype/src/pfr/pfr.c',
        '../third_party/freetype/src/type1/type1.c',
        '../third_party/freetype/src/type42/type42.c',
        '../third_party/freetype/src/winfonts/winfnt.c',
      ],
      'include_dirs': [
        '../third_party/freetype/internal',
        '../third_party/freetype/builds',
        '../third_party/freetype/include',
        '../third_party/freetype',
      ],
      'cflags': [
        '-W',
        '-Wall',
        '-fPIC',
        '-DPIC',
        '-DDARWIN_NO_CARBON',
        '-DFT2_BUILD_LIBRARY',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/freetype/include',  # For ft2build.h
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
