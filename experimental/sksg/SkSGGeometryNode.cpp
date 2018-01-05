/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGGeometryNode.h"

#include "SkPath.h"

namespace sksg {

// Geometry nodes don't generate damage on their own, but via their aggregation ancestor Draw nodes.
GeometryNode::GeometryNode() : INHERITED(kBubbleDamage_Trait) {}

void GeometryNode::draw(SkCanvas* canvas, const SkPaint& paint) const {
    SkASSERT(!this->hasInval());
    this->onDraw(canvas, paint);
}

SkPath GeometryNode::asPath() const {
    SkASSERT(!this->hasInval());
    return this->onAsPath();
}

} // namespace sksg
