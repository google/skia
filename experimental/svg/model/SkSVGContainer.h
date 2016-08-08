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
    virtual ~SkSVGContainer() = default;

    void appendChild(sk_sp<SkSVGNode>) override;

protected:
    SkSVGContainer(SkSVGTag);

    void onRender(const SkSVGRenderContext&) const override;

private:
    SkSTArray<1, sk_sp<SkSVGNode>, true> fChildren;

    typedef SkSVGTransformableNode INHERITED;
};

#endif // SkSVGSVG_DEFINED
