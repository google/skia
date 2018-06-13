/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkView.h"

#include "SkCanvas.h"
#include "SkTo.h"

static inline uint32_t SkSetClearShift(uint32_t bits, bool cond, unsigned shift) {
    SkASSERT((int)cond == 0 || (int)cond == 1);
    return (bits & ~(1 << shift)) | ((int)cond << shift);
}

////////////////////////////////////////////////////////////////////////

SkView::SkView(uint32_t flags) : fFlags(SkToU8(flags)) {
    fWidth = fHeight = 0;
}

SkView::~SkView() {}

void SkView::setFlags(uint32_t flags) {
    SkASSERT((flags & ~kAllFlagMasks) == 0);
    fFlags = SkToU8(flags);
}

void SkView::setVisibleP(bool pred) {
    this->setFlags(SkSetClearShift(fFlags, pred, kVisible_Shift));
}

void SkView::setClipToBounds(bool pred) {
    this->setFlags(SkSetClearShift(fFlags, !pred, kNoClip_Shift));
}

void SkView::setSize(SkScalar width, SkScalar height) {
    width = SkMaxScalar(0, width);
    height = SkMaxScalar(0, height);

    if (fWidth != width || fHeight != height)
    {
        fWidth = width;
        fHeight = height;
        this->onSizeChange();
    }
}

void SkView::draw(SkCanvas* canvas) {
    if (fWidth && fHeight && this->isVisible()) {
        SkRect    r;
        r.set(0, 0, fWidth, fHeight);
        if (this->isClipToBounds() && canvas->quickReject(r)) {
            return;
        }

        SkAutoCanvasRestore    as(canvas, true);

        if (this->isClipToBounds()) {
            canvas->clipRect(r);
        }

        int sc = canvas->save();
        this->onDraw(canvas);
        canvas->restoreToCount(sc);
    }
}

////////////////////////////////////////////////////////////////////////////

SkView::Click::Click(SkView* target) {
    SkASSERT(target);
    fTargetID = target->getSinkID();
}

SkView::Click::~Click() {}

SkView::Click* SkView::findClickHandler(SkScalar x, SkScalar y, unsigned modi) {
    if (x < 0 || y < 0 || x >= fWidth || y >= fHeight) {
        return nullptr;
    }

    return this->onFindClickHandler(x, y, modi);
}

void SkView::DoClickDown(Click* click, int x, int y, unsigned modi) {
    SkASSERT(click);

    SkView* target = (SkView*)SkEventSink::FindSink(click->fTargetID);
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

void SkView::DoClickMoved(Click* click, int x, int y, unsigned modi) {
    SkASSERT(click);

    SkView* target = (SkView*)SkEventSink::FindSink(click->fTargetID);
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

void SkView::DoClickUp(Click* click, int x, int y, unsigned modi) {
    SkASSERT(click);

    SkView* target = (SkView*)SkEventSink::FindSink(click->fTargetID);
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

void SkView::onDraw(SkCanvas* canvas) {}

void SkView::onSizeChange() {}

SkView::Click* SkView::onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) {
    return nullptr;
}

bool SkView::onClick(Click*) {
    return false;
}
