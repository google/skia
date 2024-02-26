/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGOpacityEffect_DEFINED
#define SkSGOpacityEffect_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "modules/sksg/include/SkSGEffectNode.h"
#include "modules/sksg/include/SkSGNode.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <utility>

class SkCanvas;
class SkMatrix;
struct SkPoint;

namespace sksg {
class InvalidationController;

/**
 * Concrete Effect node, applying opacity to its descendants.
 *
 */
class OpacityEffect final : public EffectNode {
public:
    static sk_sp<OpacityEffect> Make(sk_sp<RenderNode> child, float opacity = 1) {
        return child ? sk_sp<OpacityEffect>(new OpacityEffect(std::move(child), opacity)) : nullptr;
    }

    SG_ATTRIBUTE(Opacity, float, fOpacity)

protected:
    OpacityEffect(sk_sp<RenderNode>, float);

    void onRender(SkCanvas*, const RenderContext*) const override;
    const RenderNode* onNodeAt(const SkPoint&)     const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    float fOpacity;

    using INHERITED = EffectNode;
};

} // namespace sksg

#endif // SkSGOpacityEffect_DEFINED
