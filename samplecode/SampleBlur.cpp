/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkGradientShader.h"
#include "SkUtils.h"
#include "SkView.h"

static SkBitmap make_bitmap() {
    SkPMColor c[256];
    for (int i = 0; i < 256; i++) {
        c[i] = SkPackARGB32(255 - i, 0, 0, 0);
    }

    SkBitmap bm;
    bm.allocPixels(SkImageInfo::Make(256, 256, kIndex_8_SkColorType,
                                     kPremul_SkAlphaType),
                   SkColorTable::Make(c, 256));

    const float cx = bm.width() * 0.5f;
    const float cy = bm.height() * 0.5f;
    for (int y = 0; y < bm.height(); y++) {
        float dy = y - cy;
        dy *= dy;
        uint8_t* p = bm.getAddr8(0, y);
        for (int x = 0; x < 256; x++) {
            float dx = x - cx;
            dx *= dx;
            float d = (dx + dy) / (cx/2);
            int id = (int)d;
            if (id > 255) {
                id = 255;
            }
            p[x] = id;
        }
    }
    return bm;
}

class BlurView : public SampleView {
    SkBitmap    fBM;
public:
    BlurView() {
        if (false) { // avoid bit rot, suppress warning
            fBM = make_bitmap();
        }
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Blur");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        drawBG(canvas);

        SkBlurStyle NONE = SkBlurStyle(-999);
        static const struct {
            SkBlurStyle fStyle;
            int         fCx, fCy;
        } gRecs[] = {
            { NONE,                                 0,  0 },
            { kInner_SkBlurStyle,  -1,  0 },
            { kNormal_SkBlurStyle,  0,  1 },
            { kSolid_SkBlurStyle,   0, -1 },
            { kOuter_SkBlurStyle,   1,  0 },
        };

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(25);
        canvas->translate(-40, 0);

        SkBlurMaskFilter::BlurFlags flags = SkBlurMaskFilter::kNone_BlurFlag;
        for (int j = 0; j < 2; j++) {
            canvas->save();
            paint.setColor(SK_ColorBLUE);
            for (size_t i = 0; i < SK_ARRAY_COUNT(gRecs); i++) {
                if (gRecs[i].fStyle != NONE) {
                    paint.setMaskFilter(SkBlurMaskFilter::Make(gRecs[i].fStyle,
                                      SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(20)),
                                      flags));
                } else {
                    paint.setMaskFilter(nullptr);
                }
                canvas->drawCircle(200 + gRecs[i].fCx*100.f,
                                   200 + gRecs[i].fCy*100.f, 50, paint);
            }
            // draw text
            {
                paint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle,
                                                           SkBlurMask::ConvertRadiusToSigma(4),
                                                           flags));
                SkScalar x = SkIntToScalar(70);
                SkScalar y = SkIntToScalar(400);
                paint.setColor(SK_ColorBLACK);
                canvas->drawString("Hamburgefons Style", x, y, paint);
                canvas->drawString("Hamburgefons Style", x, y + SkIntToScalar(50), paint);
                paint.setMaskFilter(nullptr);
                paint.setColor(SK_ColorWHITE);
                x -= SkIntToScalar(2);
                y -= SkIntToScalar(2);
                canvas->drawString("Hamburgefons Style", x, y, paint);
            }
            canvas->restore();
            flags = SkBlurMaskFilter::kHighQuality_BlurFlag;
            canvas->translate(350, 0);
        }
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new BlurView; }
static SkViewRegister reg(MyFactory);
