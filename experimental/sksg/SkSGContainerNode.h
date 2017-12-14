/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGContainerNode_DEFINED
#define SkSGContainerNode_DEFINED

#include "SkSGRenderableNode.h"

#include "SkTArray.h"

namespace sksg {

class ContainerNode : public RenderableNode {
public:

    void addChild(sk_sp<Node>);
    void removeChild(const sk_sp<Node>&);

protected:
    ContainerNode();

private:
    SkSTArray<1, sk_sp<Node>, true> fChildren;
};

} // namespace sksg

#endif // SkSGContainerNode_DEFINED
