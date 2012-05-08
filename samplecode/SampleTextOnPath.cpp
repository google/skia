
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkBlurDrawLooper.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkPathMeasure.h"

#define REPEAT_COUNT    1

static void textStrokePath(SkCanvas* canvas) {
    SkPaint paint;
    SkPath  path;
    SkRect  rect;

    canvas->save();
    canvas->scale(SkIntToScalar(250),SkIntToScalar(250));

    rect.set(SkFloatToScalar(0.0f),  SkFloatToScalar(0.21f),
             SkFloatToScalar(0.78f), SkFloatToScalar(0.99f));

    path.addArc(rect, SkIntToScalar(280), SkIntToScalar(350));

    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(0xFFFF0000);
    paint.setTextSize(SkFloatToScalar(0.085f));
    paint.setStrokeWidth(SkFloatToScalar(.005f));

    canvas->drawPath(path, paint);

    paint.setLooper(new SkBlurDrawLooper(SkFloatToScalar(0.002f),
                                          SkFloatToScalar(0.0f),
                                          SkFloatToScalar(0.0f),
                                          (SkColor)0xFF000000))->unref();

    const char* text = "DRAWING STROKED TEXT WITH A BLUR ON A PATH";
    size_t      len = strlen(text);

    canvas->drawTextOnPathHV(text, len, path, 0,
                             SkFloatToScalar(-0.025f), paint);
    canvas->restore();
}

static void textPathMatrix(SkCanvas* canvas) {
    SkPaint paint;
    SkPath  path;
    SkMatrix matrix;
    
    path.moveTo(SkIntToScalar(050), SkIntToScalar(200));
    path.quadTo(SkIntToScalar(250), SkIntToScalar(000),
                SkIntToScalar(450), SkIntToScalar(200));

    paint.setAntiAlias(true);
    
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);
    paint.setStyle(SkPaint::kFill_Style);
    paint.setTextSize(SkIntToScalar(48));
    paint.setTextAlign(SkPaint::kRight_Align);
    
    const char* text = "Reflection";
    size_t      len = strlen(text);

    SkPathMeasure   meas(path, false);
    SkScalar pathLen = meas.getLength();

    canvas->drawTextOnPath(text, len, path, NULL, paint);
    
    paint.setColor(SK_ColorRED);
    matrix.setScale(-SK_Scalar1, SK_Scalar1);
    matrix.postTranslate(pathLen, 0);
    canvas->drawTextOnPath(text, len, path, &matrix, paint);
    
    paint.setColor(SK_ColorBLUE);
    matrix.setScale(SK_Scalar1, -SK_Scalar1);
    canvas->drawTextOnPath(text, len, path, &matrix, paint);
    
    paint.setColor(SK_ColorGREEN);
    matrix.setScale(-SK_Scalar1, -SK_Scalar1);
    matrix.postTranslate(pathLen, 0);
    canvas->drawTextOnPath(text, len, path, &matrix, paint);
}

class TextOnPathView : public SampleView {
public:
    SkPath      fPath;
    SkScalar    fHOffset;

	TextOnPathView() {
        SkRect r;
        r.set(SkIntToScalar(100), SkIntToScalar(100),
              SkIntToScalar(300), SkIntToScalar(300));
        fPath.addOval(r);
        fPath.offset(SkIntToScalar(-50), SkIntToScalar(-50));

        fHOffset = SkIntToScalar(50);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Text On Path");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    virtual void onDrawContent(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(48));

        const char* text = "Hamburgefons";
        size_t      len = strlen(text);

        for (int j = 0; j < REPEAT_COUNT; j++) {
            SkScalar x = fHOffset;

            paint.setColor(SK_ColorBLACK);
            canvas->drawTextOnPathHV(text, len, fPath,
                                     x, paint.getTextSize()/2, paint);

            paint.setColor(SK_ColorRED);
            canvas->drawTextOnPathHV(text, len, fPath,
                                     x + SkIntToScalar(50), 0, paint);

            paint.setColor(SK_ColorBLUE);
            canvas->drawTextOnPathHV(text, len, fPath,
                         x + SkIntToScalar(100), -paint.getTextSize()/2, paint);
        }

        paint.setColor(SK_ColorGREEN);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawPath(fPath, paint);
        
        canvas->translate(SkIntToScalar(275), 0);
        textStrokePath(canvas);

        canvas->translate(SkIntToScalar(-275), SkIntToScalar(250));
        textPathMatrix(canvas);
        
        if (REPEAT_COUNT > 1)
            this->inval(NULL);
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        fHints += 1;
        this->inval(NULL);
        return this->INHERITED::onFindClickHandler(x, y);
    }
    
    virtual bool onClick(Click* click) {
        return this->INHERITED::onClick(click);
    }
    
private:
    int fHints;
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() {
    return new TextOnPathView;
}

static SkViewRegister reg(MyFactory);
