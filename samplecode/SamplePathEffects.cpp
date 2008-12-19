#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "Sk1DPathEffect.h"
#include "SkCornerPathEffect.h"
#include "SkPathMeasure.h"
#include "SkRandom.h"
#include "SkColorPriv.h"
#include "SkPixelXorXfermode.h"

static void test_grow(SkPath* path)
{
    for (int i = 0; i < 100000; i++)
    {
        path->lineTo(i, i);
        path->lineTo(i, i*2);
    }
}

#define CORNER_RADIUS   12
static SkScalar gPhase;

static const int gXY[] = {
    4, 0, 0, -4, 8, -4, 12, 0, 8, 4, 0, 4
};

static SkPathEffect* make_pe(int flags)
{
    if (flags == 1)
        return new SkCornerPathEffect(SkIntToScalar(CORNER_RADIUS));

    SkPath  path;
    path.moveTo(SkIntToScalar(gXY[0]), SkIntToScalar(gXY[1]));
    for (unsigned i = 2; i < SK_ARRAY_COUNT(gXY); i += 2)
        path.lineTo(SkIntToScalar(gXY[i]), SkIntToScalar(gXY[i+1]));
    path.close();
    path.offset(SkIntToScalar(-6), 0);

    SkPathEffect* outer = new SkPath1DPathEffect(path, SkIntToScalar(12), gPhase, SkPath1DPathEffect::kRotate_Style);
    
    if (flags == 2)
        return outer;

    SkPathEffect* inner = new SkCornerPathEffect(SkIntToScalar(CORNER_RADIUS));

    SkPathEffect* pe = new SkComposePathEffect(outer, inner);
    outer->unref();
    inner->unref();
    return pe;
}

static SkPathEffect* make_warp_pe()
{
    SkPath  path;
    path.moveTo(SkIntToScalar(gXY[0]), SkIntToScalar(gXY[1]));
    for (unsigned i = 2; i < SK_ARRAY_COUNT(gXY); i += 2)
        path.lineTo(SkIntToScalar(gXY[i]), SkIntToScalar(gXY[i+1]));
    path.close();
    path.offset(SkIntToScalar(-6), 0);

    SkPathEffect* outer = new SkPath1DPathEffect(path, SkIntToScalar(12), gPhase, SkPath1DPathEffect::kMorph_Style);
    SkPathEffect* inner = new SkCornerPathEffect(SkIntToScalar(CORNER_RADIUS));

    SkPathEffect* pe = new SkComposePathEffect(outer, inner);
    outer->unref();
    inner->unref();
    return pe;
}

///////////////////////////////////////////////////////////

#include "SkColorFilter.h"
#include "SkPorterDuff.h"
#include "SkLayerRasterizer.h"

class testrast : public SkLayerRasterizer {
public:
    testrast()
    {
        SkPaint paint;
        paint.setAntiAlias(true);

#if 0        
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SK_Scalar1*4);
        this->addLayer(paint);
    
        paint.setStrokeWidth(SK_Scalar1*1);
        paint.setPorterDuffXfermode(SkPorterDuff::kClear_Mode);
        this->addLayer(paint);
#else
        paint.setAlpha(0x66);
        this->addLayer(paint, SkIntToScalar(4), SkIntToScalar(4));
    
        paint.setAlpha(0xFF);
        this->addLayer(paint);
#endif
    }
};

class PathEffectView : public SkView {
    SkPath  fPath;
    SkPoint fClickPt;
public:
	PathEffectView()
    {
        SkRandom    rand;
        int         steps = 20;
        SkScalar    dist = SkIntToScalar(500);
        SkScalar    x = SkIntToScalar(20);
        SkScalar    y = SkIntToScalar(50);
        
        fPath.moveTo(x, y);
        for (int i = 0; i < steps; i++)
        {
            x += dist/steps;
            fPath.lineTo(x, y + SkIntToScalar(rand.nextS() % 25));
        }

        fClickPt.set(SkIntToScalar(200), SkIntToScalar(200));
    }
	
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)
    {
            if (SampleCode::TitleQ(*evt))
            {
                SampleCode::TitleR(evt, "PathEffects");
                return true;
            }
            return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas)
    {
        canvas->drawColor(0xFFDDDDDD);

#if 0
        SkPath path;
        test_grow(&path);
        SkPaint p;
        
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(SK_Scalar1);
        canvas->drawPath(path, p);
        path.close();
#endif
    }
    
    virtual void onDraw(SkCanvas* canvas)
    {
        this->drawBG(canvas);
        
        if (true)
        {
            canvas->drawColor(SK_ColorWHITE);
            
            SkPixelXorXfermode  mode(SK_ColorWHITE);
            SkPaint             paint;
            
            paint.setColor(SK_ColorRED);
            paint.setXfermode(&mode);
            paint.setStrokeWidth(SkIntToScalar(8));
            
            canvas->drawLine(SkIntToScalar(100), SkIntToScalar(100),
                             SkIntToScalar(200), SkIntToScalar(200), paint);
            canvas->drawLine(SkIntToScalar(100), SkIntToScalar(200),
                             SkIntToScalar(200), SkIntToScalar(100), paint);
         //   return;
        }
        
        if (false)
        {
            SkPath  path;
            SkPoint pts[] = { SkIntToScalar(100), SkIntToScalar(100),
                              SkIntToScalar(200), SkIntToScalar(100),
                              SkIntToScalar(100), SkIntToScalar(200)
                            };
            SkPaint paint;
            
            pts[2] = fClickPt;

            paint.setAntiAlias(true);
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(SkIntToScalar(5));
            
            path.moveTo(pts[0]);
            path.arcTo(pts[1], pts[2], SkIntToScalar(50));
            canvas->drawPath(path, paint);
            
            paint.setStrokeWidth(0);
            paint.setColor(SK_ColorRED);
            canvas->drawLine(pts[0].fX, pts[0].fY, pts[1].fX, pts[1].fY, paint);
            canvas->drawLine(pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY, paint);
            return;
        }
        
        gPhase -= SK_Scalar1;
        this->inval(nil);
        
        SkPaint paint;
        
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(5));
        canvas->drawPath(fPath, paint);
        paint.setStrokeWidth(0);
        
        paint.setColor(SK_ColorRED);
        paint.setPathEffect(make_pe(1))->unref();
        canvas->drawPath(fPath, paint);
        
        canvas->translate(0, SkIntToScalar(50));
        
        paint.setColor(SK_ColorBLUE);
        paint.setPathEffect(make_pe(2))->unref();
        canvas->drawPath(fPath, paint);
        
        canvas->translate(0, SkIntToScalar(50));
        
        paint.setARGB(0xFF, 0, 0xBB, 0);
        paint.setPathEffect(make_pe(3))->unref();
        canvas->drawPath(fPath, paint);
        
        canvas->translate(0, SkIntToScalar(50));

        paint.setARGB(0xFF, 0, 0, 0);
        paint.setPathEffect(make_warp_pe())->unref();
        paint.setRasterizer(new testrast)->unref();
        canvas->drawPath(fPath, paint);
        
        {
            SkRect  oval;
            
            oval.set(SkIntToScalar(50), SkIntToScalar(100),
                     SkIntToScalar(150), SkIntToScalar(150));
            canvas->drawRoundRect(oval, SkIntToScalar(8), SkIntToScalar(8), paint);
        }
        
        {
            SkRect  bounds;
            SkPaint paint;
            
            paint.setAntiAlias(true);
            paint.setAlpha(0x80);
            paint.setColorFilter(
                SkColorFilter::CreatePorterDuffFilter(
                    SkColorSetARGB(0x44, 0, 0xFF, 0), SkPorterDuff::kSrcATop_Mode))->unref();
            
            bounds.set(SkIntToScalar(10), SkIntToScalar(10), SkIntToScalar(150), SkIntToScalar(70));
            canvas->saveLayer(&bounds, &paint,
                              (SkCanvas::SaveFlags)(SkCanvas::kHasAlphaLayer_SaveFlag | SkCanvas::kFullColorLayer_SaveFlag));
            
            paint.setColorFilter(NULL);
            paint.setColor(SK_ColorRED);
            canvas->drawOval(bounds, paint);

            paint.setColor(SK_ColorBLUE);
            paint.setAlpha(0x80);
            bounds.inset(SkIntToScalar(10), SkIntToScalar(10));
            canvas->drawOval(bounds, paint);
            
            canvas->restore();
        }
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new PathEffectView; }
static SkViewRegister reg(MyFactory);

