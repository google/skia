#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkRandom.h"
#include "SkLayerDrawLooper.h"
#include "SkBlurMaskFilter.h"

#define WIDTH   200
#define HEIGHT  200

class LooperView : public SampleView {
public:

    SkLayerDrawLooper*   fLooper;

	LooperView() {
        static const struct {
            SkColor         fColor;
            SkPaint::Style  fStyle;
            SkScalar        fWidth;
            SkScalar        fOffset;
            int             fBlur;
        } gParams[] = {
            { SK_ColorWHITE, SkPaint::kStroke_Style, SkIntToScalar(1)*3/4, 0, 0 },
            { SK_ColorRED, SkPaint::kStroke_Style, SkIntToScalar(4), 0, 0 },
            { SK_ColorBLUE, SkPaint::kFill_Style, 0, 0, 0 },
            { 0x88000000, SkPaint::kFill_Style, 0, SkIntToScalar(10), 3 }
        };

        fLooper = new SkLayerDrawLooper;

        SkLayerDrawLooper::LayerInfo info;
        info.fFlagsMask = SkPaint::kAntiAlias_Flag;
        info.fPaintBits = SkLayerDrawLooper::kStyle_Bit | SkLayerDrawLooper::kMaskFilter_Bit;
        info.fColorMode = SkXfermode::kSrc_Mode;
        
        for (size_t i = 0; i < SK_ARRAY_COUNT(gParams); i++) {
            info.fOffset.set(gParams[i].fOffset, gParams[i].fOffset);
            SkPaint* paint = fLooper->addLayer(info);
            paint->setAntiAlias(true);
            paint->setColor(gParams[i].fColor);
            paint->setStyle(gParams[i].fStyle);
            paint->setStrokeWidth(gParams[i].fWidth);
            if (gParams[i].fBlur > 0) {
                SkMaskFilter* mf = SkBlurMaskFilter::Create(SkIntToScalar(gParams[i].fBlur),
                                                            SkBlurMaskFilter::kNormal_BlurStyle);
                paint->setMaskFilter(mf)->unref();
            }
        }
        
        this->setBGColor(0xFFDDDDDD);
    }

    virtual ~LooperView() {
        SkSafeUnref(fLooper);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "DrawLooper");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkPaint  paint;
        paint.setTextSize(SkIntToScalar(72));
        paint.setLooper(fLooper);

        canvas->drawCircle(SkIntToScalar(50), SkIntToScalar(50),
                           SkIntToScalar(30), paint);

        canvas->drawRectCoords(SkIntToScalar(150), SkIntToScalar(50),
                               SkIntToScalar(200), SkIntToScalar(100), paint);

        canvas->drawText("Looper", 6, SkIntToScalar(230), SkIntToScalar(100),
                         paint);
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new LooperView; }
static SkViewRegister reg(MyFactory);

