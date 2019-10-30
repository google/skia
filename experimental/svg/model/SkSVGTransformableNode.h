/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGTransformableNode_DEFINED
#define SkSVGTransformableNode_DEFINED

#include "experimental/svg/model/SkSVGNode.h"
#include "include/core/SkMatrix.h"

class SkSVGTransformableNode : public SkSVGNode {
public:
    ~SkSVGTransformableNode() override = default;

    void setTransform(const SkSVGTransformType& t) { fTransform = t; }

protected:
    SkSVGTransformableNode(SkSVGTag);

    bool onPrepareToRender(SkSVGRenderContext*) const override;

    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

    void mapToParent(SkPath*) const;

private:
    // FIXME: should be sparse
    SkSVGTransformType fTransform;

    typedef SkSVGNode INHERITED;
};

#endif // SkSVGTransformableNode_DEFINED
