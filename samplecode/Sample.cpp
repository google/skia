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
        r.setLTRB(0, 0, fWidth, fHeight);
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

bool Sample::mouse(SkPoint point, skui::InputState clickState, skui::ModifierKey modifierKeys) {
    switch (clickState) {
        case skui::InputState::kDown:
            fClick = nullptr;
            if (point.x() < 0 || point.y() < 0 || point.x() >= fWidth || point.y() >= fHeight) {
                return false;
            }
            fClick.reset(this->onFindClickHandler(point.x(), point.y(), modifierKeys));
            if (!fClick) {
                return false;
            }
            fClick->fPrev = fClick->fCurr = fClick->fOrig = point;
            fClick->fState = skui::InputState::kDown;
            fClick->fModifierKeys = modifierKeys;
            this->onClick(fClick.get());
            return true;
        case skui::InputState::kMove:
            if (fClick) {
                fClick->fPrev = fClick->fCurr;
                fClick->fCurr = point;
                fClick->fState = skui::InputState::kMove;
                fClick->fModifierKeys = modifierKeys;
                return this->onClick(fClick.get());
            }
            return false;
        case skui::InputState::kUp:
            if (fClick) {
                fClick->fPrev = fClick->fCurr;
                fClick->fCurr = point;
                fClick->fState = skui::InputState::kUp;
                fClick->fModifierKeys = modifierKeys;
                bool result = this->onClick(fClick.get());
                fClick = nullptr;
                return result;
            }
            return false;
        default:
            // Ignore other cases
            SkASSERT(false);
            break;
    }
    SkASSERT(false);
    return false;
}

//////////////////////////////////////////////////////////////////////

void Sample::onSizeChange() {}

Sample::Click* Sample::onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) {
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
