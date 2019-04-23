/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGTrimEffect_DEFINED
#define SkSGTrimEffect_DEFINED

#include "modules/sksg/include/SkSGGeometryNode.h"

#include "include/core/SkPath.h"
#include "include/effects/SkTrimPathEffect.h"

class SkCanvas;
class SkPaint;

namespace sksg {

/**
 * Concrete Geometry node, applying a trim effect to its child.
 */
class TrimEffect final : public GeometryNode {
public:
    static sk_sp<TrimEffect> Make(sk_sp<GeometryNode> child) {
        return child ? sk_sp<TrimEffect>(new TrimEffect(std::move(child))) : nullptr;
    }

    ~TrimEffect() override;

    SG_ATTRIBUTE(Start , SkScalar              , fStart )
    SG_ATTRIBUTE(Stop  , SkScalar              , fStop  )
    SG_ATTRIBUTE(Mode  , SkTrimPathEffect::Mode, fMode  )

protected:
    void onClip(SkCanvas*, bool antiAlias) const override;
    void onDraw(SkCanvas*, const SkPaint&) const override;
    bool onContains(const SkPoint&)        const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;
    SkPath onAsPath() const override;

private:
    explicit TrimEffect(sk_sp<GeometryNode>);

    const sk_sp<GeometryNode> fChild;

    SkPath                    fTrimmedPath;
    SkScalar                  fStart = 0,
                              fStop  = 1;
    SkTrimPathEffect::Mode    fMode  = SkTrimPathEffect::Mode::kNormal;

    using INHERITED = GeometryNode;
};

} // namespace sksg

#endif // SkSGTrimEffect_DEFINED
