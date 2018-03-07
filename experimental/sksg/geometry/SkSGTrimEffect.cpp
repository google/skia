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
#include "SkStrokeRec.h"

namespace sksg {

TrimEffect::TrimEffect(sk_sp<GeometryNode> child)
    : fChild(std::move(child)) {
    this->observeInval(fChild);
}

TrimEffect::~TrimEffect() {
    this->unobserveInval(fChild);
}

void TrimEffect::onClip(SkCanvas* canvas, bool antiAlias) const {
    canvas->clipPath(fTrimmedPath, SkClipOp::kIntersect, antiAlias);
}

void TrimEffect::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    SkASSERT(!paint.getPathEffect());

    canvas->drawPath(fTrimmedPath, paint);
}

SkPath TrimEffect::onAsPath() const {
    return fTrimmedPath;
}

SkRect TrimEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    const auto childbounds = fChild->revalidate(ic, ctm);

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

    fTrimmedPath.reset();

    if (len > 0) {
        // If the trim is positioned exactly at the beginning of the path, we don't expect any
        // visible wrap-around.  But due to limited precision / accumulated error, the dash effect
        // may sometimes start one extra dash at the very end (a flickering dot during animation).
        // To avoid this, we bump the dash len by |epsilon|.
        const SkScalar dashes[] = { len, pathLen + SK_ScalarNearlyZero - len };
        auto dashEffect = SkDashPathEffect::Make(dashes,
                                                 SK_ARRAY_COUNT(dashes),
                                                 -start - offset);
        if (dashEffect) {
            SkStrokeRec rec(SkStrokeRec::kHairline_InitStyle);
            SkAssertResult(dashEffect->filterPath(&fTrimmedPath, path, &rec, &childbounds));
        }
    }

    return fTrimmedPath.computeTightBounds();
}

} // namespace sksg
