/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"

#include "SkCanvas.h"
#include "SkView.h"

namespace {

const int N = 8;
const SkScalar kWidth = SkIntToScalar(640);
const SkScalar kHeight = SkIntToScalar(480);
const char gIsOverview[] = "is-overview";

}  // namespace

class OverView : public SkView {
public:
    OverView(int count, const SkViewFactory* factories[]);
    virtual ~OverView();

protected:
    // Overridden from SkEventSink:
    virtual bool onEvent(const SkEvent&) SK_OVERRIDE;
    virtual bool onQuery(SkEvent* evt) SK_OVERRIDE {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Overview");
            return true;
        }
        if (evt->isType(gIsOverview)) {
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }


    // Overridden from SkView:
    virtual void onSizeChange() SK_OVERRIDE;
    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        canvas->drawColor(SK_ColorLTGRAY);
    }

    virtual SkCanvas* beforeChildren(SkCanvas*) SK_OVERRIDE;

    virtual bool onSendClickToChildren(SkScalar x, SkScalar y) SK_OVERRIDE {
        return false;
    }

    virtual Click* onFindClickHandler(SkScalar x, SkScalar y) SK_OVERRIDE {
        int ix = (int)(SkScalarDiv(x * N, kWidth));
        int iy = (int)(SkScalarDiv(y * N, kHeight));
        if (ix >= 0 && iy >= 0) {
            SkEvent evt("set-curr-index");
            evt.setFast32(iy * N + ix);
            this->sendEventToParents(evt);
        }
        return NULL;
    }

private:
    int fCount;
    const SkViewFactory** fFactories;

    typedef SkView INHERITED;
};

SkView* create_overview(int count, const SkViewFactory* factories[]) {
    return SkNEW_ARGS(OverView, (count, factories));
}

bool is_overview(SkView* view) {
    SkEvent isOverview(gIsOverview);
    return view->doQuery(&isOverview);
}

OverView::OverView(int count, const SkViewFactory* factories[]) {
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
        SkView* view = (*fFactories[i])();
        view->setVisibleP(true);
        this->attachChildToBack(view)->unref();
        view->setLoc(locX, locY);
        view->setSize(kWidth, kHeight);
        locX += kWidth;
        if ((i % N) == N - 1) {
            locY += kHeight;
            locX = 0;
        }
    }
}

SkCanvas* OverView::beforeChildren(SkCanvas* canvas) {
    canvas->scale(SK_Scalar1 / N, SK_Scalar1 / N);
    return canvas;
}
