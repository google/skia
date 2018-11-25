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
    this->observeInval(fChild);
}

TrimEffect::~TrimEffect() {
    this->unobserveInval(fChild);
}

void TrimEffect::onClip(SkCanvas* canvas, bool antiAlias) const {
    canvas->clipPath(fChild->asPath(), SkClipOp::kIntersect, antiAlias);
}

// TODO
//   This is a quick hack to get something on the screen.  What we really want here is to apply
//   the geometry transformation and cache the result on revalidation. Or an SkTrimPathEffect.
void TrimEffect::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    SkASSERT(!paint.getPathEffect());

    const auto path = fChild->asPath();
    SkScalar pathLen = 0;
    SkPathMeasure measure(path, false);
    do {
        pathLen += measure.getLength();
    } while (measure.nextContour());

    const auto start  = pathLen * fStart,
               end    = pathLen * fEnd,
               offset = pathLen * fOffset,
               len    = end - start;

    if (len <= 0) {
        return;
    }

    const SkScalar dashes[] = { len, pathLen - len };
    SkPaint dashedPaint(paint);
    dashedPaint.setPathEffect(SkDashPathEffect::Make(dashes,
                                                     SK_ARRAY_COUNT(dashes),
                                                     -start - offset));

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
