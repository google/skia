#include "SampleCode.h"
#include "SkDumpCanvas.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "Sk64.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkShape.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkXfermode.h"

#include "SkStream.h"
#include "SkXMLParser.h"

class SignalShape : public SkShape {
public:
    SignalShape() : fSignal(0) {}

    SkShape* setSignal(int n) {
        fSignal = n;
        return this;
    }

protected:
    virtual void onDraw(SkCanvas* canvas) {
        SkDebugf("---- sc %d\n", canvas->getSaveCount() - 1);
    }

private:
    int fSignal;
};

static SkPMColor SignalProc(SkPMColor src, SkPMColor dst) {
    return dst;
}

/*  Picture playback will skip blocks of draw calls that follow a clip() call
    that returns empty, and jump down to the corresponding restore() call.

    This is a great preformance win for drawing very large/tall pictures with
    a small visible window (think scrolling a long document). These tests make
    sure that (a) we are performing the culling, and (b) we don't get confused
    by nested save() calls, nor by calls to restoreToCount().
 */
static void test_saveRestoreCulling() {
    SkPaint signalPaint;
    SignalShape signalShape;

    SkPicture pic;
    SkRect r = SkRect::MakeWH(0, 0);
    int n;
    SkCanvas* canvas = pic.beginRecording(100, 100);
    int startN = canvas->getSaveCount();
    SkDebugf("---- start sc %d\n", startN);
    canvas->drawShape(signalShape.setSignal(1));
    canvas->save();
    canvas->drawShape(signalShape.setSignal(2));
    n = canvas->save();
    canvas->drawShape(signalShape.setSignal(3));
    canvas->save();
    canvas->clipRect(r);
    canvas->drawShape(signalShape.setSignal(4));
    canvas->restoreToCount(n);
    canvas->drawShape(signalShape.setSignal(5));
    canvas->restore();
    canvas->drawShape(signalShape.setSignal(6));
    SkASSERT(canvas->getSaveCount() == startN);

    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
    bm.allocPixels();
    SkCanvas c(bm);
    c.drawPicture(pic);
}

///////////////////////////////////////////////////////////////////////////////

#include "SkImageRef_GlobalPool.h"

static SkBitmap load_bitmap() {
    SkStream* stream = new SkFILEStream("/skimages/sesame_street_ensemble-hp.jpg");
    SkAutoUnref aur(stream);
    
    SkBitmap bm;
    if (SkImageDecoder::DecodeStream(stream, &bm, SkBitmap::kNo_Config,
                                     SkImageDecoder::kDecodeBounds_Mode)) {
        SkPixelRef* pr = new SkImageRef_GlobalPool(stream, bm.config(), 1);
        bm.setPixelRef(pr)->unref();
    }
    return bm;
}

static void drawCircle(SkCanvas* canvas, int r, SkColor color) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(color);

    canvas->drawCircle(SkIntToScalar(r), SkIntToScalar(r), SkIntToScalar(r),
                       paint);
}

class PictureView : public SkView {
    SkBitmap fBitmap;
public:
	PictureView() {
        SkImageRef_GlobalPool::SetRAMBudget(16 * 1024);

        fBitmap = load_bitmap();

        fPicture = new SkPicture;
        SkCanvas* canvas = fPicture->beginRecording(100, 100);
        SkPaint paint;
        paint.setAntiAlias(true);
        
        canvas->drawBitmap(fBitmap, 0, 0, NULL);

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

        test_saveRestoreCulling();
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

        canvas->save();
        canvas->scale(0.5f, 0.5f);
        canvas->drawBitmap(fBitmap, 0, 0, NULL);
        canvas->restore();

        const char beforeStr[] = "before circle";
        const char afterStr[] = "after circle";

        paint.setAntiAlias(true);
    
        paint.setColor(SK_ColorRED);
        canvas->drawData(beforeStr, sizeof(beforeStr));
        canvas->drawCircle(SkIntToScalar(50), SkIntToScalar(50),
                           SkIntToScalar(40), paint);
        canvas->drawData(afterStr, sizeof(afterStr));
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

        if (false) {
            SkDebugfDumper dumper;
            SkDumpCanvas dumpCanvas(&dumper);
            dumpCanvas.drawPicture(*pict);
        }
        
        // test that we can re-record a subpicture, and see the results
        
        SkRandom rand(SampleCode::GetAnimTime());
        canvas->translate(SkIntToScalar(10), SkIntToScalar(250));
        drawCircle(fSubPicture->beginRecording(50, 50), 25,
                   rand.nextU() | 0xFF000000);
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

    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new PictureView; }
static SkViewRegister reg(MyFactory);

