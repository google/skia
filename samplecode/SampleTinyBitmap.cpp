
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkColorPriv.h"
#include "SkShader.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkUtils.h"

static SkBitmap make_bitmap() {
    const int N = 1;

    SkPMColor c[N];
    for (int i = 0; i < N; i++) {
        c[i] = SkPackARGB32(0x80, 0x80, 0, 0);
    }
    SkColorTable* ctable = new SkColorTable(c, N);

    SkBitmap bm;
    bm.allocPixels(SkImageInfo::Make(1, 1, kIndex_8_SkColorType,
                                     kPremul_SkAlphaType),
                   nullptr, ctable);
    ctable->unref();

    bm.lockPixels();
    for (int y = 0; y < bm.height(); y++) {
        uint8_t* p = bm.getAddr8(0, y);
        for (int x = 0; x < bm.width(); x++) {
            p[x] = 0;
        }
    }
    bm.unlockPixels();
    return bm;
}

class TinyBitmapView : public SampleView {
    SkBitmap    fBM;
public:
    TinyBitmapView() {
        fBM = make_bitmap();
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "TinyBitmap");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    static void setBitmapOpaque(SkBitmap* bm, bool isOpaque) {
        SkAutoLockPixels alp(*bm);  // needed for ctable
        bm->setAlphaType(isOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkShader* s = SkShader::CreateBitmapShader(fBM, SkShader::kRepeat_TileMode,
                                                   SkShader::kMirror_TileMode);
        SkPaint paint;
        paint.setShader(s)->unref();
        canvas->drawPaint(paint);
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TinyBitmapView; }
static SkViewRegister reg(MyFactory);
