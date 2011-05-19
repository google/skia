
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkParsePath.h"
#include "SkTime.h"
#include "SkTypeface.h"

#include "SkGeometry.h"

// http://code.google.com/p/skia/issues/detail?id=32
static void test_cubic() {
    SkPoint src[4] = {
        { 556.25000f, 523.03003f },
        { 556.23999f, 522.96002f },
        { 556.21997f, 522.89001f },
        { 556.21997f, 522.82001f }
    };
    SkPoint dst[11];
    dst[10].set(42, -42);   // one past the end, that we don't clobber these
    SkScalar tval[] = { 0.33333334f, 0.99999994f };

    SkChopCubicAt(src, dst, tval, 2);

#if 0
    for (int i = 0; i < 11; i++) {
        SkDebugf("--- %d [%g %g]\n", i, dst[i].fX, dst[i].fY);
    }
#endif
}

static void test_cubic2() {
    const char* str = "M2242 -590088L-377758 9.94099e+07L-377758 9.94099e+07L2242 -590088Z";
    SkPath path;
    SkParsePath::FromSVGString(str, &path);
    
    {
#ifdef SK_BUILD_FOR_WIN
        // windows doesn't have strtof
        float x = (float)strtod("9.94099e+07", NULL);
#else
        float x = strtof("9.94099e+07", NULL);
#endif
        int ix = (int)x;
        int fx = (int)(x * 65536);
        int ffx = SkScalarToFixed(x);
        printf("%g %x %x %x\n", x, ix, fx, ffx);
        
        SkRect r = path.getBounds();
        SkIRect ir;
        r.round(&ir);
        printf("[%g %g %g %g] [%x %x %x %x]\n",
               r.fLeft, r.fTop, r.fRight, r.fBottom,
               ir.fLeft, ir.fTop, ir.fRight, ir.fBottom);
    }
    
    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, 300, 200);
    bitmap.allocPixels();

    SkCanvas canvas(bitmap);
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas.drawPath(path, paint);
}

class PathView : public SampleView {
public:
    int fDStroke, fStroke, fMinStroke, fMaxStroke;
    SkPath fPath[6];
    bool fShowHairline;
    
	PathView() {
        test_cubic();
        test_cubic2();

        fShowHairline = false;
        
        fDStroke = 1;
        fStroke = 10;
        fMinStroke = 10;
        fMaxStroke = 180;

        const int V = 85;
        
        fPath[0].moveTo(SkIntToScalar(40), SkIntToScalar(70));
        fPath[0].lineTo(SkIntToScalar(70), SkIntToScalar(70) + SK_Scalar1/1);
        fPath[0].lineTo(SkIntToScalar(110), SkIntToScalar(70));
        
        fPath[1].moveTo(SkIntToScalar(40), SkIntToScalar(70));
        fPath[1].lineTo(SkIntToScalar(70), SkIntToScalar(70) - SK_Scalar1/1);
        fPath[1].lineTo(SkIntToScalar(110), SkIntToScalar(70));
        
        fPath[2].moveTo(SkIntToScalar(V), SkIntToScalar(V));
        fPath[2].lineTo(SkIntToScalar(50), SkIntToScalar(V));
        fPath[2].lineTo(SkIntToScalar(50), SkIntToScalar(50));
        
        fPath[3].moveTo(SkIntToScalar(50), SkIntToScalar(50));
        fPath[3].lineTo(SkIntToScalar(50), SkIntToScalar(V));
        fPath[3].lineTo(SkIntToScalar(V), SkIntToScalar(V));
        
        fPath[4].moveTo(SkIntToScalar(50), SkIntToScalar(50));
        fPath[4].lineTo(SkIntToScalar(50), SkIntToScalar(V));
        fPath[4].lineTo(SkIntToScalar(52), SkIntToScalar(50));
        
        fPath[5].moveTo(SkIntToScalar(52), SkIntToScalar(50));
        fPath[5].lineTo(SkIntToScalar(50), SkIntToScalar(V));
        fPath[5].lineTo(SkIntToScalar(50), SkIntToScalar(50));
        
        this->setBGColor(0xFFDDDDDD);
    }
    
    void nextStroke() {
        fStroke += fDStroke;
        if (fStroke > fMaxStroke || fStroke < fMinStroke)
            fDStroke = -fDStroke;
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Paths");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawPath(SkCanvas* canvas, const SkPath& path, SkPaint::Join j) {
        SkPaint paint;
        
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeJoin(j);
        paint.setStrokeWidth(SkIntToScalar(fStroke));

        if (fShowHairline) {
            SkPath  fill;
            
            paint.getFillPath(path, &fill);            
            paint.setStrokeWidth(0);
            canvas->drawPath(fill, paint);
        } else {
            canvas->drawPath(path, paint);
        }
        
        paint.setColor(SK_ColorRED);
        paint.setStrokeWidth(0);
        canvas->drawPath(path, paint);
    }
    
    virtual void onDrawContent(SkCanvas* canvas) {        
        canvas->translate(SkIntToScalar(50), SkIntToScalar(50));

        static const SkPaint::Join gJoins[] = {
            SkPaint::kBevel_Join,
            SkPaint::kMiter_Join,
            SkPaint::kRound_Join
        };

        for (size_t i = 0; i < SK_ARRAY_COUNT(gJoins); i++) {
            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(fPath); j++) {
                this->drawPath(canvas, fPath[j], gJoins[i]);
                canvas->translate(SkIntToScalar(200), 0);
            }
            canvas->restore();
            
            canvas->translate(0, SkIntToScalar(200));
        }
        
        this->nextStroke();
        this->inval(NULL);
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        fShowHairline = !fShowHairline;
        this->inval(NULL);
        return this->INHERITED::onFindClickHandler(x, y);
    }
    
private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new PathView; }
static SkViewRegister reg(MyFactory);

