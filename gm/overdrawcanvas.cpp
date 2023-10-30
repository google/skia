/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkOverdrawCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkOverdrawColorFilter.h"
#include "tools/fonts/FontToolUtils.h"

#define WIDTH 500
#define HEIGHT 500

static const SkColor kOverdrawColors[6] = {
    0x00000000, 0x5fff0000, 0x2f0000ff, 0x2f00ff00, 0x3fff0000, 0x7fff0000,
};


DEF_SIMPLE_GM_BG(overdraw_canvas,       canvas, WIDTH, HEIGHT, SK_ColorWHITE) {
    // Set up the overdraw canvas.
    SkImageInfo offscreenInfo = SkImageInfo::MakeA8(WIDTH, HEIGHT);
    sk_sp<SkSurface> offscreen = SkSurfaces::Raster(offscreenInfo);
    SkFont font = ToolUtils::DefaultPortableFont();
    auto c = offscreen->getCanvas();

    SkOverdrawCanvas overdrawCanvas(c);

    overdrawCanvas.drawRect(SkRect::MakeLTRB(10, 10, 200, 200), SkPaint());
    overdrawCanvas.drawRect(SkRect::MakeLTRB(20, 20, 190, 190), SkPaint());
    overdrawCanvas.drawRect(SkRect::MakeLTRB(30, 30, 180, 180), SkPaint());
    overdrawCanvas.drawRect(SkRect::MakeLTRB(40, 40, 170, 170), SkPaint());
    overdrawCanvas.drawRect(SkRect::MakeLTRB(50, 50, 160, 160), SkPaint());
    overdrawCanvas.drawRect(SkRect::MakeLTRB(60, 60, 150, 150), SkPaint());

    char text[] = "Ae_p";
    overdrawCanvas.drawSimpleText(text, 4, SkTextEncoding::kUTF8, 300, 300, font, SkPaint());

    sk_sp<SkImage> counts = offscreen->makeImageSnapshot();

    // Draw overdraw colors to the canvas.  The color filter will convert counts to colors.
    SkPaint paint;
    paint.setColorFilter(SkOverdrawColorFilter::MakeWithSkColors(kOverdrawColors));
    canvas->drawImage(counts.get(), 0.0f, 0.0f, SkSamplingOptions(), &paint);
    canvas->drawString("This is some text:", 180, 300, font, SkPaint());
}

static sk_sp<SkImage> overdraw_text_grid(bool useCTM) {
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeA8(256, 512));
    auto canvas = SkOverdrawCanvas(surface->getCanvas());

    SkPaint paint;
    paint.setColor(SK_ColorWHITE);

    SkFont font = ToolUtils::DefaultPortableFont();

    for (int n = 1; n <= 20; n++) {
        const float x = 10.0f;
        const float y = n * 20.0f;

        for (int i = 0; i < n * 10; i++) {
            const char* text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            if (useCTM) {
                canvas.save();
                canvas.translate(x, y);
                canvas.drawString(text, 0, 0, font, paint);
                canvas.restore();
            } else {
                canvas.drawString(text, x, y, font, paint);
            }
        }
    }
    return surface->makeImageSnapshot();
}

// This GM tests the underlying problem from skbug.com/13732. Text drawn through an overdraw
// canvas would have the CTM applied twice. If everything is working, both images generated should
// look identical. When the bug was present, the second image would have the lines "double spaced",
// because the translations were applied twice.
DEF_SIMPLE_GM_BG(overdraw_text_xform, canvas, 512, 512, SK_ColorBLACK) {
    SkPaint imgPaint;
    imgPaint.setColor(SK_ColorWHITE);
    canvas->drawImage(overdraw_text_grid(false),   0, 0, SkFilterMode::kNearest, &imgPaint);
    canvas->drawImage(overdraw_text_grid( true), 256, 0, SkFilterMode::kNearest, &imgPaint);
}
