/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkString.h"

#if SK_SUPPORT_GPU
#   include "GrContext.h"
#else
class GrContext;
#endif

//////////////////////////////////////////////////////////////////////////////

bool SampleCode::CharQ(const SkEvent& evt, SkUnichar* outUni) {
    if (evt.isType(gCharEvtName)) {
        if (outUni) {
            *outUni = evt.getFast32();
        }
        return true;
    }
    return false;
}

bool SampleCode::TitleQ(const SkEvent& evt) {
    return evt.isType(gTitleEvtName);
}

void SampleCode::TitleR(SkEvent* evt, const char title[]) {
    SkASSERT(evt && TitleQ(*evt));
    evt->setString(gTitleEvtName, title);
}

bool SampleCode::RequestTitle(SkView* view, SkString* title) {
    SkEvent evt(gTitleEvtName);
    if (view->doQuery(&evt)) {
        title->set(evt.findString(gTitleEvtName));
        return true;
    }
    return false;
}

SkViewRegister* SkViewRegister::gHead;
SkViewRegister::SkViewRegister(SkViewFactory* fact) : fFact(fact) {
    fFact->ref();
    fChain = gHead;
    gHead = this;
}

///////////////////////////////////////////////////////////////////////////////

SkFuncViewFactory::SkFuncViewFactory(SkViewCreateFunc func)
    : fCreateFunc(func) {
}

SkView* SkFuncViewFactory::operator() () const {
    return (*fCreateFunc)();
}

SkViewRegister::SkViewRegister(SkViewCreateFunc func) {
    fFact = new SkFuncViewFactory(func);
    fChain = gHead;
    gHead = this;
}

///////////////////////////////////////////////////////////////////////////////

static const char is_sample_view_tag[] = "sample-is-sample-view";

bool SampleView::IsSampleView(SkView* view) {
    SkEvent evt(is_sample_view_tag);
    return view->doQuery(&evt);
}

bool SampleView::onQuery(SkEvent* evt) {
    if (evt->isType(is_sample_view_tag)) {
        return true;
    }
    return this->INHERITED::onQuery(evt);
}

void SampleView::onDraw(SkCanvas* canvas) {
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
}

void SampleView::onDrawBackground(SkCanvas* canvas) {
    canvas->drawColor(fBGColor);
}

