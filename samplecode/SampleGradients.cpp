#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"

static SkShader* setgrad(const SkRect& r, SkColor c0, SkColor c1) {
    SkColor colors[] = { c0, c1 };
    SkPoint pts[] = { { r.fLeft, r.fTop }, { r.fRight, r.fTop } };
    return SkGradientShader::CreateLinear(pts, colors, NULL, 2,
                                          SkShader::kClamp_TileMode, NULL);
}

static void test_alphagradients(SkCanvas* canvas) {
    SkRect r;
    r.set(SkIntToScalar(10), SkIntToScalar(10),
          SkIntToScalar(410), SkIntToScalar(30));
    SkPaint p, p2;
    p2.setStyle(SkPaint::kStroke_Style);
    
    p.setShader(setgrad(r, 0xFF00FF00, 0x0000FF00))->unref();
    canvas->drawRect(r, p);
    canvas->drawRect(r, p2);
    
    r.offset(0, r.height() + SkIntToScalar(4));
    p.setShader(setgrad(r, 0xFF00FF00, 0x00000000))->unref();
    canvas->drawRect(r, p);
    canvas->drawRect(r, p2);
    
    r.offset(0, r.height() + SkIntToScalar(4));
    p.setShader(setgrad(r, 0xFF00FF00, 0x00FF0000))->unref();
    canvas->drawRect(r, p);
    canvas->drawRect(r, p2);
}

///////////////////////////////////////////////////////////////////////////////

struct GradData {
    int             fCount;
    const SkColor*  fColors;
    const SkScalar* fPos;
};

static const SkColor gColors[] = {
    SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK
};
static const SkScalar gPos0[] = { 0, SK_Scalar1 };
static const SkScalar gPos1[] = { SK_Scalar1/4, SK_Scalar1*3/4 };
static const SkScalar gPos2[] = {
    0, SK_Scalar1/8, SK_Scalar1/2, SK_Scalar1*7/8, SK_Scalar1
};

static const GradData gGradData[] = {
    { 2, gColors, NULL },
    { 2, gColors, gPos0 },
    { 2, gColors, gPos1 },
    { 5, gColors, NULL },
    { 5, gColors, gPos2 }
};

static SkShader* MakeLinear(const SkPoint pts[2], const GradData& data,
                            SkShader::TileMode tm, SkUnitMapper* mapper) {
    return SkGradientShader::CreateLinear(pts, data.fColors, data.fPos,
                                          data.fCount, tm, mapper);
}
                                          
static SkShader* MakeRadial(const SkPoint pts[2], const GradData& data,
                            SkShader::TileMode tm, SkUnitMapper* mapper) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::CreateRadial(center, center.fX, data.fColors,
                                          data.fPos, data.fCount, tm, mapper);
}

static SkShader* MakeSweep(const SkPoint pts[2], const GradData& data,
                           SkShader::TileMode tm, SkUnitMapper* mapper) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::CreateSweep(center.fX, center.fY, data.fColors,
                                         data.fPos, data.fCount, mapper);
}

typedef SkShader* (*GradMaker)(const SkPoint pts[2], const GradData& data,
                     SkShader::TileMode tm, SkUnitMapper* mapper);
static const GradMaker gGradMakers[] = {
    MakeLinear, MakeRadial, MakeSweep
};

///////////////////////////////////////////////////////////////////////////////

class GradientsView : public SkView {
public:
	GradientsView() {}
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Gradients");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        SkPoint pts[2] = {
            { 0, 0 },
            { SkIntToScalar(100), SkIntToScalar(100) }
        };
        SkShader::TileMode tm = SkShader::kClamp_TileMode;
        SkRect r = { 0, 0, SkIntToScalar(100), SkIntToScalar(100) };
        SkPaint paint;
        paint.setAntiAlias(true);

        canvas->save();
        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        for (size_t i = 0; i < SK_ARRAY_COUNT(gGradData); i++) {
            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(gGradMakers); j++) {
                SkShader* shader = gGradMakers[j](pts, gGradData[i], tm, NULL);
                paint.setShader(shader);
                canvas->drawRect(r, paint);
                shader->unref();
                canvas->translate(0, SkIntToScalar(120));
            }
            canvas->restore();
            canvas->translate(SkIntToScalar(120), 0);
        }
        canvas->restore();
        
        canvas->translate(0, SkIntToScalar(370));
        test_alphagradients(canvas);
    }
    
private:
    typedef SkView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new GradientsView; }
static SkViewRegister reg(MyFactory);

