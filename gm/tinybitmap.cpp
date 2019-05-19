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

namespace skiagm {

static SkBitmap make_bitmap() {
    SkBitmap bm;
    bm.allocN32Pixels(1, 1);
    *bm.getAddr32(0, 0) = SkPackARGB32(0x80, 0x80, 0, 0);
    return bm;
}

class TinyBitmapGM : public GM {
public:
    TinyBitmapGM() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    SkString onShortName() {
        return SkString("tinybitmap");
    }

    virtual SkISize onISize() { return SkISize::Make(100, 100); }

    virtual void onDraw(SkCanvas* canvas) {
        SkBitmap bm = make_bitmap();
        SkPaint paint;
        paint.setAlphaf(0.5f);
        paint.setShader(bm.makeShader(SkTileMode::kRepeat, SkTileMode::kMirror));
        canvas->drawPaint(paint);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new TinyBitmapGM; )

}
