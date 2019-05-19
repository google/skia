/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGTrimEffect.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkStrokeRec.h"
#include "include/effects/SkTrimPathEffect.h"

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

bool TrimEffect::onContains(const SkPoint& p) const {
    return fTrimmedPath.contains(p.x(), p.y());
}

SkPath TrimEffect::onAsPath() const {
    return fTrimmedPath;
}

SkRect TrimEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    const auto childbounds = fChild->revalidate(ic, ctm);
    const auto path        = fChild->asPath();

    if (auto trim = SkTrimPathEffect::Make(fStart, fStop, fMode)) {
        fTrimmedPath.reset();
        SkStrokeRec rec(SkStrokeRec::kHairline_InitStyle);
        SkAssertResult(trim->filterPath(&fTrimmedPath, path, &rec, &childbounds));
    } else {
        fTrimmedPath = path;
    }

    fTrimmedPath.shrinkToFit();

    return fTrimmedPath.computeTightBounds();
}

} // namespace sksg
