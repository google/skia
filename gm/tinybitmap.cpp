/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"

namespace {
class TinyBitmapGM : public skiagm::GM {
    void onOnceBeforeDraw() override { this->setBGColor(0xFFDDDDDD); }

    SkString onShortName() override { return SkString("tinybitmap"); }

    virtual SkISize onISize() override { return SkISize::Make(100, 100); }

    void onDraw(SkCanvas* canvas) override {
        SkBitmap bm;
        bm.allocN32Pixels(1, 1);
        *bm.getAddr32(0, 0) = SkPackARGB32(0x80, 0x80, 0, 0);
        SkPaint paint;
        paint.setAlphaf(0.5f);
        paint.setShader(bm.makeShader(SkTileMode::kRepeat, SkTileMode::kMirror));
        canvas->drawPaint(paint);
    }
};
}

DEF_GM( return new TinyBitmapGM; )
