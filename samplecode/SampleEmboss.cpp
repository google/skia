#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "Sk64.h"
#include "SkColorShader.h"
#include "SkEmbossMaskFilter.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkKernel33MaskFilter.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkXfermode.h"

class EmbossView : public SkView {
    SkEmbossMaskFilter::Light   fLight;
public:
	EmbossView()
    {
        fLight.fDirection[0] = SK_Scalar1;
        fLight.fDirection[1] = SK_Scalar1;
        fLight.fDirection[2] = SK_Scalar1;
        fLight.fAmbient = 128;
        fLight.fSpecular = 16*2;
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)
    {
        if (SampleCode::TitleQ(*evt))
        {
            SampleCode::TitleR(evt, "Emboss");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas)
    {
        canvas->drawColor(SK_ColorWHITE);
    }
    
    virtual void onDraw(SkCanvas* canvas)
    {
        this->drawBG(canvas);

        SkPaint paint;
        
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(10));
        paint.setMaskFilter(new SkEmbossMaskFilter(fLight, SkIntToScalar(4)))->unref();
        paint.setShader(new SkColorShader(SK_ColorBLUE))->unref();
        paint.setDither(true);
        
        canvas->drawCircle(SkIntToScalar(50), SkIntToScalar(50),
                           SkIntToScalar(30), paint);
    }
    
private:

    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new EmbossView; }
static SkViewRegister reg(MyFactory);

