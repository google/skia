/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGMerge.h"

#include "SkCanvas.h"
#include "SkPathOps.h"

namespace sksg {

Merge::Merge(std::vector<sk_sp<GeometryNode>>&& geos, Mode mode)
    : fGeos(std::move(geos))
    , fMode(mode) {
    for (const auto& geo : fGeos) {
        geo->addInvalReceiver(this);
    }
}

Merge::~Merge() {
    for (const auto& geo : fGeos) {
        geo->removeInvalReceiver(this);
    }
}

void Merge::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    SkASSERT(!this->hasInval());
    canvas->drawPath(fMerged, paint);
}

SkPath Merge::onAsPath() const {
    return fMerged;
}

static SkPathOp mode_to_op(Merge::Mode mode) {
    switch (mode) {
    case Merge::Mode::kUnion:
        return kUnion_SkPathOp;
    case Merge::Mode::kIntersect:
        return kIntersect_SkPathOp;
    case Merge::Mode::kDifference:
        return kDifference_SkPathOp;
    case Merge::Mode::kReverseDifference:
        return kReverseDifference_SkPathOp;
    case Merge::Mode::kXOR:
        return kXOR_SkPathOp;
    default:
        break;
    }

    return kUnion_SkPathOp;
}

SkRect Merge::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    const auto op = mode_to_op(fMode);
    SkOpBuilder builder;

    fMerged.reset();

    for (const auto& geo : fGeos) {
        geo->revalidate(ic, ctm);
        if (fMode == Mode::kMerge) {
            fMerged.addPath(geo->asPath());
        } else {
            builder.add(geo->asPath(), geo == fGeos.front() ? kUnion_SkPathOp : op);
        }
    }

    if (fMode != Mode::kMerge) {
        builder.resolve(&fMerged);
    }

    return fMerged.computeTightBounds();
}

} // namespace sksg
