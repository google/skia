/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkanClipEffect_DEFINED
#define SkanClipEffect_DEFINED

#include "SkanEffectNode.h"

namespace skan {

class GeometryNode;

/**
 * Concrete Effect node, applying a clip to its descendants.
 *
 */
class ClipEffect final : public EffectNode {
public:
    static sk_sp<ClipEffect> Make(sk_sp<RenderNode> child, sk_sp<GeometryNode> clip,
                                  bool aa = false) {
        return (child && clip)
            ? sk_sp<ClipEffect>(new ClipEffect(std::move(child), std::move(clip), aa))
            : nullptr;
    }

    ~ClipEffect() override;

protected:
    ClipEffect(sk_sp<RenderNode>, sk_sp<GeometryNode>, bool aa);

    void onRender(SkCanvas*) const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    const sk_sp<GeometryNode> fClipNode;
    const bool                fAntiAlias;

    bool                      fNoop = false;

    typedef EffectNode INHERITED;
};

} // namespace skan

#endif // SkanClipEffect_DEFINED
