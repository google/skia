
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

#define GR_FONT_ATLAS_NUM_PLOTS_X    5
#define GR_FONT_ATLAS_NUM_PLOTS_Y    6

#define GR_FONT_ATLAS_TEXTURE_WIDTH  (GR_FONT_ATLAS_PLOT_WIDTH*GR_FONT_ATLAS_NUM_PLOTS_X)
SK_COMPILE_ASSERT(GR_FONT_ATLAS_TEXTURE_WIDTH == 1640, font_atlas_unexpected_size);
#define GR_FONT_ATLAS_TEXTURE_HEIGHT (GR_FONT_ATLAS_PLOT_HEIGHT*GR_FONT_ATLAS_NUM_PLOTS_Y)
SK_COMPILE_ASSERT(GR_FONT_ATLAS_TEXTURE_HEIGHT == 1968, font_atlas_unexpected_size);

// one over width and height
#define GR_FONT_ATLAS_RECIP_WIDTH    "0.00060975609"
#define GR_FONT_ATLAS_RECIP_HEIGHT   "0.00050813008"

// 1/(3*width)
#define GR_FONT_ATLAS_LCD_DELTA      "0.00020325203"

#endif
