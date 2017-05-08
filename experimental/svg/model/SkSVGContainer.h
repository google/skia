/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGContainer_DEFINED
#define SkSVGContainer_DEFINED

#include "SkSVGTransformableNode.h"
#include "SkTArray.h"

class SkSVGContainer : public SkSVGTransformableNode {
public:
    ~SkSVGContainer() override = default;

    void appendChild(sk_sp<SkSVGNode>) override;

protected:
    explicit SkSVGContainer(SkSVGTag);

    void onRender(const SkSVGRenderContext&) const override;

    SkPath onAsPath(const SkSVGRenderContext&) const override;

    bool hasChildren() const final;

    // TODO: add some sort of child iterator, and hide the container.
    SkSTArray<1, sk_sp<SkSVGNode>, true> fChildren;

private:
    typedef SkSVGTransformableNode INHERITED;
};

#endif // SkSVGSVG_DEFINED
