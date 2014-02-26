
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GMSampleView_DEFINED
#define GMSampleView_DEFINED

#include "SampleCode.h"
#include "gm.h"

class GMSampleView : public SampleView {
private:
    bool fShowSize;
    typedef skiagm::GM GM;

public:
    GMSampleView(GM* gm)
    : fShowSize(false), fGM(gm) {}

    virtual ~GMSampleView() {
        delete fGM;
    }

    static SkEvent* NewShowSizeEvt(bool doShowSize) {
        SkEvent* evt = SkNEW_ARGS(SkEvent, ("GMSampleView::showSize"));
        evt->setFast32(doShowSize);
        return evt;
    }

protected:
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SkString name("GM:");
            name.append(fGM->getName());
            SampleCode::TitleR(evt, name.c_str());
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual bool onEvent(const SkEvent& evt) SK_OVERRIDE {
        if (evt.isType("GMSampleView::showSize")) {
            fShowSize = SkToBool(evt.getFast32());
            return true;
        }
        return this->INHERITED::onEvent(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        {
            SkAutoCanvasRestore acr(canvas, fShowSize);
            fGM->drawContent(canvas);
        }
        if (fShowSize) {
            SkISize size = fGM->getISize();
            SkRect r = SkRect::MakeWH(SkIntToScalar(size.width()),
                                      SkIntToScalar(size.height()));
            SkPaint paint;
            paint.setColor(0x40FF8833);
            canvas->drawRect(r, paint);
        }
    }

    virtual void onDrawBackground(SkCanvas* canvas) {
        fGM->drawBackground(canvas);
    }

private:
    GM* fGM;
    typedef SampleView INHERITED;
};

#endif
