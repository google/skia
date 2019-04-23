/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkString.h"
#include "samplecode/Sample.h"

#if SK_SUPPORT_GPU
#   include "include/gpu/GrContext.h"
#else
class GrContext;
#endif

//////////////////////////////////////////////////////////////////////////////

Sample::Event::Event() : Event("") {}

Sample::Event::Event(const Event& that) {
    *this = that;
}

Sample::Event::Event(const char type[]) : fType(type), f32(0) {
    SkASSERT(type);
}

Sample::Event::~Event() {}

bool Sample::Event::isType(const char type[]) const {
    return fType.equals(type);
}

const char* Sample::kCharEvtName = "SampleCode_Char_Event";
const char* Sample::kTitleEvtName = "SampleCode_Title_Event";

bool Sample::CharQ(const Event& evt, SkUnichar* outUni) {
    if (evt.isType(kCharEvtName)) {
        if (outUni) {
            *outUni = evt.getFast32();
        }
        return true;
    }
    return false;
}

bool Sample::TitleQ(const Event& evt) {
    return evt.isType(kTitleEvtName);
}

void Sample::TitleR(Event* evt, const char title[]) {
    SkASSERT(evt && TitleQ(*evt));
    evt->setString(kTitleEvtName, title);
}

bool Sample::RequestTitle(Sample* view, SkString* title) {
    Event evt(kTitleEvtName);
    if (view->doQuery(&evt)) {
        title->set(evt.findString(kTitleEvtName));
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool Sample::doEvent(const Event& evt) {
    return this->onEvent(evt);
}

bool Sample::onEvent(const Event&) {
    return false;
}

bool Sample::doQuery(Event* evt) {
    SkASSERT(evt);
    return this->onQuery(evt);
}

bool Sample::onQuery(Sample::Event* evt) {
    return false;
}

////////////////////////////////////////////////////////////////////////


void Sample::setSize(SkScalar width, SkScalar height) {
    width = SkMaxScalar(0, width);
    height = SkMaxScalar(0, height);

    if (fWidth != width || fHeight != height)
    {
        fWidth = width;
        fHeight = height;
        this->onSizeChange();
    }
}

void Sample::draw(SkCanvas* canvas) {
    if (fWidth && fHeight) {
        SkRect    r;
        r.set(0, 0, fWidth, fHeight);
        if (canvas->quickReject(r)) {
            return;
        }

        SkAutoCanvasRestore    as(canvas, true);
        int sc = canvas->save();

        if (!fHaveCalledOnceBeforeDraw) {
            fHaveCalledOnceBeforeDraw = true;
            this->onOnceBeforeDraw();
        }
        this->onDrawBackground(canvas);

        SkAutoCanvasRestore acr(canvas, true);
        this->onDrawContent(canvas);
#if SK_SUPPORT_GPU
        // Ensure the GrContext doesn't combine GrDrawOps across draw loops.
        if (GrContext* context = canvas->getGrContext()) {
            context->flush();
        }
#endif

        canvas->restoreToCount(sc);
    }
}

////////////////////////////////////////////////////////////////////////////

Sample::Click::Click(Sample* target) {
    SkASSERT(target);
    fTarget = sk_ref_sp(target);
}

Sample::Click::~Click() {}

Sample::Click* Sample::findClickHandler(SkScalar x, SkScalar y, unsigned modi) {
    if (x < 0 || y < 0 || x >= fWidth || y >= fHeight) {
        return nullptr;
    }

    return this->onFindClickHandler(x, y, modi);
}

void Sample::DoClickDown(Click* click, int x, int y, unsigned modi) {
    SkASSERT(click);

    Sample* target = click->fTarget.get();
    if (nullptr == target) {
        return;
    }

    click->fIOrig.set(x, y);
    click->fICurr = click->fIPrev = click->fIOrig;

    click->fOrig.iset(x, y);
    click->fPrev = click->fCurr = click->fOrig;

    click->fState = Click::kDown_State;
    click->fModifierKeys = modi;
    target->onClick(click);
}

void Sample::DoClickMoved(Click* click, int x, int y, unsigned modi) {
    SkASSERT(click);

    Sample* target = click->fTarget.get();
    if (nullptr == target) {
        return;
    }

    click->fIPrev = click->fICurr;
    click->fICurr.set(x, y);

    click->fPrev = click->fCurr;
    click->fCurr.iset(x, y);

    click->fState = Click::kMoved_State;
    click->fModifierKeys = modi;
    target->onClick(click);
}

void Sample::DoClickUp(Click* click, int x, int y, unsigned modi) {
    SkASSERT(click);

    Sample* target = click->fTarget.get();
    if (nullptr == target) {
        return;
    }

    click->fIPrev = click->fICurr;
    click->fICurr.set(x, y);

    click->fPrev = click->fCurr;
    click->fCurr.iset(x, y);

    click->fState = Click::kUp_State;
    click->fModifierKeys = modi;
    target->onClick(click);
}

//////////////////////////////////////////////////////////////////////

void Sample::onSizeChange() {}

Sample::Click* Sample::onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) {
    return nullptr;
}

bool Sample::onClick(Click*) {
    return false;
}

void Sample::onDrawBackground(SkCanvas* canvas) {
    canvas->drawColor(fBGColor);
}

// need to explicitly declare this, or we get some weird infinite loop llist
template SampleRegistry* SampleRegistry::gHead;
