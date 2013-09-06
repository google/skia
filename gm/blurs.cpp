
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"

namespace skiagm {

class BlursGM : public GM {
public:
    BlursGM() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
#ifdef SK_SCALAR_IS_FIXED
    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        // SkCanvas::drawCircle, used by this test, performs a quick reject.
        // The large size given to the device used by SkGPipeCanvas means that
        // the device clip will not be set properly and circles will be
        // rejected when in FIXED.
        return this->INHERITED::onGetFlags() | GM::kSkipPipe_Flag;
    }
#endif

    virtual SkString onShortName() {
        return SkString("blurs");
    }

    virtual SkISize onISize() {
        return make_isize(700, 500);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkBlurMaskFilter::BlurStyle NONE = SkBlurMaskFilter::BlurStyle(-999);
        static const struct {
            SkBlurMaskFilter::BlurStyle fStyle;
            int                         fCx, fCy;
        } gRecs[] = {
            { NONE,                                 0,  0 },
            { SkBlurMaskFilter::kInner_BlurStyle,  -1,  0 },
            { SkBlurMaskFilter::kNormal_BlurStyle,  0,  1 },
            { SkBlurMaskFilter::kSolid_BlurStyle,   0, -1 },
            { SkBlurMaskFilter::kOuter_BlurStyle,   1,  0 },
        };

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(25));
        canvas->translate(SkIntToScalar(-40), SkIntToScalar(0));

        SkBlurMaskFilter::BlurFlags flags = SkBlurMaskFilter::kNone_BlurFlag;
        for (int j = 0; j < 2; j++) {
            canvas->save();
            paint.setColor(SK_ColorBLUE);
            for (size_t i = 0; i < SK_ARRAY_COUNT(gRecs); i++) {
                if (gRecs[i].fStyle != NONE) {
                    SkMaskFilter* mf = SkBlurMaskFilter::Create(gRecs[i].fStyle,
                                           SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(20)),
                                           flags);
                    paint.setMaskFilter(mf)->unref();
                } else {
                    paint.setMaskFilter(NULL);
                }
                canvas->drawCircle(SkIntToScalar(200 + gRecs[i].fCx*100),
                                   SkIntToScalar(200 + gRecs[i].fCy*100),
                                   SkIntToScalar(50),
                                   paint);
            }
            // draw text
            {
                SkMaskFilter* mf = SkBlurMaskFilter::Create(SkBlurMaskFilter::kNormal_BlurStyle,
                                           SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(4)),
                                           flags);
                paint.setMaskFilter(mf)->unref();
                SkScalar x = SkIntToScalar(70);
                SkScalar y = SkIntToScalar(400);
                paint.setColor(SK_ColorBLACK);
                canvas->drawText("Hamburgefons Style", 18, x, y, paint);
                canvas->drawText("Hamburgefons Style", 18,
                                 x, y + SkIntToScalar(50), paint);
                paint.setMaskFilter(NULL);
                paint.setColor(SK_ColorWHITE);
                x -= SkIntToScalar(2);
                y -= SkIntToScalar(2);
                canvas->drawText("Hamburgefons Style", 18, x, y, paint);
            }
            canvas->restore();
            flags = SkBlurMaskFilter::kHighQuality_BlurFlag;
            canvas->translate(SkIntToScalar(350), SkIntToScalar(0));
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new BlursGM; }
static GMRegistry reg(MyFactory);

}
