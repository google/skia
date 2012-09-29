
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBBoxHierarchyRecord.h"
#include "SkPictureStateTree.h"
#include "SkBBoxHierarchy.h"

SkBBoxHierarchyRecord::SkBBoxHierarchyRecord(uint32_t recordFlags, SkBBoxHierarchy* h)
    : INHERITED(recordFlags) {
    fStateTree = SkNEW(SkPictureStateTree);
    fBoundingHierarchy = h;
    fBoundingHierarchy->ref();
}

void SkBBoxHierarchyRecord::handleBBox(const SkRect& bounds) {
    SkIRect r;
    bounds.roundOut(&r);
    SkPictureStateTree::Draw* draw = fStateTree->appendDraw(this->writeStream().size());
    fBoundingHierarchy->insert(draw, r, true);
}

int SkBBoxHierarchyRecord::save(SaveFlags flags) {
    fStateTree->appendSave();
    return INHERITED::save(flags);
}

int SkBBoxHierarchyRecord::saveLayer(const SkRect* bounds, const SkPaint* paint,
                                     SaveFlags flags) {
    fStateTree->appendSaveLayer(this->writeStream().size());
    return INHERITED::saveLayer(bounds, paint, flags);
}

void SkBBoxHierarchyRecord::restore() {
    fStateTree->appendRestore();
    INHERITED::restore();
}

bool SkBBoxHierarchyRecord::translate(SkScalar dx, SkScalar dy) {
    bool result = INHERITED::translate(dx, dy);
    fStateTree->appendTransform(getTotalMatrix());
    return result;
}

bool SkBBoxHierarchyRecord::scale(SkScalar sx, SkScalar sy) {
    bool result = INHERITED::scale(sx, sy);
    fStateTree->appendTransform(getTotalMatrix());
    return result;
}

bool SkBBoxHierarchyRecord::rotate(SkScalar degrees) {
    bool result = INHERITED::rotate(degrees);
    fStateTree->appendTransform(getTotalMatrix());
    return result;
}

bool SkBBoxHierarchyRecord::skew(SkScalar sx, SkScalar sy) {
    bool result = INHERITED::skew(sx, sy);
    fStateTree->appendTransform(getTotalMatrix());
    return result;
}

bool SkBBoxHierarchyRecord::concat(const SkMatrix& matrix) {
    bool result = INHERITED::concat(matrix);
    fStateTree->appendTransform(getTotalMatrix());
    return result;
}

void SkBBoxHierarchyRecord::setMatrix(const SkMatrix& matrix) {
    INHERITED::setMatrix(matrix);
    fStateTree->appendTransform(getTotalMatrix());
}

bool SkBBoxHierarchyRecord::clipRect(const SkRect& rect,
                                     SkRegion::Op op,
                                     bool doAntiAlias) {
    fStateTree->appendClip(this->writeStream().size());
    return INHERITED::clipRect(rect, op, doAntiAlias);
}

bool SkBBoxHierarchyRecord::clipRegion(const SkRegion& region,
                                       SkRegion::Op op) {
    fStateTree->appendClip(this->writeStream().size());
    return INHERITED::clipRegion(region, op);
}

bool SkBBoxHierarchyRecord::clipPath(const SkPath& path,
                                     SkRegion::Op op,
                                     bool doAntiAlias) {
    fStateTree->appendClip(this->writeStream().size());
    return INHERITED::clipPath(path, op, doAntiAlias);
}

