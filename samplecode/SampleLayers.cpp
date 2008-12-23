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
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkUtils.h"
#include "SkKey.h"
#include "SkPorterDuff.h"
#include "SkXfermode.h"
#include "SkDrawFilter.h"

static void make_paint(SkPaint* paint) {
    SkColor colors[] = { 0, SK_ColorWHITE };
    SkPoint pts[] = { 0, 0, 0, SK_Scalar1*20 };
    SkShader* s = SkGradientShader::CreateLinear(pts, colors, NULL, 2, SkShader::kClamp_TileMode);
    
    paint->setShader(s)->unref();
    paint->setXfermode(SkPorterDuff::CreateXfermode(SkPorterDuff::kDstIn_Mode))->unref();
}

static void dump_layers(const char label[], SkCanvas* canvas) {
    SkDebugf("Dump Layers(%s)\n", label);

    SkCanvas::LayerIter iter(canvas, true);
    int index = 0;
    while (!iter.done()) {
        const SkBitmap& bm = iter.device()->accessBitmap(false);
        const SkIRect& clip = iter.clip().getBounds();
        SkDebugf("Layer[%d] bitmap [%d %d] X=%d Y=%d clip=[%d %d %d %d] alpha=%d\n", index++,
                 bm.width(), bm.height(), iter.x(), iter.y(),
                 clip.fLeft, clip.fTop, clip.fRight, clip.fBottom,
                 iter.paint().getAlpha());
        iter.next();
    }
}

// test drawing with strips of fading gradient above and below
static void test_fade(SkCanvas* canvas) {
    SkAutoCanvasRestore ar(canvas, true);

    SkRect r;
    
    SkPaint p;
    p.setAlpha(0x88);

    SkAutoCanvasRestore(canvas, false);

    // create the layers

    r.set(0, 0, SkIntToScalar(100), SkIntToScalar(100));
    canvas->clipRect(r);
    
    r.fBottom = SkIntToScalar(20);
    canvas->saveLayer(&r, NULL, (SkCanvas::SaveFlags)(SkCanvas::kHasAlphaLayer_SaveFlag | SkCanvas::kFullColorLayer_SaveFlag));

    r.fTop = SkIntToScalar(80);
    r.fBottom = SkIntToScalar(100);
    canvas->saveLayer(&r, NULL, (SkCanvas::SaveFlags)(SkCanvas::kHasAlphaLayer_SaveFlag | SkCanvas::kFullColorLayer_SaveFlag));
    
    // now draw the "content" 

    if (true) {
        r.set(0, 0, SkIntToScalar(100), SkIntToScalar(100));

        canvas->saveLayerAlpha(&r, 0x80);

        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setAntiAlias(true);
        canvas->drawOval(r, p);
        
        dump_layers("inside layer alpha", canvas);
        
        canvas->restore();
    } else {
        r.set(0, 0, SkIntToScalar(100), SkIntToScalar(100));
        
        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setAntiAlias(true);
        canvas->drawOval(r, p);
    }
    
//    return;

    dump_layers("outside layer alpha", canvas);

    // now apply an effect

    SkPaint paint;
    make_paint(&paint);
    r.set(0, 0, SkIntToScalar(100), SkIntToScalar(20));
//    SkDebugf("--------- draw top grad\n");
    canvas->drawRect(r, paint);

    SkMatrix m;
    SkShader* s = paint.getShader();
    m.setScale(SK_Scalar1, -SK_Scalar1);
    m.postTranslate(0, SkIntToScalar(100));
    s->setLocalMatrix(m);
    
    r.fTop = SkIntToScalar(80);
    r.fBottom = SkIntToScalar(100);
//    SkDebugf("--------- draw bot grad\n");
    canvas->drawRect(r, paint);
}

class RedFilter : public SkDrawFilter {
public:
    virtual bool filter(SkCanvas*, SkPaint* p, SkDrawFilter::Type) {
        fColor = p->getColor();
        if (fColor == SK_ColorRED) {
            p->setColor(SK_ColorGREEN);
        }
        return true;
    }
    virtual void restore(SkCanvas*, SkPaint* p, SkDrawFilter::Type) {
        p->setColor(fColor);
    }
    
private:
    SkColor fColor;
};

class LayersView : public SkView {
public:
	LayersView() {}

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Layers");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        if (false) {
            SkRect r;
			r.set(SkIntToScalar(0), SkIntToScalar(0),
				  SkIntToScalar(220), SkIntToScalar(120));
            SkPaint p;
            p.setAlpha(0x88);
            p.setAntiAlias(true);
            
            if (true) {
                canvas->saveLayer(&r, &p);
                p.setColor(0xFFFF0000);
                canvas->drawOval(r, p);
                canvas->restore();
            }

            p.setColor(0xFF0000FF);
            r.offset(SkIntToScalar(20), SkIntToScalar(50));
            canvas->drawOval(r, p);
        }

        if (false) {
            SkPaint p;
            p.setAlpha(0x88);
            p.setAntiAlias(true);

            canvas->translate(SkIntToScalar(300), 0);

            SkRect r;
			r.set(SkIntToScalar(0), SkIntToScalar(0),
				  SkIntToScalar(220), SkIntToScalar(60));

            canvas->saveLayer(&r, &p, (SkCanvas::SaveFlags)(SkCanvas::kHasAlphaLayer_SaveFlag | SkCanvas::kFullColorLayer_SaveFlag));
//            canvas->clipRect(r, SkRegion::kDifference_Op);
//            canvas->clipRect(r, SkRegion::kIntersect_Op);

			r.set(SkIntToScalar(0), SkIntToScalar(0),
				  SkIntToScalar(220), SkIntToScalar(120));
            p.setColor(SK_ColorBLUE);
            canvas->drawOval(r, p);
            canvas->restore();
            return;
        }
        
        //canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        test_fade(canvas);
        return;

    //    canvas->setDrawFilter(new RedFilter)->unref();
        
        SkRect  r;
        SkPaint p;
        
        canvas->translate(SkIntToScalar(220), SkIntToScalar(20));
        
        p.setAntiAlias(true);
        r.set(SkIntToScalar(20), SkIntToScalar(20),
              SkIntToScalar(220), SkIntToScalar(120));
        
        p.setColor(SK_ColorBLUE);
     //   p.setMaskFilter(SkBlurMaskFilter::Create(SkIntToScalar(8), SkBlurMaskFilter::kNormal_BlurStyle))->unref();
        canvas->drawRect(r, p);
        p.setMaskFilter(NULL);

        SkRect bounds = r;
        bounds.fBottom = bounds.centerY();
        canvas->saveLayer(&bounds, NULL, SkCanvas::kARGB_NoClipLayer_SaveFlag);

        p.setColor(SK_ColorRED);
        canvas->drawOval(r, p);
        
        p.setAlpha(0x80);
        p.setPorterDuffXfermode(SkPorterDuff::kDstIn_Mode);
        canvas->drawRect(bounds, p);

        canvas->restore();
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        this->inval(NULL);
        
        return this->INHERITED::onFindClickHandler(x, y);
    }
    
    virtual bool onClick(Click* click) {
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

static SkView* MyFactory() { return new LayersView; }
static SkViewRegister reg(MyFactory);

