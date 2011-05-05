#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkPaint.h"
#include "SkUnitMappers.h"
#include "SkCubicInterval.h"

#include "SkWidgetViews.h"

static SkStaticTextView* make_textview(SkView* parent,
                                       const SkRect& bounds,
                                       const SkPaint& paint) {
    SkStaticTextView* view = new SkStaticTextView;
    view->setMode(SkStaticTextView::kFixedSize_Mode);
    view->setPaint(paint);
    view->setVisibleP(true);
    view->setSize(bounds.width(), bounds.height());
    view->setLoc(bounds.fLeft, bounds.fTop);
    parent->attachChildToFront(view)->unref();
    return view;
}

static void set_scalar(SkStaticTextView* view, SkScalar value) {
    SkString str;
    str.appendScalar(value);
    view->setText(str);
}

class UnitMapperView : public SampleView {
    SkPoint fPts[4];
    SkMatrix fMatrix;
    SkStaticTextView* fViews[4];

    void setViews() {
        set_scalar(fViews[0], fPts[1].fX);
        set_scalar(fViews[1], fPts[1].fY);
        set_scalar(fViews[2], fPts[2].fX);
        set_scalar(fViews[3], fPts[2].fY);
    }

public:
    UnitMapperView() {
        fPts[0].set(0, 0);
        fPts[1].set(SK_Scalar1 / 3, SK_Scalar1 / 3);
        fPts[2].set(SK_Scalar1 * 2 / 3, SK_Scalar1 * 2 / 3);
        fPts[3].set(SK_Scalar1, SK_Scalar1);

        fMatrix.setScale(SK_Scalar1 * 200, -SK_Scalar1 * 200);
        fMatrix.postTranslate(SkIntToScalar(100), SkIntToScalar(300));

        SkRect r = {
            SkIntToScalar(350), SkIntToScalar(100),
            SkIntToScalar(500), SkIntToScalar(130)
        };
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(25));
        for (int i = 0; i < 4; i++) {
            fViews[i] = make_textview(this, r, paint);
            r.offset(0, r.height());
        }
        this->setViews();
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
    
    virtual void onDrawContent(SkCanvas* canvas) {
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

    int hittest(SkScalar x, SkScalar y) {
        SkPoint target = { x, y };
        SkPoint pts[2] = { fPts[1], fPts[2] };
        fMatrix.mapPoints(pts, 2);
        for (int i = 0; i < 2; i++) {
            if (SkPoint::Distance(pts[i], target) < SkIntToScalar(4)) {
                return i + 1;
            }
        }
        return -1;
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        fDragIndex = hittest(x, y);
        return fDragIndex >= 0 ? new Click(this) : NULL;
    }
    
    virtual bool onClick(Click* click) {
        if (fDragIndex >= 0) {
            fPts[fDragIndex] = invertPt(click->fCurr.fX, click->fCurr.fY);
            this->setViews();
            this->inval(NULL);
            return true;
        }
        return false;
    }
    
private:
    int fDragIndex;

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new UnitMapperView; }
static SkViewRegister reg(MyFactory);

