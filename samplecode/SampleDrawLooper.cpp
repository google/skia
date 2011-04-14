#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkRandom.h"
#include "SkLayerDrawLooper.h"
#include "SkBlurMaskFilter.h"

#define WIDTH   200
#define HEIGHT  200

class LooperView : public SkView {
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
        
        for (int i = 0; i < SK_ARRAY_COUNT(gParams); i++) {
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

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
//        canvas->drawColor(SK_ColorWHITE);
    }

    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

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

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        this->inval(NULL);
        return this->INHERITED::onFindClickHandler(x, y);
    }

    virtual bool onClick(Click* click) {
        return this->INHERITED::onClick(click);
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new LooperView; }
static SkViewRegister reg(MyFactory);

