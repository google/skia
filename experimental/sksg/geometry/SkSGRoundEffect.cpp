/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGRoundEffect.h"

#include "SkCanvas.h"
#include "SkCornerPathEffect.h"
#include "SkStrokeRec.h"

namespace sksg {

RoundEffect::RoundEffect(sk_sp<GeometryNode> child)
    : fChild(std::move(child)) {
    this->observeInval(fChild);
}

RoundEffect::~RoundEffect() {
    this->unobserveInval(fChild);
}

void RoundEffect::onClip(SkCanvas* canvas, bool antiAlias) const {
    canvas->clipPath(fRoundedPath, SkClipOp::kIntersect, antiAlias);
}

void RoundEffect::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    SkASSERT(!paint.getPathEffect());

    canvas->drawPath(fRoundedPath, paint);
}

SkPath RoundEffect::onAsPath() const {
    return fRoundedPath;
}

SkRect RoundEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    const auto childbounds = fChild->revalidate(ic, ctm);
    const auto path        = fChild->asPath();

    if (auto round = SkCornerPathEffect::Make(fRadius)) {
        fRoundedPath.reset();
        SkStrokeRec rec(SkStrokeRec::kHairline_InitStyle);
        SkAssertResult(round->filterPath(&fRoundedPath, path, &rec, &childbounds));
    } else {
        fRoundedPath = path;
    }

    return fRoundedPath.computeTightBounds();
}

} // namespace sksg
