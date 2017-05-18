/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

DEF_SIMPLE_GM(blend, canvas, 300, 100) {
    SkPaint p;

    // All three of these blocks should be the same color.
    canvas->save();
        canvas->scale(100,100);

        p.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::MakeXYWH(0,0,1,1), p);
        p.setColor(0xFC208000);
        canvas->drawRect(SkRect::MakeXYWH(0,0,1,1), p);

        p.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::MakeXYWH(1,0,1,1), p);
        canvas->saveLayer(nullptr, nullptr);
            p.setColor(0xFC208000);
            canvas->drawRect(SkRect::MakeXYWH(1,0,1,1), p);
        canvas->restore();

        p.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::MakeXYWH(2,0,1,1), p);
        canvas->saveLayerAlpha(nullptr, 0xFC);
            p.setColor(sk_tool_utils::color_to_565(0xFF208000));
            canvas->drawRect(SkRect::MakeXYWH(2,0,1,1), p);
        canvas->restore();
    canvas->restore();
}
