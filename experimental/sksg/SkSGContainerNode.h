/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGContainerNode_DEFINED
#define SkSGContainerNode_DEFINED

#include "SkSGRenderNode.h"

#include "SkTArray.h"

namespace sksg {

class ContainerNode : public RenderNode {
public:
    static sk_sp<ContainerNode> Make() {
        return sk_sp<ContainerNode>(new ContainerNode());
    }

    void addChild(sk_sp<RenderNode>);
    void removeChild(const sk_sp<RenderNode>&);

protected:
    ContainerNode();
    ~ContainerNode() override;

    void onRender(SkCanvas*) const override;
    SkRect onComputeBounds() const override;

private:
    SkSTArray<1, sk_sp<RenderNode>, true> fChildren;
};

} // namespace sksg

#endif // SkSGContainerNode_DEFINED
