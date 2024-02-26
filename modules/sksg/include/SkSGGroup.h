/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGGroup_DEFINED
#define SkSGGroup_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <cstddef>
#include <utility>
#include <vector>

class SkCanvas;
class SkMatrix;
struct SkPoint;

namespace sksg {
class InvalidationController;

/**
 * Concrete node, grouping together multiple descendants.
 */
class Group : public RenderNode {
public:
    static sk_sp<Group> Make() {
        return sk_sp<Group>(new Group(std::vector<sk_sp<RenderNode>>()));
    }

    static sk_sp<Group> Make(std::vector<sk_sp<RenderNode>> children) {
        return sk_sp<Group>(new Group(std::move(children)));
    }

    void addChild(sk_sp<RenderNode>);
    void removeChild(const sk_sp<RenderNode>&);

    size_t size() const { return fChildren.size(); }
    bool  empty() const { return fChildren.empty(); }
    void  clear();

protected:
    Group();
    explicit Group(std::vector<sk_sp<RenderNode>>);
    ~Group() override;

    void onRender(SkCanvas*, const RenderContext*) const override;
    const RenderNode* onNodeAt(const SkPoint&)     const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    std::vector<sk_sp<RenderNode>> fChildren;
    bool                           fRequiresIsolation = true;

    using INHERITED = RenderNode;
};

} // namespace sksg

#endif // SkSGGroup_DEFINED
