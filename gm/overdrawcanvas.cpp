/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOverdrawCanvas.h"
#include "SkOverdrawColorFilter.h"
#include "ToolUtils.h"
#include "gm.h"

#define WIDTH 500
#define HEIGHT 500


static const uint32_t kOverdrawColors[6] = {
        0x00000000, 0x5f00005f, 0x2f2f0000, 0x2f002f00, 0x3f00003f, 0x7f00007f,
};


DEF_SIMPLE_GM_BG(overdraw_canvas,       canvas, WIDTH, HEIGHT, SK_ColorWHITE) {
    // Set up the overdraw canvas.
    SkImageInfo offscreenInfo = SkImageInfo::MakeA8(WIDTH, HEIGHT);
    sk_sp<SkSurface> offscreen = SkSurface::MakeRaster(offscreenInfo);
    auto c = offscreen->getCanvas();

    SkOverdrawCanvas overdrawCanvas(c);

    overdrawCanvas.drawRect(SkRect::MakeLTRB(10, 10, 200, 200), SkPaint());
    overdrawCanvas.drawRect(SkRect::MakeLTRB(20, 20, 190, 190), SkPaint());
    overdrawCanvas.drawRect(SkRect::MakeLTRB(30, 30, 180, 180), SkPaint());
    overdrawCanvas.drawRect(SkRect::MakeLTRB(40, 40, 170, 170), SkPaint());
    overdrawCanvas.drawRect(SkRect::MakeLTRB(50, 50, 160, 160), SkPaint());
    overdrawCanvas.drawRect(SkRect::MakeLTRB(60, 60, 150, 150), SkPaint());

    char text[] = "Ae_p";
    overdrawCanvas.drawSimpleText(text, 4, SkTextEncoding::kUTF8, 300, 300, SkFont(), SkPaint());

    sk_sp<SkImage> counts = offscreen->makeImageSnapshot();

    // Draw overdraw colors to the canvas.  The color filter will convert counts to colors.
    SkPaint paint;
    paint.setColorFilter(SkOverdrawColorFilter::Make(kOverdrawColors));
    canvas->drawImage(counts.get(), 0.0f, 0.0f, &paint);
    canvas->drawString("This is some text:", 180, 300, SkFont(), SkPaint());
}

