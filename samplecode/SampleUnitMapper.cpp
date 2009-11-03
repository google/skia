#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkPaint.h"
#include "SkUnitMappers.h"
#include "SkCubicInterval.h"

class UnitMapperView : public SkView {
    SkPoint fPts[4];
    SkMatrix fMatrix;
public:
    UnitMapperView() {
        fPts[0].set(0, 0);
        fPts[1].set(SK_Scalar1 / 3, SK_Scalar1 / 3);
        fPts[2].set(SK_Scalar1 * 2 / 3, SK_Scalar1 * 2 / 3);
        fPts[3].set(SK_Scalar1, SK_Scalar1);

        fMatrix.setScale(SK_Scalar1 * 200, -SK_Scalar1 * 200);
        fMatrix.postTranslate(SkIntToScalar(100), SkIntToScalar(300));
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "UnitMapper");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xFF8888FF);

        SkRect r = { 0, 0, SK_Scalar1, SK_Scalar1 };
        
        canvas->concat(fMatrix);
        canvas->drawRect(r, paint);

        paint.setColor(SK_ColorBLACK);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(0);
        paint.setStrokeCap(SkPaint::kRound_Cap);
        
        SkPath path;
        path.moveTo(fPts[0]);
        path.cubicTo(fPts[1], fPts[2], fPts[3]);
        canvas->drawPath(path, paint);

        paint.setColor(SK_ColorRED);
        paint.setStrokeWidth(0);
        canvas->drawLine(0, 0, SK_Scalar1, SK_Scalar1, paint);

        paint.setColor(SK_ColorBLUE);
        paint.setStrokeWidth(SK_Scalar1 / 60);
        for (int i = 0; i < 50; i++) {
            SkScalar x = i * SK_Scalar1 / 49;
            canvas->drawPoint(x, SkEvalCubicInterval(&fPts[1], x), paint);
        }

        paint.setStrokeWidth(SK_Scalar1 / 20);
        paint.setColor(SK_ColorGREEN);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, &fPts[1], paint);
    }

    SkPoint invertPt(SkScalar x, SkScalar y) {
        SkPoint pt;
        SkMatrix m;
        fMatrix.invert(&m);
        m.mapXY(x, y, &pt);
        return pt;
    }

    SkPoint* hittest(SkScalar x, SkScalar y) {
        SkPoint target = { x, y };
        SkPoint pts[2] = { fPts[1], fPts[2] };
        fMatrix.mapPoints(pts, 2);
        for (int i = 0; i < 2; i++) {
            if (SkPoint::Distance(pts[i], target) < SkIntToScalar(4)) {
                return &fPts[i + 1];
            }
        }
        return NULL;
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        fDragPt = hittest(x, y);
        return fDragPt ? new Click(this) : NULL;
    }
    
    virtual bool onClick(Click* click) {
        if (fDragPt) {
            *fDragPt = invertPt(click->fCurr.fX, click->fCurr.fY);
            this->inval(NULL);
            return true;
        }
        return false;
    }
    
private:
    SkPoint* fDragPt;

    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new UnitMapperView; }
static SkViewRegister reg(MyFactory);

