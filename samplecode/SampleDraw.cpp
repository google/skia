#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkRandom.h"

class Draw : public SkRefCnt {
public:
    void draw(SkCanvas* canvas) {
        this->onDraw(canvas);
    }

protected:
    virtual void onDraw(SkCanvas*) = 0;
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

        Draw** iter = fList.begin();
        Draw** stop = fList.end();
        for (; iter < stop; iter++) {
            (*iter)->draw(canvas);
        }
        if (fDraw) {
            fDraw->draw(canvas);
        }
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        return new Click(this);
    }

    virtual bool onClick(Click* click) {
        if (Click::kUp_State == click->fState) {
            *fList.append() = fDraw;
            fDraw = NULL;
            return true;
        }

        if (Click::kDown_State == click->fState) {
            SkPaint p = fFactory->getPaint();
            p.setColor(this->randColor());
            fFactory->setPaint(p);
        }
        this->setDraw(fFactory->create(click->fOrig, click->fCurr))->unref();
        this->inval(NULL);
        return true;
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DrawView; }
static SkViewRegister reg(MyFactory);

