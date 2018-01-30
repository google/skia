/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkanGroup_DEFINED
#define SkanGroup_DEFINED

#include "SkanRenderNode.h"

#include "SkTArray.h"

namespace skan {

/**
 * Concrete node, grouping together multiple descendants.
 */
class Group : public RenderNode {
public:
    static sk_sp<Group> Make() {
        return sk_sp<Group>(new Group());
    }

    void addChild(sk_sp<RenderNode>);
    void removeChild(const sk_sp<RenderNode>&);

    size_t size() const { return SkTo<size_t>(fChildren.count()); }
    bool  empty() const { return fChildren.empty(); }

protected:
    Group();
    ~Group() override;

    void onRender(SkCanvas*) const override;
    SkRect onRevalidate(InvalidationController*, const SkMatrix&) override;

private:
    SkTArray<sk_sp<RenderNode>, true> fChildren;

    typedef RenderNode INHERITED;
};

} // namespace skan

#endif // SkanGroup_DEFINED
