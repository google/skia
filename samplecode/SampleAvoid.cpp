#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkAvoidXfermode.h"

///////////////////////////////////////////////////////////////////////////////

class AvoidView : public SkView {
    SkShader* fShader;

    enum {
        W = 480,
        H = 320
    };
public:
    AvoidView() {
        SkColor colors[] = { SK_ColorRED, SK_ColorYELLOW, SK_ColorGREEN, SK_ColorCYAN, SK_ColorBLUE };

#if 0
        SkPoint pts[] = { 0, 0, SkIntToScalar(100), SkIntToScalar(100) };
        fShader = SkGradientShader::CreateLinear(pts, colors, NULL,
                                                 SK_ARRAY_COUNT(colors),
                                                 SkShader::kMirror_TileMode);
#else
        SkPoint pts[] = { SkIntToScalar(W)/2, SkIntToScalar(H)/2 };
        fShader = SkGradientShader::CreateRadial(pts[0], SkIntToScalar(H)/5,
                                                 colors, NULL,
                                                 SK_ARRAY_COUNT(colors),
                                                 SkShader::kMirror_TileMode);
#endif
    }
    
    virtual ~AvoidView() {
        fShader->unref();
    }

protected:
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "AvoidXfermode");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
        
        SkPaint paint;
        SkRect r = { 0, 0, SkIntToScalar(W), SkIntToScalar(H) };
        
        canvas->translate(r.width() / 6, r.height() / 6);

        paint.setShader(fShader);
        canvas->drawRect(r, paint);

        static const struct {
            int                     fTolerance;
            SkAvoidXfermode::Mode   fMode;
            float                   fDX, fDY;
        } gData[] = {
            { 16,       SkAvoidXfermode::kAvoidColor_Mode, 0, 0 },
            { 255-16,   SkAvoidXfermode::kAvoidColor_Mode, 1, 0 },
            { 16,       SkAvoidXfermode::kTargetColor_Mode, 0, 1 },
            { 255-16,   SkAvoidXfermode::kTargetColor_Mode, 1, 1 },
        };

        paint.setShader(NULL);
        paint.setColor(SK_ColorMAGENTA);
        
        SkPaint frameP;
        frameP.setStyle(SkPaint::kStroke_Style);

        for (size_t i = 0; i < SK_ARRAY_COUNT(gData); i++) {
            SkAvoidXfermode mode(SK_ColorGREEN, gData[i].fTolerance,
                                 gData[i].fMode);
            paint.setXfermode(&mode);
            int div = 3;
            SkRect rr = { 0, 0, r.width()/div, r.height()/div };
            rr.offset(r.width()/4 - rr.width()/2, r.height()/4 - rr.height()/2);
            rr.offset(r.width() * gData[i].fDX/2, r.height() * gData[i].fDY/2);
            canvas->drawRect(rr, paint);
            paint.setXfermode(NULL);

            canvas->drawRect(rr, frameP);
        }
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() {
    return new AvoidView;
}

static SkViewRegister reg(MyFactory);

