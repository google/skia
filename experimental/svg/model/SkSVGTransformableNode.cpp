/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSVGTransformableNode.h"
#include "SkSVGValue.h"

SkSVGTransformableNode::SkSVGTransformableNode(SkSVGTag tag)
    : INHERITED(tag)
    , fMatrix(SkMatrix::I()) { }

void SkSVGTransformableNode::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
    case SkSVGAttribute::kTransform:
        if (const auto* transform = v.as<SkSVGTransformValue>()) {
            this->setTransform(*transform);
        }
        break;
    default:
        this->INHERITED::onSetAttribute(attr, v);
        break;
    }
}
