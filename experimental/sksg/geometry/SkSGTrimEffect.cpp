/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGTrimEffect.h"

#include "SkCanvas.h"
#include "SkDashPathEffect.h"
#include "SkPathMeasure.h"

namespace sksg {

TrimEffect::TrimEffect(sk_sp<GeometryNode> child)
    : fChild(std::move(child)) {
    fChild->addInvalReceiver(this);
}

TrimEffect::~TrimEffect() {
    fChild->removeInvalReceiver(this);
}

// TODO
//   This is a quick hack to get something on the screen.  What we really want here is to apply
//   the geometry transformation and cache the result on revalidation. Or an SkTrimPathEffect.
void TrimEffect::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    SkASSERT(!this->hasInval());

    SkASSERT(!paint.getPathEffect());

    const auto path = fChild->asPath();
    SkScalar pathLen = 0;
    SkPathMeasure measure(path, false);
    do {
        pathLen += measure.getLength();
    } while (measure.nextContour());

    const auto start  = SkScalarPin(fStart , 0, 1) * pathLen,
               end    = SkScalarPin(fEnd   , 0, 1) * pathLen,
               offset = SkScalarPin(fOffset, 0, 1) * pathLen,
               len    = SkTMax<SkScalar>(end - start, 0);

    const SkScalar dashes[4] = { 0, start, len, pathLen - end };
    SkPaint dashedPaint(paint);
    dashedPaint.setPathEffect(SkDashPathEffect::Make(dashes, 4, -offset));

    canvas->drawPath(path, dashedPaint);
}

SkPath TrimEffect::onAsPath() const {
    return fChild->asPath();
}

SkRect TrimEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());
    return fChild->revalidate(ic, ctm);
}

} // namespace sksg
