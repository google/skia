#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkBlurMaskFilter.h"
#include "SkCamera.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkDevice.h"
#include "SkGradientShader.h"
#include "SkImageDecoder.h"
#include "SkInterpolator.h"
#include "SkMaskFilter.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkShaderExtras.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkUtils.h"
#include "SkKey.h"
#include "SkPorterDuff.h"
#include "SkXfermode.h"
#include "SkDrawFilter.h"

#include "test.h"

class TestsView : public SkView {
public:
    skia::Test::Iter fIter;

	TestsView() {}

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Tests");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        skia::Test* test = fIter.next();
        if (NULL == test) {
            fIter.reset();
            test = fIter.next();
        }
        
        SkIPoint    size;
        test->getSize(&size);
        
        SkBitmap    bitmap;
        bitmap.setConfig(SkBitmap::kARGB_8888_Config, size.fX, size.fY);
        bitmap.allocPixels();
        bitmap.eraseColor(0);
        
        SkCanvas c(bitmap);
        test->draw(&c);
        
        canvas->drawBitmap(bitmap, SkIntToScalar(10), SkIntToScalar(10), NULL);
        
        SkString str;
        test->getString(skia::Test::kTitle, &str);
        SkDebugf("--- %s\n", str.c_str());
        delete test;
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        this->inval(NULL);
        
        return this->INHERITED::onFindClickHandler(x, y);
    }
    
    virtual bool onClick(Click* click) {
        this->inval(NULL);
        return this->INHERITED::onClick(click);
    }

	virtual bool handleKey(SkKey key) {
        this->inval(NULL);
        return true;
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TestsView; }
static SkViewRegister reg(MyFactory);

