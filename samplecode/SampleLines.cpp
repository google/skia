#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "Sk64.h"
#include "SkCornerPathEffect.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkKernel33MaskFilter.h"
#include "SkPath.h"
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
#include "SkColorPriv.h"
#include "SkImageDecoder.h"

class LinesView : public SkView {
public:
	LinesView()
    {
        unsigned r = 0x1F;
        unsigned g = 0x3F;
        for (unsigned a = 0; a <= 0xF; a++) {
            unsigned scale = 16 - SkAlpha15To16(a);
            unsigned sr = (a << 1) | (a >> 3);
            unsigned dr = r * scale >> 4;
            unsigned sg = (a << 2) | (a >> 2);
            unsigned dg = g * scale >> 4;
            
            unsigned ssg = sg & ~(~(a >> 3) & 1);
            
            printf("4444 sa=%d sr=%d sg=%d da=%d dr=%d dg=%d total-r=%d total-g=%d %d\n",
                   a, sr, sg, scale, dr, dg, sr+dr, sg+dg, ssg+dg);
        }
        
        for (unsigned aa = 0; aa <= 0xFF; aa++) {
            unsigned invScale = SkAlpha255To256(255 - aa);
            unsigned dst = SkAlphaMul(0xFF, invScale);
            printf("8888 sa=%02x dst=%02x sum=%d %s\n", aa, dst, aa+dst,
                   (aa+dst) > 0xFF ? "OVERFLOW" : "");
        }
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)
    {
        if (SampleCode::TitleQ(*evt))
        {
            SampleCode::TitleR(evt, "Lines");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas)
    {
//        canvas->drawColor(0xFFDDDDDD);
        canvas->drawColor(SK_ColorWHITE);
   //     canvas->drawColor(SK_ColorBLACK);
    }
    
    /*
     0x1F * x + 0x1F * (32 - x)
     */
    void drawRings(SkCanvas* canvas)
    {
        canvas->scale(SkIntToScalar(1)/2, SkIntToScalar(1)/2);
        
        SkRect  r;        
        SkScalar x = SkIntToScalar(10);
        SkScalar y = SkIntToScalar(10);
        r.set(x, y, x + SkIntToScalar(100), y + SkIntToScalar(100));
        
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkScalarHalf(SkIntToScalar(3)));
        paint.setColor(0xFFFF8800);
        paint.setColor(0xFFFFFFFF);
        canvas->drawRect(r, paint);
    }
    
    virtual void onDraw(SkCanvas* canvas)
    {
        this->drawBG(canvas);
        
        SkBitmap bm;
        SkImageDecoder::DecodeFile("/kill.gif", &bm);
        canvas->drawBitmap(bm, 0, 0, NULL);
        
        this->drawRings(canvas);
        return;

        SkPaint paint;
        
      //  fAlpha = 0x80;
        paint.setColor(SK_ColorWHITE);
        paint.setAlpha(fAlpha & 0xFF);
        SkRect r;
        
        SkScalar x = SkIntToScalar(10);
        SkScalar y = SkIntToScalar(10);
        r.set(x, y, x + SkIntToScalar(100), y + SkIntToScalar(100));
        canvas->drawRect(r, paint);
        return;
        
        paint.setColor(0xffffff00);            // yellow
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(2));
        
//        y += SK_Scalar1/2;

        canvas->drawLine(x, y, x + SkIntToScalar(90), y + SkIntToScalar(90), paint);

        paint.setAntiAlias(true);              // with anti-aliasing
        y += SkIntToScalar(10);
        canvas->drawLine(x, y, x + SkIntToScalar(90), y + SkIntToScalar(90), paint);
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) 
    {
        fAlpha = SkScalarRound(y);
        this->inval(NULL);
        return NULL;
    }
private:

    int fAlpha;
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new LinesView; }
static SkViewRegister reg(MyFactory);

