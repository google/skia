/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGEffectNode_DEFINED
#define SkSGEffectNode_DEFINED

#include "SkSGRenderNode.h"

namespace sksg {

class EffectNode : public RenderNode {
protected:
    explicit EffectNode(sk_sp<RenderNode>);
    ~EffectNode() override;

    void onRender(SkCanvas*) const override;

    void onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    sk_sp<RenderNode> fChild;

    typedef RenderNode INHERITED;
};

} // namespace sksg

#endif // SkSGEffectNode_DEFINED
