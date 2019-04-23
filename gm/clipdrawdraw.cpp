/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkRegion.h"

// This GM exercises the use case found in crbug.com/423834.
// The following pattern:
//    save();
//    clipRect(rect, noAA);
//    drawRect(bigRect, noAA);
//    restore();
//
//    drawRect(rect, noAA);
// can leave 1 pixel wide remnants of the first rect.
static void Draw(SkCanvas* canvas, const SkRect& rect) {
        SkPaint p;
        p.setAntiAlias(false);

        const SkRect bigRect = SkRect::MakeWH(600, 600);

        canvas->save();
            // draw a black rect through the clip
            canvas->save();
                canvas->clipRect(rect);
                canvas->drawRect(bigRect, p);
            canvas->restore();

            // now draw the white rect on top
            p.setColor(SK_ColorWHITE);
            canvas->drawRect(rect, p);
        canvas->restore();
}

DEF_SIMPLE_GM_BG(clipdrawdraw, canvas, 512, 512, 0xFFCCCCCC) {
        // Vertical remnant
        const SkRect rect1 = SkRect::MakeLTRB(136.5f, 137.5f, 338.5f, 293.5f);

        // Horizontal remnant
        // 179.488 rounds the right way (i.e., 179), 179.499 rounds the wrong way (i.e., 180)
        const SkRect rect2 = SkRect::MakeLTRB(207.5f, 179.499f, 530.5f, 429.5f);

        Draw(canvas, rect1);
        Draw(canvas, rect2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

DEF_SIMPLE_GM(clip_region, canvas, 256, 256) {
    SkRegion rgn({ 10, 10, 100, 100 });

    canvas->save();
    canvas->clipRegion(rgn);
    canvas->drawColor(SK_ColorRED);
    canvas->restore();

    SkRect bounds = { 30, 30, 80, 80 };
    canvas->saveLayer(&bounds, nullptr);
    canvas->clipRegion(rgn);
    canvas->drawColor(SK_ColorBLUE);
    canvas->restore();
}
