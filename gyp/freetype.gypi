# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
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
    '../third_party/externals/freetype/internal',
    '../third_party/externals/freetype/builds',
    '../third_party/externals/freetype/include',
    '../third_party/externals/freetype',
  ],
  'defines': [
    'FT2_BUILD_LIBRARY',
  ],
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
  ],
  'direct_dependent_settings': {
    'include_dirs': [
      '../third_party/externals/freetype/include',
    ],
  },
}
