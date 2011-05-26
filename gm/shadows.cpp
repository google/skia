#include "gm.h"
#include "SkBlurDrawLooper.h"

namespace skiagm {

///////////////////////////////////////////////////////////////////////////////

static void setup(SkPaint* paint, SkColor c, SkScalar strokeWidth) {
    paint->setColor(c);
    if (strokeWidth < 0) {
        paint->setStyle(SkPaint::kFill_Style);
    } else {
        paint->setStyle(SkPaint::kStroke_Style);
        paint->setStrokeWidth(strokeWidth);
    }
}

class ShadowsGM : public GM {
public:
    SkPath fCirclePath;
    SkRect fRect;

    ShadowsGM() {
        fCirclePath.addCircle(SkIntToScalar(20), SkIntToScalar(20), SkIntToScalar(10) );
        fRect.set(SkIntToScalar(10), SkIntToScalar(10),
                  SkIntToScalar(30), SkIntToScalar(30));
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
        new SkBlurDrawLooper (SkIntToScalar(10), SkIntToScalar(5),
                              SkIntToScalar(10), 0xFF0000FF,
                              SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                              SkBlurDrawLooper::kOverrideColor_BlurFlag |
                              SkBlurDrawLooper::kHighQuality_BlurFlag );
    SkAutoUnref aurL0(shadowLoopers[0]);
    shadowLoopers[1] =
        new SkBlurDrawLooper (SkIntToScalar(10), SkIntToScalar(5),
                              SkIntToScalar(10), 0xFF0000FF,
                              SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                              SkBlurDrawLooper::kOverrideColor_BlurFlag );
    SkAutoUnref aurL1(shadowLoopers[1]);
    shadowLoopers[2] =
        new SkBlurDrawLooper (SkIntToScalar(5), SkIntToScalar(5),
                              SkIntToScalar(10), 0xFF000000,
                              SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                              SkBlurDrawLooper::kHighQuality_BlurFlag  );
    SkAutoUnref aurL2(shadowLoopers[2]);
    shadowLoopers[3] =
        new SkBlurDrawLooper (SkIntToScalar(5), SkIntToScalar(-5),
                              SkIntToScalar(-10), 0x7FFF0000,
                              SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                              SkBlurDrawLooper::kOverrideColor_BlurFlag |
                              SkBlurDrawLooper::kHighQuality_BlurFlag  );
    SkAutoUnref aurL3(shadowLoopers[3]);
    shadowLoopers[4] =
        new SkBlurDrawLooper (SkIntToScalar(0), SkIntToScalar(5),
                              SkIntToScalar(5), 0xFF000000,
                              SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                              SkBlurDrawLooper::kOverrideColor_BlurFlag |
                              SkBlurDrawLooper::kHighQuality_BlurFlag  );
    SkAutoUnref aurL4(shadowLoopers[4]);

    static const struct {
        SkColor fColor;
        SkScalar fStrokeWidth;
    } gRec[] = {
        { SK_ColorRED,      -SK_Scalar1 },
        { SK_ColorGREEN,    SkIntToScalar(4) },
    };

    SkPaint paint;
    paint.setAntiAlias(true);
    for (size_t i = 0; i < SK_ARRAY_COUNT(shadowLoopers); ++i) {
        SkAutoCanvasRestore acr(canvas, true);

        paint.setLooper(shadowLoopers[i]);

        canvas->translate(SkIntToScalar(i*40), SkIntToScalar(0));
        setup(&paint, gRec[0].fColor, gRec[0].fStrokeWidth);
        canvas->drawRect(fRect, paint);

        canvas->translate(SkIntToScalar(0), SkIntToScalar(40));
        setup(&paint, gRec[1].fColor, gRec[1].fStrokeWidth);
        canvas->drawPath(fCirclePath, paint);
    }
}

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ShadowsGM; }
static GMRegistry reg(MyFactory);

}
