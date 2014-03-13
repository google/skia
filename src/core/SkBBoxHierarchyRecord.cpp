
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBBoxHierarchyRecord.h"
#include "SkPictureStateTree.h"

SkBBoxHierarchyRecord::SkBBoxHierarchyRecord(const SkISize& size,
                                             uint32_t recordFlags,
                                             SkBBoxHierarchy* h)
    : INHERITED(size, recordFlags) {
    fStateTree = SkNEW(SkPictureStateTree);
    fBoundingHierarchy = h;
    fBoundingHierarchy->ref();
    fBoundingHierarchy->setClient(this);
}

void SkBBoxHierarchyRecord::handleBBox(const SkRect& bounds) {
    SkIRect r;
    bounds.roundOut(&r);
    SkPictureStateTree::Draw* draw = fStateTree->appendDraw(this->writeStream().bytesWritten());
    fBoundingHierarchy->insert(draw, r, true);
}

void SkBBoxHierarchyRecord::willSave(SaveFlags flags) {
    fStateTree->appendSave();
    this->INHERITED::willSave(flags);
}

SkCanvas::SaveLayerStrategy SkBBoxHierarchyRecord::willSaveLayer(const SkRect* bounds,
                                                                 const SkPaint* paint,
                                                                 SaveFlags flags) {
    fStateTree->appendSaveLayer(this->writeStream().bytesWritten());
    return this->INHERITED::willSaveLayer(bounds, paint, flags);
}

void SkBBoxHierarchyRecord::willRestore() {
    fStateTree->appendRestore();
    this->INHERITED::willRestore();
}

void SkBBoxHierarchyRecord::didTranslate(SkScalar dx, SkScalar dy) {
    fStateTree->appendTransform(getTotalMatrix());
    INHERITED::didTranslate(dx, dy);
}

void SkBBoxHierarchyRecord::didScale(SkScalar sx, SkScalar sy) {
    fStateTree->appendTransform(getTotalMatrix());
    INHERITED::didScale(sx, sy);
}

void SkBBoxHierarchyRecord::didRotate(SkScalar degrees) {
    fStateTree->appendTransform(getTotalMatrix());
    INHERITED::didRotate(degrees);
}

void SkBBoxHierarchyRecord::didSkew(SkScalar sx, SkScalar sy) {
    fStateTree->appendTransform(getTotalMatrix());
    INHERITED::didSkew(sx, sy);
}

void SkBBoxHierarchyRecord::didConcat(const SkMatrix& matrix) {
    fStateTree->appendTransform(getTotalMatrix());
    INHERITED::didConcat(matrix);
}

void SkBBoxHierarchyRecord::didSetMatrix(const SkMatrix& matrix) {
    fStateTree->appendTransform(getTotalMatrix());
    INHERITED::didSetMatrix(matrix);
}

void SkBBoxHierarchyRecord::onClipRect(const SkRect& rect,
                                       SkRegion::Op op,
                                       ClipEdgeStyle edgeStyle) {
    fStateTree->appendClip(this->writeStream().bytesWritten());
    this->INHERITED::onClipRect(rect, op, edgeStyle);
}

void SkBBoxHierarchyRecord::onClipRegion(const SkRegion& region,
                                         SkRegion::Op op) {
    fStateTree->appendClip(this->writeStream().bytesWritten());
    this->INHERITED::onClipRegion(region, op);
}

void SkBBoxHierarchyRecord::onClipPath(const SkPath& path,
                                       SkRegion::Op op,
                                       ClipEdgeStyle edgeStyle) {
    fStateTree->appendClip(this->writeStream().bytesWritten());
    this->INHERITED::onClipPath(path, op, edgeStyle);
}

void SkBBoxHierarchyRecord::onClipRRect(const SkRRect& rrect,
                                        SkRegion::Op op,
                                        ClipEdgeStyle edgeStyle) {
    fStateTree->appendClip(this->writeStream().bytesWritten());
    this->INHERITED::onClipRRect(rrect, op, edgeStyle);
}

bool SkBBoxHierarchyRecord::shouldRewind(void* data) {
    // SkBBoxHierarchy::rewindInserts is called by SkPicture after the
    // SkPicture has rewound its command stream.  To match that rewind in the
    // BBH, we rewind all draws that reference commands that were recorded
    // past the point to which the SkPicture has rewound, which is given by
    // writeStream().bytesWritten().
    SkPictureStateTree::Draw* draw = static_cast<SkPictureStateTree::Draw*>(data);
    return draw->fOffset >= writeStream().bytesWritten();
}
