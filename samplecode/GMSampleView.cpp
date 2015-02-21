/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GMSampleView.h"

GMSampleView::GMSampleView(GM* gm) : fShowSize(false), fGM(gm) {}

GMSampleView::~GMSampleView() {
    delete fGM;
}

SkEvent* GMSampleView::NewShowSizeEvt(bool doShowSize) {
    SkEvent* evt = SkNEW_ARGS(SkEvent, ("GMSampleView::showSize"));
    evt->setFast32(doShowSize);
    return evt;
}

bool GMSampleView::onQuery(SkEvent* evt) {
    if (SampleCode::TitleQ(*evt)) {
        SkString name("GM:");
        name.append(fGM->getName());
        SampleCode::TitleR(evt, name.c_str());
        return true;
    }
    return this->INHERITED::onQuery(evt);
}

bool GMSampleView::onEvent(const SkEvent& evt) {
    if (evt.isType("GMSampleView::showSize")) {
        fShowSize = SkToBool(evt.getFast32());
        return true;
    }
    return this->INHERITED::onEvent(evt);
}

void GMSampleView::onDrawContent(SkCanvas* canvas) {
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

void GMSampleView::onDrawBackground(SkCanvas* canvas) {
    fGM->drawBackground(canvas);
}

bool GMSampleView::onAnimate(const SkAnimTimer& timer) {
    return fGM->animate(timer);
}

