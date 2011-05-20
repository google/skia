#include "gm.h"
#include "SkBlurMaskFilter.h"

namespace skiagm {

class BlursGM : public GM {
public:
    BlursGM() {}

protected:
    virtual SkString onShortName() {
        return SkString("blurs");
    }

    virtual SkISize onISize() {
        return make_isize(700, 500);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }

    virtual void onDraw(SkCanvas* canvas) {
        drawBG(canvas);

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
                    SkMaskFilter* mf = SkBlurMaskFilter::Create(
                            SkIntToScalar(20), gRecs[i].fStyle, flags
                    );
                    paint.setMaskFilter(mf)->unref();
                } else {
                    paint.setMaskFilter(NULL);
                }
                canvas->drawCircle(SkIntToScalar(200 + gRecs[i].fCx*100)
                                   , SkIntToScalar(200 + gRecs[i].fCy*100)
                                   , SkIntToScalar(50)
                                   , paint);
            }
            // draw text
            {
                SkMaskFilter* mf = SkBlurMaskFilter::Create(
                        SkIntToScalar(4)
                        , SkBlurMaskFilter::kNormal_BlurStyle
                        , flags
                );
                paint.setMaskFilter(mf)->unref();
                SkScalar x = SkIntToScalar(70);
                SkScalar y = SkIntToScalar(400);
                paint.setColor(SK_ColorBLACK);
                canvas->drawText("Hamburgefons Style", 18, x, y, paint);
                canvas->drawText("Hamburgefons Style", 18
                                 , x, y + SkIntToScalar(50), paint);
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

