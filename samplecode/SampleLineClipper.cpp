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
#include "SkQuadClipper.h"

static void drawQuad(SkCanvas* canvas, const SkPoint pts[3], const SkPaint& p) {
    SkPath path;
    path.moveTo(pts[0]);
    path.quadTo(pts[1], pts[2]);
    canvas->drawPath(path, p);
}

typedef void (*clipper_proc)(const SkPoint src[], const SkRect& clip,
                            SkCanvas*, const SkPaint&, const SkPaint&);

static void check_clipper(int count, const SkPoint pts[], const SkRect& clip) {
    for (int i = 0; i < count; i++) {
        SkASSERT(pts[i].fX >= clip.fLeft);
        SkASSERT(pts[i].fX <= clip.fRight);
        SkASSERT(pts[i].fY >= clip.fTop);
        SkASSERT(pts[i].fY <= clip.fBottom);
    }
}

static void line_clipper(const SkPoint src[], const SkRect& clip,
                       SkCanvas* canvas, const SkPaint& p0, const SkPaint& p1) {
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, src, p1);

    SkPoint dst[SkLineClipper::kMaxPoints];
    int count = SkLineClipper::ClipLine(src, clip, dst);
    for (int i = 0; i < count; i++) {
        check_clipper(2, &dst[i], clip);
        canvas->drawPoints(SkCanvas::kLines_PointMode, 2, &dst[i], p0);
    }
}

static void quad_clipper(const SkPoint src[], const SkRect& clip,
                       SkCanvas* canvas, const SkPaint& p0, const SkPaint& p1) {
    drawQuad(canvas, src, p1);

    SkQuadClipper2 clipper;
    if (clipper.clipQuad(src, clip)) {
        SkPoint pts[3];
        SkPath::Verb verb;
        while ((verb = clipper.next(pts)) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kLine_Verb:
                    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, pts, p0);
                    break;
                case SkPath::kQuad_Verb:
                    drawQuad(canvas, pts, p0);
                    break;
                default:
                    SkASSERT(!"unexpected verb");
            }
        }
    }
}

static const clipper_proc gProcs[] = {
    line_clipper,
    quad_clipper
};

///////////////////////////////////////////////////////////////////////////////

enum {
    W = 640/3,
    H = 480/3
};

class LineClipperView : public SkView {
    int         fProcIndex;
    SkRect      fClip;
    SkRandom    fRand;
    SkPoint     fPts[4];

    void randPts() {
        for (int i = 0; i < SK_ARRAY_COUNT(fPts); i++) {
            fPts[i].set(fRand.nextUScalar1() * 640,
                        fRand.nextUScalar1() * 480);
        }
    }

public:
	LineClipperView() {
        int x = (640 - W)/2;
        int y = (480 - H)/2;
        fClip.set(x, y, x + W, y + H);
        this->randPts();
        
        fProcIndex = 1;
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
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        SkPaint paint, paint1;
        
        drawVLine(canvas, fClip.fLeft + SK_ScalarHalf, paint);
        drawVLine(canvas, fClip.fRight - SK_ScalarHalf, paint);
        drawHLine(canvas, fClip.fTop + SK_ScalarHalf, paint);
        drawHLine(canvas, fClip.fBottom - SK_ScalarHalf, paint);
        
        paint.setColor(SK_ColorLTGRAY);
        canvas->drawRect(fClip, paint);
        
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLUE);
        paint.setStyle(SkPaint::kStroke_Style);
      //  paint.setStrokeWidth(SkIntToScalar(3));
        paint.setStrokeCap(SkPaint::kRound_Cap);
        
        paint1.setAntiAlias(true);
        paint1.setColor(SK_ColorRED);
        paint1.setStyle(SkPaint::kStroke_Style);
        gProcs[fProcIndex](fPts, fClip, canvas, paint, paint1);
        
        if (true) {
            this->randPts();
            this->inval(NULL);
        }
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
     //   fProcIndex = (fProcIndex + 1) % SK_ARRAY_COUNT(gProcs);
        if (x < 50 && y < 50) {
            this->randPts();
        }
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

