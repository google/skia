#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "Sk64.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkKernel33MaskFilter.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkXfermode.h"

#include "SkStream.h"
#include "SkXMLParser.h"

static void drawCircle(SkCanvas* canvas, int r, SkColor color) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(color);

    canvas->drawCircle(SkIntToScalar(r), SkIntToScalar(r), SkIntToScalar(r),
                       paint);
}

class PictureView : public SkView {
public:
	PictureView() {
        fPicture = new SkPicture;
        SkCanvas* canvas = fPicture->beginRecording(100, 100);
        SkPaint paint;
        paint.setAntiAlias(true);
        
        drawCircle(canvas, 50, SK_ColorBLACK);
        fSubPicture = new SkPicture;
        canvas->drawPicture(*fSubPicture);
        canvas->translate(SkIntToScalar(50), 0);
        canvas->drawPicture(*fSubPicture);
        canvas->translate(0, SkIntToScalar(50));
        canvas->drawPicture(*fSubPicture);
        canvas->translate(SkIntToScalar(-50), 0);
        canvas->drawPicture(*fSubPicture);
        // fPicture now has (4) references to us. We can release ours, and just
        // unref fPicture in our destructor, and it will in turn take care of
        // the other references to fSubPicture
        fSubPicture->unref();
    }
    
    virtual ~PictureView() {
        fPicture->unref();
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Picture");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas) {
//        canvas->drawColor(0xFFDDDDDD);
        canvas->drawColor(SK_ColorWHITE);
   //     canvas->drawColor(SK_ColorBLACK);
    }
    
    void drawSomething(SkCanvas* canvas) {
        SkPaint paint;
        
        paint.setAntiAlias(true);
    
        paint.setColor(SK_ColorRED);
        canvas->drawCircle(SkIntToScalar(50), SkIntToScalar(50),
                           SkIntToScalar(40), paint);
        paint.setColor(SK_ColorBLACK);
        paint.setTextSize(SkIntToScalar(40));
        canvas->drawText("Picture", 7, SkIntToScalar(50), SkIntToScalar(62),
                         paint);
    }

    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        drawSomething(canvas);
        
        SkPicture* pict = new SkPicture;
        SkAutoUnref aur(pict);

        drawSomething(pict->beginRecording(100, 100));
        pict->endRecording();
        
        canvas->save();
        canvas->translate(SkIntToScalar(300), SkIntToScalar(50));
        canvas->scale(-SK_Scalar1, -SK_Scalar1);
        canvas->translate(-SkIntToScalar(100), -SkIntToScalar(50));
        canvas->drawPicture(*pict);
        canvas->restore();

        canvas->save();
        canvas->translate(SkIntToScalar(200), SkIntToScalar(150));
        canvas->scale(SK_Scalar1, -SK_Scalar1);
        canvas->translate(0, -SkIntToScalar(50));
        canvas->drawPicture(*pict);
        canvas->restore();
        
        canvas->save();
        canvas->translate(SkIntToScalar(100), SkIntToScalar(100));
        canvas->scale(-SK_Scalar1, SK_Scalar1);
        canvas->translate(-SkIntToScalar(100), 0);
        canvas->drawPicture(*pict);
        canvas->restore();
        
        // test that we can re-record a subpicture, and see the results
        
        canvas->translate(SkIntToScalar(10), SkIntToScalar(250));
        drawCircle(fSubPicture->beginRecording(50, 50), 25,
                   fRand.nextU() | 0xFF000000);
        canvas->drawPicture(*fPicture);
        delayInval(500);
    }
    
private:
    #define INVAL_ALL_TYPE  "inval-all"
    
    void delayInval(SkMSec delay) {
        (new SkEvent(INVAL_ALL_TYPE))->post(this->getSinkID(), delay);
    }
    
    virtual bool onEvent(const SkEvent& evt) {
        if (evt.isType(INVAL_ALL_TYPE)) {
            this->inval(NULL);
            return true;
        }
        return this->INHERITED::onEvent(evt);
    }

    SkPicture*  fPicture;
    SkPicture*  fSubPicture;
    SkRandom    fRand;

    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new PictureView; }
static SkViewRegister reg(MyFactory);

