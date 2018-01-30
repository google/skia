/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkanTrimEffect_DEFINED
#define SkanTrimEffect_DEFINED

#include "SkanGeometryNode.h"

#include "SkPath.h"

class SkCanvas;
class SkPaint;

namespace skan {

/**
 * Concrete Geometry node, applying a trim effect to its child.
 */
class TrimEffect final : public GeometryNode {
public:
    static sk_sp<TrimEffect> Make(sk_sp<GeometryNode> child) {
        return child ? sk_sp<TrimEffect>(new TrimEffect(std::move(child))) : nullptr;
    }

    ~TrimEffect() override;

    SG_ATTRIBUTE(Start , SkScalar, fStart )
    SG_ATTRIBUTE(End   , SkScalar, fEnd   )
    SG_ATTRIBUTE(Offset, SkScalar, fOffset)

protected:
    void onClip(SkCanvas*, bool antiAlias) const override;
    void onDraw(SkCanvas*, const SkPaint&) const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;
    SkPath onAsPath() const override;

private:
    explicit TrimEffect(sk_sp<GeometryNode>);

    const sk_sp<GeometryNode> fChild;

    SkScalar                  fStart  = 0, // starting t
                              fEnd    = 1, // ending t
                              fOffset = 0; // t offset
};

} // namespace skan

#endif // SkanTrimEffect_DEFINED
