#include "gm.h"
#include "SkPicture.h"
#include "SkRectShape.h"
#include "SkBlurDrawLooper.h"

namespace skiagm {

///////////////////////////////////////////////////////////////////////////////

class ShadowsGM : public GM {

public:
    SkPath fCirclePath;
    SkPaint fPaint;
    SkRectShape fRectShape;
    ShadowsGM() {
        fCirclePath.addCircle(SkIntToScalar(20), SkIntToScalar(20), SkIntToScalar(10) );
    fPaint.setStrokeWidth(SkIntToScalar(4));
    fPaint.setAntiAlias(true);
    fPaint.setColor(0xFF00FF00);
    fPaint.setStyle(SkPaint::kStroke_Style); 
    SkRect rect;
    rect.set(SkIntToScalar(10), SkIntToScalar(10),
             SkIntToScalar(30), SkIntToScalar(30));
    fRectShape.setRect(rect);
    fRectShape.paint().setColor(SK_ColorRED);
    }

    virtual ~ShadowsGM() {
    }

protected:
    virtual SkString onShortName() {
        return SkString("shadows");
    }

    virtual SkISize onISize() {
        return make_isize(200, 80);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }

    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

    SkBlurDrawLooper* shadowLoopers[5];
    shadowLoopers[0] =
        new SkBlurDrawLooper (10, 5, 10, 0xFF0000FF, 
                              SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                              SkBlurDrawLooper::kOverrideColor_BlurFlag |
                              SkBlurDrawLooper::kHighQuality_BlurFlag );
    SkAutoUnref aurL0(shadowLoopers[0]);
    shadowLoopers[1] =
        new SkBlurDrawLooper (10, 5, 10, 0xFF0000FF, 
                              SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                              SkBlurDrawLooper::kOverrideColor_BlurFlag );
    SkAutoUnref aurL1(shadowLoopers[1]);
    shadowLoopers[2] =
        new SkBlurDrawLooper (5, 5, 10, 0xFF000000,
                              SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                              SkBlurDrawLooper::kHighQuality_BlurFlag  );
    SkAutoUnref aurL2(shadowLoopers[2]);
    shadowLoopers[3] =
        new SkBlurDrawLooper (5, -5 ,-10, 0x7FFF0000, 
                              SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                              SkBlurDrawLooper::kOverrideColor_BlurFlag |
                              SkBlurDrawLooper::kHighQuality_BlurFlag  );
    SkAutoUnref aurL3(shadowLoopers[3]);
    shadowLoopers[4] =
        new SkBlurDrawLooper (0, 5, 5, 0xFF000000, 
                              SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                              SkBlurDrawLooper::kOverrideColor_BlurFlag |
                              SkBlurDrawLooper::kHighQuality_BlurFlag  );
    SkAutoUnref aurL4(shadowLoopers[4]);

    for (int looper = 0; looper < 5; looper ++)
    {
        fRectShape.paint().setLooper(shadowLoopers[looper]);
        canvas->resetMatrix();
        canvas->translate(SkIntToScalar(looper*40), SkIntToScalar(0));
        canvas->drawShape(&fRectShape);
        fPaint.setLooper(shadowLoopers[looper]); 
        canvas->translate(SkIntToScalar(0), SkIntToScalar(40));
        canvas->drawPath(fCirclePath, fPaint);
    }
}

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ShadowsGM; }
static GMRegistry reg(MyFactory);

}
