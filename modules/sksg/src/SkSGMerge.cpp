/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGMerge.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/pathops/SkPathOps.h"
#include "include/private/base/SkAssert.h"
#include "modules/sksg/include/SkSGNode.h"

class SkMatrix;

namespace sksg {

Merge::Merge(std::vector<Rec>&& recs)
    : fRecs(std::move(recs)) {
    for (const auto& rec : fRecs) {
        this->observeInval(rec.fGeo);
    }
}

Merge::~Merge() {
    for (const auto& rec : fRecs) {
        this->unobserveInval(rec.fGeo);
    }
}

void Merge::onClip(SkCanvas* canvas, bool antiAlias) const {
    canvas->clipPath(fMerged, SkClipOp::kIntersect, antiAlias);
}

void Merge::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    canvas->drawPath(fMerged, paint);
}

bool Merge::onContains(const SkPoint& p) const {
    return fMerged.contains(p.x(), p.y());
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

    SkOpBuilder builder;
    SkPathBuilder merger;

    bool in_builder = false;

    auto append = [&](const SkPath& path) {
        if (in_builder) {
            if (auto result = builder.resolve()) {
                merger = *result;
            }
            in_builder = false;
        }

        if (merger.isEmpty()) {
            // First merge path determines the fill type.
            merger = path;
        } else {
            merger.addPath(path);
        }
    };

    for (const auto& rec : fRecs) {
        rec.fGeo->revalidate(ic, ctm);

        if (rec.fMode == Mode::kMerge) {
            // Merge (append) is not supported by SkOpBuidler.
            append(rec.fGeo->asPath());
            continue;
        }

        if (!in_builder) {
            builder.add(merger.detach(), kUnion_SkPathOp);
            in_builder = true;
        }

        builder.add(rec.fGeo->asPath(), mode_to_op(rec.fMode));
    }

    fMerged = in_builder
        ? builder.resolve().value_or(SkPath())
        : merger.detach();

    return fMerged.computeTightBounds();
}

} // namespace sksg
