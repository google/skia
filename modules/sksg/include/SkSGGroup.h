/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGGroup_DEFINED
#define SkSGGroup_DEFINED

#include "modules/sksg/include/SkSGRenderNode.h"

#include <vector>

namespace sksg {

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
    explicit Group(std::vector<sk_sp<RenderNode>>);
    ~Group() override;

    void onRender(SkCanvas*, const RenderContext*) const override;
    const RenderNode* onNodeAt(const SkPoint&)     const override;

    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    std::vector<sk_sp<RenderNode>> fChildren;

    typedef RenderNode INHERITED;
};

} // namespace sksg

#endif // SkSGGroup_DEFINED
