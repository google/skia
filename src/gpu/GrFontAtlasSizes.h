
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFontAtlasSizes_DEFINED
#define GrFontAtlasSizes_DEFINED

#define GR_FONT_ATLAS_TEXTURE_WIDTH    1024
#define GR_FONT_ATLAS_A8_TEXTURE_WIDTH 2048
#define GR_FONT_ATLAS_TEXTURE_HEIGHT   2048

#define GR_FONT_ATLAS_PLOT_WIDTH       256
#define GR_FONT_ATLAS_A8_PLOT_WIDTH    512
#define GR_FONT_ATLAS_PLOT_HEIGHT      256

#define GR_FONT_ATLAS_NUM_PLOTS_X     (GR_FONT_ATLAS_TEXTURE_WIDTH / GR_FONT_ATLAS_PLOT_WIDTH)
#define GR_FONT_ATLAS_A8_NUM_PLOTS_X  (GR_FONT_ATLAS_A8_TEXTURE_WIDTH / GR_FONT_ATLAS_A8_PLOT_WIDTH)
#define GR_FONT_ATLAS_NUM_PLOTS_Y     (GR_FONT_ATLAS_TEXTURE_HEIGHT / GR_FONT_ATLAS_PLOT_HEIGHT)

// one over width and height
#define GR_FONT_ATLAS_RECIP_WIDTH      "0.0009765625"
#define GR_FONT_ATLAS_A8_RECIP_WIDTH   "0.00048828125"
#define GR_FONT_ATLAS_RECIP_HEIGHT     "0.00048828125"

// 1/(3*width)
// only used for distance fields, which are A8
#define GR_FONT_ATLAS_LCD_DELTA        "0.000162760417"

#endif
