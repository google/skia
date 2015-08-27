
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkColorPriv.h"
#include "SkShader.h"
#include "SkCanvas.h"
#include "SkUtils.h"

namespace skiagm {

static SkBitmap make_bitmap() {
    const SkPMColor c[] = { SkPackARGB32(0x80, 0x80, 0, 0) };
    SkColorTable* ctable = new SkColorTable(c, SK_ARRAY_COUNT(c));

    SkBitmap bm;
    bm.allocPixels(SkImageInfo::Make(1, 1, kIndex_8_SkColorType,
                                     kPremul_SkAlphaType),
                   nullptr, ctable);
    ctable->unref();

    bm.lockPixels();
    *bm.getAddr8(0, 0) = 0;
    bm.unlockPixels();
    return bm;
}

class TinyBitmapGM : public GM {
public:
    TinyBitmapGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFDDDDDD));
    }

protected:
    SkString onShortName() {
        return SkString("tinybitmap");
    }

    virtual SkISize onISize() { return SkISize::Make(100, 100); }

    virtual void onDraw(SkCanvas* canvas) {
        SkBitmap bm = make_bitmap();
        SkShader* s =
            SkShader::CreateBitmapShader(bm, SkShader::kRepeat_TileMode,
                                         SkShader::kMirror_TileMode);
        SkPaint paint;
        paint.setAlpha(0x80);
        paint.setShader(s)->unref();
        canvas->drawPaint(paint);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new TinyBitmapGM; }
static GMRegistry reg(MyFactory);

}
