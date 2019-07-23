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

void Sample::setWindowSize(SkSize size) {
    size = {SkMaxScalar(0, size.width()), SkMaxScalar(0, size.height())};
    if (fWindowSize != size) {
        fWindowSize = size;
        this->onSizeChange();
    }
}

void Sample::drawSample(SkCanvas* canvas) {
    if (fWindowSize.isEmpty() || canvas->quickReject(SkRect::MakeSize(fWindowSize))) {
        return;
    }

    SkAutoCanvasRestore as(canvas, true);
    this->skiagm::GM::draw(canvas);
#if SK_SUPPORT_GPU
    // Ensure the GrContext doesn't combine GrDrawOps across draw loops.
    if (GrContext* context = canvas->getGrContext()) {
        context->flush();
    }
#endif
}

////////////////////////////////////////////////////////////////////////////

bool Sample::mouse(SkPoint point, InputState clickState, ModifierKey modifierKeys) {
    switch (clickState) {
        case InputState::kDown:
            fClick = nullptr;
            if (!SkRect::MakeSize(fWindowSize).contains(point.x(), point.y())) {
                return false;
            }
            fClick.reset(this->onFindClickHandler(point.x(), point.y(), modifierKeys));
            if (!fClick) {
                return false;
            }
            fClick->fPrev = fClick->fCurr = fClick->fOrig = point;
            fClick->fState = InputState::kDown;
            fClick->fModifierKeys = modifierKeys;
            this->onClick(fClick.get());
            return true;
        case InputState::kMove:
            if (fClick) {
                fClick->fPrev = fClick->fCurr;
                fClick->fCurr = point;
                fClick->fState = InputState::kMove;
                fClick->fModifierKeys = modifierKeys;
                return this->onClick(fClick.get());
            }
            return false;
        case InputState::kUp:
            if (fClick) {
                fClick->fPrev = fClick->fCurr;
                fClick->fCurr = point;
                fClick->fState = InputState::kUp;
                fClick->fModifierKeys = modifierKeys;
                bool result = this->onClick(fClick.get());
                fClick = nullptr;
                return result;
            }
            return false;
    }
    SkASSERT(false);
    return false;
}

//////////////////////////////////////////////////////////////////////

void Sample::onSizeChange() {}

Sample::Click* Sample::onFindClickHandler(SkScalar, SkScalar, ModifierKey) { return nullptr; }

bool Sample::onClick(Click*) { return false; }

// need to explicitly declare this, or we get some weird infinite loop llist
template SampleRegistry* SampleRegistry::gHead;
