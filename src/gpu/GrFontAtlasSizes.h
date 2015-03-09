
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFontAtlasSizes_DEFINED
#define GrFontAtlasSizes_DEFINED

#include "SkDistanceFieldGen.h"

#define GR_SDF_MAX_SIZE              156

// allows us to fit four of the largest distance field glyphs
#define GR_FONT_ATLAS_PLOT_WIDTH     (2*(GR_SDF_MAX_SIZE+2*SK_DistanceFieldPad))
#define GR_FONT_ATLAS_PLOT_HEIGHT    (2*(GR_SDF_MAX_SIZE+2*SK_DistanceFieldPad))

#define GR_FONT_ATLAS_NUM_PLOTS_X    4
#define GR_FONT_ATLAS_NUM_PLOTS_Y    8

#define GR_FONT_ATLAS_TEXTURE_WIDTH  (GR_FONT_ATLAS_PLOT_WIDTH*GR_FONT_ATLAS_NUM_PLOTS_X)
SK_COMPILE_ASSERT(GR_FONT_ATLAS_TEXTURE_WIDTH == 1312, font_atlas_unexpected_size);
#define GR_FONT_ATLAS_TEXTURE_HEIGHT (GR_FONT_ATLAS_PLOT_HEIGHT*GR_FONT_ATLAS_NUM_PLOTS_Y)
SK_COMPILE_ASSERT(GR_FONT_ATLAS_TEXTURE_HEIGHT == 2624, font_atlas_unexpected_size);

// one over width and height
#define GR_FONT_ATLAS_RECIP_WIDTH    "0.00076219512"
#define GR_FONT_ATLAS_RECIP_HEIGHT   "0.00038109756"

// 1/(3*width)
#define GR_FONT_ATLAS_LCD_DELTA      "0.00025406504"

#endif
