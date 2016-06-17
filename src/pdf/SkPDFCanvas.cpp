/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkNinePatchIter.h"
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
void SkPDFCanvas::onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    this->INHERITED::onClipRect(rect, op, kHard_ClipEdgeStyle);
}

void SkPDFCanvas::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    this->INHERITED::onClipRRect(rrect, op, kHard_ClipEdgeStyle);
}

void SkPDFCanvas::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    this->INHERITED::onClipPath(path, op, kHard_ClipEdgeStyle);
}

void SkPDFCanvas::onDrawBitmapNine(const SkBitmap& bitmap,
                                   const SkIRect& center,
                                   const SkRect& dst,
                                   const SkPaint* paint) {
    SkNinePatchIter iter(bitmap.width(), bitmap.height(), center, dst);
    SkRect srcR, dstR;
    while (iter.next(&srcR, &dstR)) {
        this->drawBitmapRect(bitmap, srcR, dstR, paint);
    }
}

void SkPDFCanvas::onDrawImageNine(const SkImage* image,
                                  const SkIRect& center,
                                  const SkRect& dst,
                     const SkPaint* paint) {
    SkNinePatchIter iter(image->width(), image->height(), center, dst);
    SkRect srcR, dstR;
    while (iter.next(&srcR, &dstR)) {
        this->drawImageRect(image, srcR, dstR, paint);
    }
}

void SkPDFCanvas::onDrawImageRect(const SkImage* image,
                                  const SkRect* srcPtr,
                                  const SkRect& dst,
                                  const SkPaint* paint,
                                  SkCanvas::SrcRectConstraint constraint) {
    SkRect bounds = SkRect::Make(image->bounds());
    SkRect src = srcPtr ? *srcPtr : bounds;
    SkAutoCanvasRestore autoCanvasRestore(this, true);
    if (src != bounds) {
        this->clipRect(dst);
    }
    this->concat(SkMatrix::MakeRectToRect(src, dst,
                                 SkMatrix::kFill_ScaleToFit));
    this->drawImage(image, 0, 0, paint);
}

void SkPDFCanvas::onDrawBitmapRect(const SkBitmap& bitmap,
                                   const SkRect* srcPtr,
                                   const SkRect& dst,
                                   const SkPaint* paint,
                                   SkCanvas::SrcRectConstraint constraint) {
    SkRect bounds = SkRect::Make(bitmap.bounds());
    SkRect src = srcPtr ? *srcPtr : bounds;
    SkAutoCanvasRestore autoCanvasRestore(this, true);
    if (src != bounds) {
        this->clipRect(dst);
    }
    this->concat(SkMatrix::MakeRectToRect(src, dst,
                                 SkMatrix::kFill_ScaleToFit));
    this->drawBitmap(bitmap, 0, 0, paint);
}
