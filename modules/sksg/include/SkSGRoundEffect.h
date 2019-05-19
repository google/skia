/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGRoundEffect_DEFINED
#define SkSGRoundEffect_DEFINED

#include "modules/sksg/include/SkSGGeometryNode.h"

#include "include/core/SkPath.h"

namespace sksg {

/**
 * Concrete Geometry node, applying a rounded-corner effect to its child.
 */
class RoundEffect final : public GeometryNode {
public:
    static sk_sp<RoundEffect> Make(sk_sp<GeometryNode> child) {
        return child ? sk_sp<RoundEffect>(new RoundEffect(std::move(child))) : nullptr;
    }

    ~RoundEffect() override;

    SG_ATTRIBUTE(Radius, SkScalar, fRadius)

protected:
    void onClip(SkCanvas*, bool antiAlias) const override;
    void onDraw(SkCanvas*, const SkPaint&) const override;
    bool onContains(const SkPoint&)        const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;
    SkPath onAsPath() const override;

private:
    explicit RoundEffect(sk_sp<GeometryNode>);

    const sk_sp<GeometryNode> fChild;

    SkPath                    fRoundedPath;
    SkScalar                  fRadius = 0;

    using INHERITED = GeometryNode;
};

} // namespace sksg

#endif // SkSGRoundEffect_DEFINED
