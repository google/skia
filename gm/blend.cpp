/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

DEF_SIMPLE_GM(blend, canvas, 300, 100) {
    SkPaint p;

    // All three of these blocks should be the same color.
    canvas->save();
        canvas->scale(100,100);

        p.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::MakeXYWH(0,0,1,1), p);
        p.setColor(0xFC008000);
        canvas->drawRect(SkRect::MakeXYWH(0,0,1,1), p);

        p.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::MakeXYWH(1,0,1,1), p);
        canvas->saveLayer(nullptr, nullptr);
            p.setColor(0xFC008000);
            canvas->drawRect(SkRect::MakeXYWH(1,0,1,1), p);
        canvas->restore();

        p.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::MakeXYWH(2,0,1,1), p);
        canvas->saveLayerAlpha(nullptr, 0xFC);
            p.setColor(sk_tool_utils::color_to_565(0xFF008000));
            canvas->drawRect(SkRect::MakeXYWH(2,0,1,1), p);
        canvas->restore();
    canvas->restore();

    // Print out the colors in each block (if we're looking at 8888 raster).
    if (canvas->imageInfo().colorType() == kN32_SkColorType) {
        if (const SkPMColor* px = (const SkPMColor*)canvas->peekPixels(nullptr, nullptr)) {
            p.setColor(SK_ColorWHITE);
            for (int i = 0; i < 3; i++) {
                SkPMColor c = px[i * 100];
                SkString text = SkStringPrintf("0x%08x", c);
                canvas->drawText(text.c_str(), text.size(), i * 100.0f + 20.0f, 50.0f, p);
            }
        }
    }
}
