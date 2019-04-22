/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGGeometryNode.h"

#include "include/core/SkPath.h"

namespace sksg {

// Geometry nodes don't generate damage on their own, but via their aggregation ancestor Draw nodes.
GeometryNode::GeometryNode() : INHERITED(kBubbleDamage_Trait) {}

void GeometryNode::clip(SkCanvas* canvas, bool aa) const {
    SkASSERT(!this->hasInval());
    this->onClip(canvas, aa);
}

void GeometryNode::draw(SkCanvas* canvas, const SkPaint& paint) const {
    SkASSERT(!this->hasInval());
    this->onDraw(canvas, paint);
}

bool GeometryNode::contains(const SkPoint& p) const {
    SkASSERT(!this->hasInval());
    return this->bounds().contains(p.x(), p.y()) ? this->onContains(p) : false;
}

SkPath GeometryNode::asPath() const {
    SkASSERT(!this->hasInval());
    return this->onAsPath();
}

} // namespace sksg
