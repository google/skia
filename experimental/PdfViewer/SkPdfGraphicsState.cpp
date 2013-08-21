/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfGraphicsState.h"
#include "SkPdfNativeTokenizer.h"

#include "SkDashPathEffect.h"

SkPdfContext::SkPdfContext(SkPdfNativeDoc* doc)
    : fPdfDoc(doc)
    , fTmpPageAllocator(new SkPdfAllocator()) {
}

SkPdfContext::~SkPdfContext() {
    delete fTmpPageAllocator;
}

void SkPdfGraphicsState::applyGraphicsState(SkPaint* paint, bool stroking) {
    if (stroking) {
        fStroking.applyGraphicsState(paint);
    } else {
        fNonStroking.applyGraphicsState(paint);
    }

    // TODO(edisonn): get this from pdfContext->options,
    // or pdfContext->addPaintOptions(&paint);
    paint->setAntiAlias(true);

    // TODO(edisonn): miter, ...
    if (stroking) {
        paint->setStrokeWidth(SkDoubleToScalar(fLineWidth));
        // TODO(edisonn): perf, two sets of allocs, create SkDashPathEffect constr that takes ownership
        // of the intervals
        if (fDashArrayLength > 0) {
            paint->setPathEffect(new SkDashPathEffect(fDashArray, fDashArrayLength, fDashPhase))->unref();
        }
    }

    // TODO(edisonn): NYI multiple blend modes
    if (fBlendModesLength == 1 && fBlendModes[0] != SkXfermode::kSrc_Mode) {
        paint->setXfermodeMode(fBlendModes[0]);
    }

    //paint->setStrokeMiter(SkDoubleToScalar(fMiterLimit));
    // TODO(edisonn): impl cap and join
}
