#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "Sk64.h"
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

static SkRandom gRand;

static const struct {
    const char* fName;
    uint32_t    fFlags;
    bool        fFlushCache;
} gHints[] = {
    { "Linear", SkPaint::kLinearText_Flag,     false },
    { "Normal",   0,                           true },
    { "Subpixel", SkPaint::kSubpixelText_Flag, true }
};

#ifdef SK_DEBUG
    #define REPEAT_COUNT    1
#else
    #define REPEAT_COUNT    5000
#endif

class PointsView : public SkView {
    bool fAA;
public:
	PointsView() : fAA(false) {}
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)
    {
        if (SampleCode::TitleQ(*evt))
        {
            SampleCode::TitleR(evt, "Points");
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
    
    static void fill_pts(SkPoint pts[], size_t n, SkRandom* rand)
    {
        for (size_t i = 0; i < n; i++)
            pts[i].set(rand->nextUScalar1() * 640, rand->nextUScalar1() * 480);
    }
    
    virtual void onDraw(SkCanvas* canvas)
    {
        this->drawBG(canvas);
        
        canvas->translate(SK_Scalar1, SK_Scalar1);
        
        SkRandom rand;
        SkPaint  p0, p1, p2, p3;
        const size_t n = 99;
        const int TIMES = 1;
        
        p0.setColor(SK_ColorRED);
        p1.setColor(SK_ColorGREEN);
        p2.setColor(SK_ColorBLUE);
        p3.setColor(SK_ColorWHITE);
        
     //   fAA = !fAA;
        
        p0.setAntiAlias(fAA);
        p1.setAntiAlias(fAA);
        p2.setAntiAlias(fAA);
        p3.setAntiAlias(fAA);
        
        p0.setStrokeWidth(SkIntToScalar(4));
        p2.setStrokeWidth(SkIntToScalar(6));

        SkPoint* pts = new SkPoint[n];
        fill_pts(pts, n, &rand);

//        SkMSec now = SkTime::GetMSecs();
        for (int times = 0; times < TIMES; times++)
        {
            canvas->drawPoints(SkCanvas::kPolygon_PointMode, n, pts, p0);
            canvas->drawPoints(SkCanvas::kLines_PointMode, n, pts, p1);
            canvas->drawPoints(SkCanvas::kPoints_PointMode, n, pts, p2);
            canvas->drawPoints(SkCanvas::kPoints_PointMode, n, pts, p3);
        }
  //      printf("----- msecs %d\n", SkTime::GetMSecs() - now);
        delete[] pts;
    }
    
private:

    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new PointsView; }
static SkViewRegister reg(MyFactory);

