/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGClipEffect_DEFINED
#define SkSGClipEffect_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "modules/sksg/include/SkSGEffectNode.h"
#include "modules/sksg/include/SkSGGeometryNode.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <utility>

class SkCanvas;
class SkMatrix;
struct SkPoint;

namespace sksg {
class InvalidationController;

/**
 * Concrete Effect node, applying a clip to its descendants.
 *
 */
class ClipEffect final : public EffectNode {
public:
    static sk_sp<ClipEffect> Make(sk_sp<RenderNode> child, sk_sp<GeometryNode> clip,
                                  bool aa = false, bool force_clip = false) {
        return (child && clip)
            ? sk_sp<ClipEffect>(new ClipEffect(std::move(child), std::move(clip), aa, force_clip))
            : nullptr;
    }

    ~ClipEffect() override;

protected:
    ClipEffect(sk_sp<RenderNode>, sk_sp<GeometryNode>, bool aa, bool force_clip);

    void onRender(SkCanvas*, const RenderContext*) const override;
    const RenderNode* onNodeAt(const SkPoint&)     const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    const sk_sp<GeometryNode> fClipNode;
    const bool                fAntiAlias,
                              fForceClip;

    bool                      fNoop = false;

    using INHERITED = EffectNode;
};

}  // namespace sksg

#endif // SkSGClipEffect_DEFINED
