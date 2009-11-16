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
#include "SkTime.h"
#include "SkRandom.h"
#include "SkLineClipper.h"

enum {
    W = 640/4,
    H = 480/4
};

class LineClipperView : public SkView {
    SkRect      fClip;
    SkRandom    fRand;
    SkPoint     fPts[2];

    void randPts() {
        fPts[0].set(fRand.nextUScalar1() * 640, fRand.nextUScalar1() * 480);
        fPts[1].set(fRand.nextUScalar1() * 640, fRand.nextUScalar1() * 480);
    }

public:
	LineClipperView() {
        int x = (640 - W)/2;
        int y = (480 - H)/2;
        fClip.set(x, y, x + W, y + H);
        this->randPts();
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "LineClipper");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }
    
    static void drawVLine(SkCanvas* canvas, SkScalar x, const SkPaint& paint) {
        canvas->drawLine(x, -999, x, 999, paint);
    }
    
    static void drawHLine(SkCanvas* canvas, SkScalar y, const SkPaint& paint) {
        canvas->drawLine(-999, y, 999, y, paint);
    }
    
    static void check_lineclipper(int count, const SkPoint pts[],
                                  const SkRect& clip) {
        if (count > 0) {
            for (int i = 0; i <= count; i++) {
                SkASSERT(pts[i].fX >= clip.fLeft);
                SkASSERT(pts[i].fX <= clip.fRight);
                SkASSERT(pts[i].fY >= clip.fTop);
                SkASSERT(pts[i].fY <= clip.fBottom);
            }
        }
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        SkPaint paint;
        
        drawVLine(canvas, fClip.fLeft + SK_ScalarHalf, paint);
        drawVLine(canvas, fClip.fRight - SK_ScalarHalf, paint);
        drawHLine(canvas, fClip.fTop + SK_ScalarHalf, paint);
        drawHLine(canvas, fClip.fBottom - SK_ScalarHalf, paint);
        
        paint.setColor(SK_ColorLTGRAY);
        canvas->drawRect(fClip, paint);
        
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLUE);
        paint.setStrokeWidth(SkIntToScalar(3));
        paint.setStrokeCap(SkPaint::kRound_Cap);
        SkPoint pts[SkLineClipper::kMaxPoints];
        int count = SkLineClipper::ClipLine(fPts, fClip, pts);
        check_lineclipper(count, pts, fClip);
        for (int i = 0; i < count; i++) {
            canvas->drawPoints(SkCanvas::kLines_PointMode, 2, &pts[i], paint);
        }

        paint.setColor(SK_ColorRED);
        paint.setStrokeWidth(0);
        canvas->drawPoints(SkCanvas::kLines_PointMode, 2, fPts, paint);
        
        if (true) {
            this->randPts();
            this->inval(NULL);
        }
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        this->randPts();
        this->inval(NULL);
        return NULL;
    }
        
    virtual bool onClick(Click* click) {
        return false;
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new LineClipperView; }
static SkViewRegister reg(MyFactory);

