/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGDashEffect_DEFINED
#define SkSGDashEffect_DEFINED

#include "include/core/SkPath.h"
#include "modules/sksg/include/SkSGGeometryNode.h"

#include <vector>

namespace  sksg {

/**
 * Apply a dash effect to the child geometry.
 *
 * Follows the same semantics as SkDashPathEffect, with one minor tweak: when the number of
 * intervals is odd, they are repeated once more to attain an even sequence (same as SVG
 * stroke-dasharray: https://www.w3.org/TR/SVG11/painting.html#StrokeDasharrayProperty).
 */
class DashEffect final : public GeometryNode {
public:
    static sk_sp<DashEffect> Make(sk_sp<GeometryNode> child) {
        return child ? sk_sp<DashEffect>(new DashEffect(std::move(child))) : nullptr;
    }

    ~DashEffect() override;

    SG_ATTRIBUTE(Intervals, std::vector<float>, fIntervals)
    SG_ATTRIBUTE(Phase,                 float , fPhase    )

protected:
    void onClip(SkCanvas*, bool antiAlias) const override;
    void onDraw(SkCanvas*, const SkPaint&) const override;
    bool onContains(const SkPoint&)        const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;
    SkPath onAsPath() const override;

private:
    explicit DashEffect(sk_sp<GeometryNode>);

    const sk_sp<GeometryNode> fChild;

    SkPath fDashedPath; // cache

    std::vector<float> fIntervals;
    float              fPhase;
};

} // namespace sksg

#endif // SkSGDashEffect_DEFINED
