#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkRandom.h"

class Draw : public SkRefCnt {
public:
    Draw() : fFlags(0) {}

    enum Flags {
        kSelected_Flag  = 1 << 0
    };
    int getFlags() const { return fFlags; }
    void setFlags(int flags);

    bool isSelected() const { return SkToBool(fFlags & kSelected_Flag); }
    void setSelected(bool pred) {
        if (pred) {
            fFlags |= kSelected_Flag;
        } else {
            fFlags &= ~kSelected_Flag;
        }
    }

    void draw(SkCanvas* canvas) {
        int sc = canvas->save();
        this->onDraw(canvas);
        canvas->restoreToCount(sc);

        if (this->isSelected()) {
            this->drawSelection(canvas);
        }
    }

    void drawSelection(SkCanvas* canvas) {
        int sc = canvas->save();
        this->onDrawSelection(canvas);
        canvas->restoreToCount(sc);
    }

    void getBounds(SkRect* bounds) {
        this->onGetBounds(bounds);
    }

    bool hitTest(SkScalar x, SkScalar y) {
        return this->onHitTest(x, y);
    }

    void offset(SkScalar dx, SkScalar dy) {
        if (dx || dy) {
            this->onOffset(dx, dy);
        }
    }

protected:
    virtual void onDraw(SkCanvas*) = 0;
    virtual void onGetBounds(SkRect*) = 0;
    virtual void onOffset(SkScalar dx, SkScalar dy) = 0;
    virtual void onDrawSelection(SkCanvas* canvas) {
        SkRect r;
        this->getBounds(&r);
        SkPaint paint;
        SkPoint pts[4];
        r.toQuad(pts);
        paint.setStrokeWidth(SkIntToScalar(10));
        paint.setColor(0x80FF8844);
        paint.setStrokeCap(SkPaint::kRound_Cap);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, 4, pts, paint);
    }
    virtual bool onHitTest(SkScalar x, SkScalar y) {
        SkRect bounds;
        this->getBounds(&bounds);
        return bounds.contains(x, y);
    }

private:
    int fFlags;
};

class RDraw : public Draw {
public:
    enum Style {
        kRect_Style,
        kOval_Style,
        kRRect_Style
    };

    RDraw(const SkRect& r, Style s) : fRect(r), fStyle(s) {}

    void setRect(const SkRect& r) {
        fRect = r;
    }

    void setPaint(const SkPaint& p) {
        fPaint = p;
    }

protected:
    virtual void onDraw(SkCanvas* canvas) {
        switch (fStyle) {
            case kRect_Style:
                canvas->drawRect(fRect, fPaint);
                break;
            case kOval_Style:
                canvas->drawOval(fRect, fPaint);
                break;
            case kRRect_Style: {
                SkScalar rx = fRect.width() / 5;
                SkScalar ry = fRect.height() / 5;
                if (rx < ry) {
                    ry = rx;
                } else {
                    rx = ry;
                }
                canvas->drawRoundRect(fRect, rx, ry, fPaint);
                break;
            }
        }
    }

    virtual void onGetBounds(SkRect* bounds) {
        *bounds = fRect;
    }

    virtual void onOffset(SkScalar dx, SkScalar dy) {
        fRect.offset(dx, dy);
    }

private:
    SkRect  fRect;
    SkPaint fPaint;
    Style   fStyle;
};

class DrawFactory {
public:
    DrawFactory() {
        fPaint.setAntiAlias(true);
    }

    const SkPaint& getPaint() const { return fPaint; }

    void setPaint(const SkPaint& p) {
        fPaint = p;
    }

    virtual Draw* create(const SkPoint&, const SkPoint&) = 0;
    
private:
    SkPaint fPaint;
};

class RectFactory : public DrawFactory {
public:
    virtual Draw* create(const SkPoint& p0, const SkPoint& p1) {
        SkRect r;
        r.set(p0.x(), p0.y(), p1.x(), p1.y());
        r.sort();

        RDraw* d = new RDraw(r, RDraw::kRRect_Style);
        d->setPaint(this->getPaint());
        return d;
    }
};

class DrawView : public SkView {
    Draw*           fDraw;
    DrawFactory*    fFactory;
    SkRandom        fRand;
    SkTDArray<Draw*> fList;

public:
    DrawView() : fDraw(NULL) {
        fFactory = new RectFactory;
    }

    virtual ~DrawView() {
        fList.unrefAll();
        SkSafeUnref(fDraw);
    }

    Draw* setDraw(Draw* d) {
        SkRefCnt_SafeAssign(fDraw, d);
        return d;
    }

    SkColor randColor() {
        return (SkColor)fRand.nextU() | 0xFF000000;
    }

    Draw* hitTestList(SkScalar x, SkScalar y) const {
        Draw** first = fList.begin();
        for (Draw** iter = fList.end(); iter > first;) {
            --iter;
            if ((*iter)->hitTest(x, y)) {
                return *iter;
            }
        }
        return NULL;
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Draw");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
//        canvas->drawColor(SK_ColorWHITE);
    }

    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        for (Draw** iter = fList.begin(); iter < fList.end(); iter++) {
            (*iter)->draw(canvas);
        }
        if (fDraw) {
            fDraw->draw(canvas);
        }
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        for (Draw** iter = fList.begin(); iter < fList.end(); iter++) {
            (*iter)->setSelected(false);
        }

        Click* c = new Click(this);
        Draw* d = this->hitTestList(x, y);
        if (d) {
            d->setSelected(true);
            c->setType("dragger");
        } else {
            c->setType("maker");
        }
        return c;
    }

    virtual bool onClick(Click* click) {
        if (Click::kUp_State == click->fState) {
            if (click->isType("maker")) {
                if (SkPoint::Distance(click->fOrig, click->fCurr) > SkIntToScalar(3)) {
                    *fList.append() = fDraw;
                } else {
                    fDraw->unref();
                }
                fDraw = NULL;
            }
            return true;
        }

        if (Click::kDown_State == click->fState) {
            SkPaint p = fFactory->getPaint();
            p.setColor(this->randColor());
            fFactory->setPaint(p);
        }

        if (click->isType("maker")) {
            this->setDraw(fFactory->create(click->fOrig, click->fCurr))->unref();
        } else if (click->isType("dragger")) {
            for (Draw** iter = fList.begin(); iter < fList.end(); iter++) {
                if ((*iter)->isSelected()) {
                    (*iter)->offset(click->fCurr.x() - click->fPrev.x(),
                                    click->fCurr.y() - click->fPrev.y());
                }
            }
        }
        this->inval(NULL);
        return true;
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DrawView; }
static SkViewRegister reg(MyFactory);

