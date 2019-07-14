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
    SkSize size = {SkMaxScalar(0, width), SkMaxScalar(0, height)};

    if (fSize != size) {
        fSize = size;
        this->onSizeChange();
    }
}

void Sample::draw(SkCanvas* canvas) {
    if (!fSize.isZero()) {
        if (canvas->quickReject(SkRect::MakeSize(fSize))) {
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

void Sample::onSizeChange() {}

void Sample::onDrawBackground(SkCanvas* canvas) {
    canvas->drawColor(fBGColor);
}

// need to explicitly declare this, or we get some weird infinite loop llist
template SampleRegistry* SampleRegistry::gHead;
