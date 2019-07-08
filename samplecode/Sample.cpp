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

Sample::Click* Sample::findClickHandler(SkScalar x, SkScalar y, ModifierKey modi) {
    if (x < 0 || y < 0 || x >= fWidth || y >= fHeight) {
        return nullptr;
    }

    return this->onFindClickHandler(x, y, modi);
}

void Sample::DoClickDown(Click* click, int x, int y, ModifierKey modi) {
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

void Sample::DoClickMoved(Click* click, int x, int y, ModifierKey modi) {
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

void Sample::DoClickUp(Click* click, int x, int y, ModifierKey modi) {
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

Sample::Click* Sample::onFindClickHandler(SkScalar x, SkScalar y, ModifierKey modi) {
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
