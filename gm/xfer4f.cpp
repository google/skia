/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkImageInfo.h"
#include "SkXfermode.h"

static void draw_rect(SkCanvas* canvas, const SkRect& r, SkColor c, SkColorProfileType profile) {
    const SkIRect ir = r.round();

    SkBitmap bm;
    bm.allocN32Pixels(ir.width(), ir.height());
    bm.eraseColor(0xFFFFFFFF);
    SkPixmap pm;
    bm.peekPixels(&pm);

    uint32_t flags = 0;
    if (SkColorGetA(c) == 0xFF) {
        flags |= SkXfermode::kSrcIsOpaque_PM4fFlag;
    }
    if (kSRGB_SkColorProfileType == profile) {
        flags |= SkXfermode::kDstIsSRGB_PM4fFlag;
    }

    const SkXfermode::PM4fState state { nullptr, flags };

    const SkPM4f src = SkColor4f::FromColor(c).premul();
    auto proc1 = SkXfermode::GetPM4fProc1(SkXfermode::kSrcOver_Mode, flags);
    for (int y = 0; y < ir.height()/2; ++y) {
        proc1(state, pm.writable_addr32(0, y), src, ir.width(), nullptr);
    }

    SkPM4f srcRow[1000];
    for (int i = 0; i < ir.width(); ++i) {
        srcRow[i] = src;
    }
    auto procN = SkXfermode::GetPM4fProcN(SkXfermode::kSrcOver_Mode, flags);
    // +1 to skip a row, so we can see the boundary between proc1 and procN
    for (int y = ir.height()/2 + 1; y < ir.height(); ++y) {
        procN(state, pm.writable_addr32(0, y), srcRow, ir.width(), nullptr);
    }

    canvas->drawBitmap(bm, r.left(), r.top(), nullptr);
}

/*
 *  Test SkXfer4fProcs directly for src-over, comparing them to current SkColor blits.
 */
DEF_SIMPLE_GM(xfer4f_srcover, canvas, 580, 380) {
    const SkScalar W = 50;
    const SkScalar H = 100;

    const int profiles[] = {
        -1,
        kLinear_SkColorProfileType,
        kSRGB_SkColorProfileType,
    };
    const SkColor colors[] = {
        SK_ColorBLACK, SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE,
        0x88000000, 0x88FF0000, 0x8800FF00, 0x880000FF
    };
    canvas->translate(20, 20);

    const SkRect r = SkRect::MakeWH(W, H);
    for (auto profile : profiles) {
        canvas->save();
        for (SkColor c : colors) {
            if (profile < 0) {
                SkPaint p;
                p.setColor(c);
                canvas->drawRect(r, p);
            } else {
                draw_rect(canvas, r, c, (SkColorProfileType)profile);
            }
            canvas->translate(W + 20, 0);
        }
        canvas->restore();
        canvas->translate(0, H + 20);
    }
}
