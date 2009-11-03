#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkView.h"

static const int N = 8;
const SkScalar W = SkIntToScalar(640);
const SkScalar H = SkIntToScalar(480); 

class OverView : public SkView {
public:
    OverView(int count, const SkViewFactory factories[]);
    virtual ~OverView();
    
protected:
    virtual bool onEvent(const SkEvent&);
    virtual void onSizeChange();
    
    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorLTGRAY);
    }

    virtual SkCanvas* beforeChildren(SkCanvas*);

    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Overview");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        int ix = (int)(SkScalarDiv(x * N, W));
        int iy = (int)(SkScalarDiv(y * N, H));
        if (ix >= 0 && iy >= 0) {
            SkEvent evt("set-curr-index");
            evt.setFast32(iy * N + ix);
            this->sendEventToParents(evt);
        }
        return NULL;
    }

private:
    int             fCount;
    const SkViewFactory*  fFactories;

    typedef SkView INHERITED;
};

SkView* create_overview(int count, const SkViewFactory factories[]);
SkView* create_overview(int count, const SkViewFactory factories[]) {
    return SkNEW_ARGS(OverView, (count, factories));
};

OverView::OverView(int count, const SkViewFactory factories[]) {
    fCount = count;
    fFactories = factories;
}

OverView::~OverView() {
}

bool OverView::onEvent(const SkEvent& evt) {
    return this->INHERITED::onEvent(evt);
}

void OverView::onSizeChange() {
    this->detachAllChildren();
    
    SkScalar locX = 0;
    SkScalar locY = 0;
    for (int i = 0; i < fCount; i++) {
        SkView* view = fFactories[i]();
        view->setVisibleP(true);
        this->attachChildToBack(view)->unref();
        view->setLoc(locX, locY);
        view->setSize(W, H);
        locX += W;
        if ((i % N) == N - 1) {
            locY += H;
            locX = 0;
        }
    }
}

SkCanvas* OverView::beforeChildren(SkCanvas* canvas) {
    canvas->scale(SK_Scalar1 / N, SK_Scalar1 / N);
    return canvas;
}


