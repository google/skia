/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGDashEffect.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkStrokeRec.h"
#include "include/effects/SkDashPathEffect.h"

#include <algorithm>

namespace sksg {

namespace  {

sk_sp<SkPathEffect> make_dash(const std::vector<float> intervals, float phase) {
    if (intervals.empty()) {
        return nullptr;
    }

    const auto* intervals_ptr   = intervals.data();
    auto        intervals_count = intervals.size();

    SkSTArray<32, float, true> storage;
    if (intervals_count & 1) {
        intervals_count *= 2;
        storage.resize(intervals_count);
        intervals_ptr = storage.data();

        std::copy(intervals.begin(), intervals.end(), storage.begin());
        std::copy(intervals.begin(), intervals.end(), storage.begin() + intervals.size());
    }

    return SkDashPathEffect::Make(intervals_ptr, SkToInt(intervals_count), phase);
}

} // namespace

DashEffect::DashEffect(sk_sp<GeometryNode> child)
    : fChild(std::move(child)) {
    this->observeInval(fChild);
}

DashEffect::~DashEffect() {
    this->unobserveInval(fChild);
}

void DashEffect::onClip(SkCanvas* canvas, bool antiAlias) const {
    canvas->clipPath(fDashedPath, SkClipOp::kIntersect, antiAlias);
}

void DashEffect::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    canvas->drawPath(fDashedPath, paint);
}

bool DashEffect::onContains(const SkPoint& p) const {
    return fDashedPath.contains(p.x(), p.y());
}

SkPath DashEffect::onAsPath() const {
    return fDashedPath;
}

SkRect DashEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    const auto child_bounds = fChild->revalidate(ic, ctm);
    const auto child_path   = fChild->asPath();

    fDashedPath.reset();

    auto dash_patheffect = make_dash(fIntervals, fPhase);
    SkStrokeRec rec(SkStrokeRec::kHairline_InitStyle);

    if (!dash_patheffect ||
        !dash_patheffect->filterPath(&fDashedPath, child_path, &rec, &child_bounds)) {
        fDashedPath = std::move(child_path);
    }
    fDashedPath.shrinkToFit();

    return fDashedPath.computeTightBounds();
}

} // namespace sksg
