/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLatticeIter.h"
#include "SkPDFCanvas.h"
#include "SkPDFDevice.h"

SkPDFCanvas::SkPDFCanvas(const sk_sp<SkPDFDevice>& dev)
    : SkCanvas(dev.get()) {}

SkPDFCanvas::~SkPDFCanvas() {}

/*
 *  PDF's impl sometimes wants to access the raster clip as a SkRegion. To keep this valid,
 *  we intercept all clip calls to ensure that the clip stays BW (i.e. never antialiased), since
 *  an antialiased clip won't build a SkRegion (it builds SkAAClip).
 */
void SkPDFCanvas::onClipRect(const SkRect& rect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->INHERITED::onClipRect(rect, op, kHard_ClipEdgeStyle);
}

void SkPDFCanvas::onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->INHERITED::onClipRRect(rrect, op, kHard_ClipEdgeStyle);
}

void SkPDFCanvas::onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->INHERITED::onClipPath(path, op, kHard_ClipEdgeStyle);
}

void SkPDFCanvas::onDrawBitmapNine(const SkBitmap& bitmap,
                                   const SkIRect& center,
                                   const SkRect& dst,
                                   const SkPaint* paint) {
    SkLatticeIter iter(bitmap.width(), bitmap.height(), center, dst);
    SkRect srcR, dstR;
    while (iter.next(&srcR, &dstR)) {
        this->drawBitmapRect(bitmap, srcR, dstR, paint);
    }
}

void SkPDFCanvas::onDrawImageNine(const SkImage* image,
                                  const SkIRect& center,
                                  const SkRect& dst,
                     const SkPaint* paint) {
    SkLatticeIter iter(image->width(), image->height(), center, dst);
    SkRect srcR, dstR;
    while (iter.next(&srcR, &dstR)) {
        this->drawImageRect(image, srcR, dstR, paint);
    }
}

void SkPDFCanvas::onDrawImageRect(const SkImage* image,
                                  const SkRect* src,
                                  const SkRect& dst,
                                  const SkPaint* paint,
                                  SkCanvas::SrcRectConstraint constraint) {
    SkAutoCanvasRestore autoCanvasRestore(this, true);
    this->clipRect(dst);
    this->SkCanvas::onDrawImageRect(image, src, dst, paint, constraint);
}

void SkPDFCanvas::onDrawBitmapRect(const SkBitmap& bitmap,
                                   const SkRect* src,
                                   const SkRect& dst,
                                   const SkPaint* paint,
                                   SkCanvas::SrcRectConstraint constraint) {
    SkAutoCanvasRestore autoCanvasRestore(this, true);
    this->clipRect(dst);
    this->SkCanvas::onDrawBitmapRect(bitmap, src, dst, paint, constraint);
}

void SkPDFCanvas::onDrawImageLattice(const SkImage* image,
                                     const Lattice& lattice,
                                     const SkRect& dst,
                                     const SkPaint* paint) {
    SkLatticeIter iter(lattice, dst);
    SkRect srcR, dstR;
    while (iter.next(&srcR, &dstR)) {
        this->drawImageRect(image, srcR, dstR, paint);
    }
}

void SkPDFCanvas::onDrawBitmapLattice(const SkBitmap& bitmap,
                                      const Lattice& lattice,
                                      const SkRect& dst,
                                      const SkPaint* paint) {
    SkLatticeIter iter(lattice, dst);
    SkRect srcR, dstR;
    while (iter.next(&srcR, &dstR)) {
        this->drawBitmapRect(bitmap, srcR, dstR, paint);
    }
}

