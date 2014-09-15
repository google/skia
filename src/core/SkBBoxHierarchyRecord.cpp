
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
    SkPictureStateTree::Draw* draw = fStateTree->appendDraw(this->writeStream().bytesWritten());
    fBoundingHierarchy->insert(draw, bounds, true);
}

void SkBBoxHierarchyRecord::willSave() {
    fStateTree->appendSave();
    this->INHERITED::willSave();
}

SkCanvas::SaveLayerStrategy SkBBoxHierarchyRecord::willSaveLayer(const SkRect* bounds,
                                                                 const SkPaint* paint,
                                                                 SaveFlags flags) {
    // For now, assume all filters affect transparent black.
    // FIXME: This could be made less conservative as an optimization.
    bool paintAffectsTransparentBlack = paint &&
        ((paint->getImageFilter()) ||
         (paint->getColorFilter()));
    bool needToHandleBBox = paintAffectsTransparentBlack;
    if (!needToHandleBBox && paint) {
      // Unusual Xfermodes require us to process a saved layer
      // even with operations outisde the clip.
      // For example, DstIn is used by masking layers.
      // https://code.google.com/p/skia/issues/detail?id=1291
      SkXfermode* xfermode = paint->getXfermode();
      SkXfermode::Mode mode;
      // SrcOver is the common case with a NULL xfermode, so we should
      // make that the fast path and bypass the mode extraction and test.
      if (xfermode && xfermode->asMode(&mode)) {
        switch (mode) {
          case SkXfermode::kClear_Mode:
          case SkXfermode::kSrc_Mode:
          case SkXfermode::kSrcIn_Mode:
          case SkXfermode::kDstIn_Mode:
          case SkXfermode::kSrcOut_Mode:
          case SkXfermode::kDstATop_Mode:
          case SkXfermode::kModulate_Mode:
            needToHandleBBox = true;
            break;
          default:
            break;
        }
      }
    }

    SkRect drawBounds;
    if (needToHandleBBox) {
        SkIRect deviceBounds;
        this->getClipDeviceBounds(&deviceBounds);
        drawBounds.set(deviceBounds);
    }
    fStateTree->appendSaveLayer(this->writeStream().bytesWritten());
    SkCanvas::SaveLayerStrategy strategy = this->INHERITED::willSaveLayer(bounds, paint, flags);
    if (needToHandleBBox) {
        this->handleBBox(drawBounds);
        this->addNoOp();
    }
    return strategy;
}

void SkBBoxHierarchyRecord::willRestore() {
    fStateTree->appendRestore();
    this->INHERITED::willRestore();
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
