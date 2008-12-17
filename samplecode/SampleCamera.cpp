#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkCamera.h"
#include "SkEmbossMaskFilter.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkRandom.h"

class CameraView : public SkView {
public:
	CameraView()
    {
        fRX = fRY = fRZ = 0;
    }
    
    virtual ~CameraView()
    {
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)
    {
        if (SampleCode::TitleQ(*evt))
        {
            SampleCode::TitleR(evt, "Camera");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas)
    {
        canvas->drawColor(0xFFDDDDDD);
//        canvas->drawColor(0, SkPorterDuff::kClear_Mode);
    }
    
    virtual void onDraw(SkCanvas* canvas)
    {
        this->drawBG(canvas);

        canvas->translate(this->width()/2, this->height()/2);

        Sk3DView    view;
        view.rotateX(SkIntToScalar(fRX));
        view.rotateY(SkIntToScalar(fRY));
        view.applyToCanvas(canvas);
        
        SkPaint paint;
        SkScalar rad = SkIntToScalar(50);
        SkScalar dim = rad*2;

        if (view.dotWithNormal(0, 0, SK_Scalar1) < 0) {
            paint.setColor(SK_ColorRED);
        }
        
        paint.setAntiAlias(true);

#if 0
        SkEmbossMaskFilter::Light light;
        light.fDirection[0] = SK_Scalar1;
        light.fDirection[1] = SK_Scalar1;
        light.fDirection[2] = SK_Scalar1;
        light.fAmbient = 180;
        light.fSpecular = 16 * 2;
        paint.setMaskFilter(new SkEmbossMaskFilter(light, SkIntToScalar(4)));
#endif

        canvas->drawCircle(0, 0, rad, paint);
        canvas->drawCircle(-dim, -dim, rad, paint);
        canvas->drawCircle(-dim,  dim, rad, paint);
        canvas->drawCircle( dim, -dim, rad, paint);
        canvas->drawCircle( dim,  dim, rad, paint);
        
        fRY += 1;
        if (fRY >= 360)
            fRY = 0;
        this->inval(NULL);
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) 
    {
        SkScalar angle = SkScalarDiv(this->height()/2 - y, this->height());
        fRX = SkScalarRound(angle * 180);
        return this->INHERITED::onFindClickHandler(x, y);
    }
    
    virtual bool onClick(Click* click) 
    {
        return this->INHERITED::onClick(click);
    }
    
private:
    int fRX, fRY, fRZ;
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new CameraView; }
static SkViewRegister reg(MyFactory);

