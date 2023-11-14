/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSurface.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include "include/ports/SkFontMgr_fontconfig.h"

#include <cstdio>

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <name.png>", argv[0]);
        return 1;
    }
    SkFILEWStream output(argv[1]);
    if (!output.isValid()) {
        printf("Cannot open output file %s\n", argv[1]);
        return 1;
    }
    sk_sp<SkSurface> surface = SkSurfaces::Raster(SkImageInfo::MakeN32(100, 50, kOpaque_SkAlphaType));
    SkCanvas* canvas = surface->getCanvas();
    sk_sp<SkFontMgr> mgr = SkFontMgr_New_FontConfig(nullptr);
    sk_sp<SkTypeface> face = mgr->matchFamilyStyle("Roboto", SkFontStyle());
    if (!face) {
      printf("Cannot open typeface\n");
      return 1;
    }
    SkFont font(face, 14);
    SkPaint paint;
    paint.setColor(SK_ColorGREEN);
    canvas->clear(SK_ColorYELLOW);
    canvas->drawString("Hello world!", 10, 25, font, paint);
    SkPixmap pixmap;
    if (surface->peekPixels(&pixmap)) {
        if (!SkPngEncoder::Encode(&output, pixmap, {})) {
          printf("Cannot write output\n");
          return 1;
        }
    } else {
      printf("Cannot readback on surface\n");
      return 1;
    }
    return 0;
}
