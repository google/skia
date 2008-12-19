#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkComposeShader.h"
#include "Sk1DPathEffect.h"
#include "SkCornerPathEffect.h"
#include "SkPathMeasure.h"
#include "SkRandom.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkPorterDuff.h"
#include "SkLayerRasterizer.h"

class ArcsView : public SkView {
public:
	ArcsView()
    {
        fSweep = SkIntToScalar(100);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)
    {
        if (SampleCode::TitleQ(*evt))
        {
            SampleCode::TitleR(evt, "Arcs");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas)
    {
        canvas->drawColor(0xFFDDDDDD);
    }
    
    static void drawRectWithLines(SkCanvas* canvas, const SkRect& r, const SkPaint& p)
    {
        canvas->drawRect(r, p);
        canvas->drawLine(r.fLeft, r.fTop, r.fRight, r.fBottom, p);
        canvas->drawLine(r.fLeft, r.fBottom, r.fRight, r.fTop, p);
        canvas->drawLine(r.fLeft, r.centerY(), r.fRight, r.centerY(), p);
        canvas->drawLine(r.centerX(), r.fTop, r.centerX(), r.fBottom, p);
    }
    
    static void draw_label(SkCanvas* canvas, const SkRect& rect,
                            int start, int sweep)
    {
        SkPaint paint;
        
        paint.setAntiAlias(true);
        paint.setTextAlign(SkPaint::kCenter_Align);
        
        SkString    str;
        
        str.appendS32(start);
        str.append(", ");
        str.appendS32(sweep);
        canvas->drawText(str.c_str(), str.size(), rect.centerX(),
                         rect.fBottom + paint.getTextSize() * 5/4, paint);
    }
    
    static void drawArcs(SkCanvas* canvas)
    {
        SkPaint paint;
        SkRect  r;
        SkScalar w = SkIntToScalar(75);
        SkScalar h = SkIntToScalar(50);

        r.set(0, 0, w, h);
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        
        canvas->save();
        canvas->translate(SkIntToScalar(10), SkIntToScalar(300));
        
        paint.setStrokeWidth(SkIntToScalar(1));
        
        static const int gAngles[] = {
            0, 360,
            0, 45,
            0, -45,
            720, 135,
            -90, 269,
            -90, 270,
            -90, 271,
            -180, -270,
            225, 90
        };
        
        for (int i = 0; i < SK_ARRAY_COUNT(gAngles); i += 2)
        {
            paint.setColor(SK_ColorBLACK);
            drawRectWithLines(canvas, r, paint);

            paint.setColor(SK_ColorRED);
            canvas->drawArc(r, SkIntToScalar(gAngles[i]),
                            SkIntToScalar(gAngles[i+1]), false, paint);
            
            draw_label(canvas, r, gAngles[i], gAngles[i+1]);

            canvas->translate(w * 8 / 7, 0);
        }
        
        canvas->restore();
    }
    
    virtual void onDraw(SkCanvas* canvas)
    {
        this->drawBG(canvas);

        SkRect  r;
        SkPaint paint;
        
        paint.setAntiAlias(true);
        paint.setStrokeWidth(SkIntToScalar(2));
        paint.setStyle(SkPaint::kStroke_Style);
        
        r.set(0, 0, SkIntToScalar(200), SkIntToScalar(200));
        r.offset(SkIntToScalar(20), SkIntToScalar(20));
        
        if (false) {
            const SkScalar d = SkIntToScalar(3);
            const SkScalar rad[] = { d, d, d, d, d, d, d, d };
            SkPath path;
            path.addRoundRect(r, rad);
            canvas->drawPath(path, paint);
            return;
        }

        drawRectWithLines(canvas, r, paint);
        
   //     printf("----- sweep %g %X\n", SkScalarToFloat(fSweep), SkDegreesToRadians(fSweep));
        
        
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(0x800000FF);
        canvas->drawArc(r, 0, fSweep, true, paint);

        paint.setColor(0x800FF000);
        canvas->drawArc(r, 0, fSweep, false, paint);

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorRED);
        canvas->drawArc(r, 0, fSweep, true, paint);
        
        paint.setStrokeWidth(0);
        paint.setColor(SK_ColorBLUE);
        canvas->drawArc(r, 0, fSweep, false, paint);
        
        fSweep += SK_Scalar1/4;
        if (fSweep > SkIntToScalar(360))
            fSweep = 0;
        
        drawArcs(canvas);
        this->inval(NULL);
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) 
    {
     //   fSweep += SK_Scalar1;
        this->inval(NULL);
        return this->INHERITED::onFindClickHandler(x, y);
    }
    
    virtual bool onClick(Click* click) 
    {
        return this->INHERITED::onClick(click);
    }
    
private:
    SkScalar fSweep;

    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ArcsView; }
static SkViewRegister reg(MyFactory);

