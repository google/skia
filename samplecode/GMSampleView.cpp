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
    SkEvent* evt = new SkEvent("GMSampleView::showSize");
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

    SkUnichar uni;
    if (SampleCode::CharQ(*evt, &uni)) {
        if (fGM->handleKey(uni)) {
            this->inval(nullptr);
            return true;
        }
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

#include "SkPicture.h"
static sk_sp<SkPicture> round_trip_serialize(SkPicture* src) {
    return SkPicture::MakeFromData(src->serialize().get());
}

#include "SkPictureRecorder.h"

#define VAR_WIDTH   400
#define VAR_HEIGHT  60

static SkRect find_variable_bounds(int index, float right) {
    SkRect r = { right - VAR_WIDTH, 0, right, VAR_HEIGHT };
    r.offset(0, index * VAR_HEIGHT);
    return r;
}

void GMSampleView::onDrawContent(SkCanvas* canvas) {
    SkPictureRecorder recorder;
    SkCanvas* origCanvas = canvas;

    if (false) {
        SkISize size = fGM->getISize();
        canvas = recorder.beginRecording(SkRect::MakeIWH(size.width(), size.height()));
    }

    {
        SkAutoCanvasRestore acr(canvas, true);
        fGM->drawContent(canvas);
    }

    if (origCanvas != canvas) {
        sk_sp<SkPicture> pic = recorder.finishRecordingAsPicture();
        if (false) {
            pic = round_trip_serialize(pic.get());
        }
        origCanvas->drawPicture(pic);
        canvas = origCanvas;
    }

    if (fShowSize) {
        SkISize size = fGM->getISize();
        SkRect r = SkRect::MakeWH(SkIntToScalar(size.width()),
                                  SkIntToScalar(size.height()));
        SkPaint paint;
        paint.setColor(0x40FF8833);
        canvas->drawRect(r, paint);
    }

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(30);
    paint.setStyle(SkPaint::kStroke_Style);
    for (int i = 0; i < fGM->fVars.count(); ++i) {
//        skiagm::Variable* var = fGM->fVars[i];
//        canvas->drawText(var->fName.c_str(), var->fName.size(), x, y, paint);
        SkRect r = find_variable_bounds(i, this->width());
        canvas->drawRect(r, paint);
    }
}

class MyClick : public SkView::Click {
public:
    MyClick(SkView* view, int index) : Click(view), fIndex(index) {}

    int fIndex;
};

SkView::Click* GMSampleView::onFindClickHandler(float x, float y, unsigned) {
    for (int i = 0; i < fGM->fVars.count(); ++i) {
        SkRect r = find_variable_bounds(i, this->width());
        if (r.contains(SkRect::MakeXYWH(x, y, 1, 1))) {
            return new MyClick(this, i);
        }
    }
    return nullptr;
}

bool GMSampleView::onClick(Click* click) {
    int i = ((MyClick*)click)->fIndex;
    skiagm::Variable* var = fGM->fVars[i];

    SkRect r = find_variable_bounds(i, this->width());
    *var->fValue = var->fMin + (click->fCurr.fX - r.fLeft) * (var->fMax - var->fMin) / r.width();
    this->inval(nullptr);
    return true;
}

void GMSampleView::onDrawBackground(SkCanvas* canvas) {
    fGM->drawBackground(canvas);
}

bool GMSampleView::onAnimate(const SkAnimTimer& timer) {
    return fGM->animate(timer);
}
