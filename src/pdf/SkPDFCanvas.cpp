/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLatticeIter.h"
#include "SkPDFCanvas.h"
#include "SkPDFDevice.h"
#include "SkPixelRef.h"

SkPDFCanvas::SkPDFCanvas(const sk_sp<SkPDFDevice>& dev)
    : SkCanvas(dev.get()) {}

SkPDFCanvas::~SkPDFCanvas() {}

/*
 *  PDF's impl sometimes wants to access the raster clip as a SkRegion. To keep this valid,
 *  we intercept all clip calls to ensure that the clip stays BW (i.e. never antialiased), since
 *  an antialiased clip won't build a SkRegion (it builds SkAAClip).
 */
void SkPDFCanvas::onClipRect(const SkRect& rect, ClipOp op, ClipEdgeStyle edgeStyle) {
    this->INHERITED::onClipRect(rect, op, kHard_ClipEdgeStyle);
}

void SkPDFCanvas::onClipRRect(const SkRRect& rrect, ClipOp op, ClipEdgeStyle edgeStyle) {
    this->INHERITED::onClipRRect(rrect, op, kHard_ClipEdgeStyle);
}

void SkPDFCanvas::onClipPath(const SkPath& path, ClipOp op, ClipEdgeStyle edgeStyle) {
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
                                  const SkRect* srcPtr,
                                  const SkRect& dst,
                                  const SkPaint* paint,
                                  SkCanvas::SrcRectConstraint constraint) {
    SkRect bounds = SkRect::Make(image->bounds());
    SkRect src = srcPtr ? *srcPtr : bounds;
    SkAutoCanvasRestore autoCanvasRestore(this, true);
    if (src != bounds) {
        // Convert to bitmap to do subset-deduping via SkBitmapKey.
        // https://fiddle.skia.org/c/@image_subsets_get_different_uids
        SkBitmap bitmap;
        if (image->asLegacyBitmap(&bitmap, SkImage::kRO_LegacyBitmapMode)) {
            if (SkPixelRef* pr = bitmap.pixelRef()) {
                if (bitmap.getSubset() == pr->info().bounds()) {
                    pr->setImmutableWithID(image->uniqueID());
                }
            }
            this->onDrawBitmapRect(bitmap, srcPtr, dst, paint, constraint);
        }
        return;
    }
    SkMatrix transform =
        SkMatrix::MakeRectToRect(src, dst, SkMatrix::kFill_ScaleToFit);
    this->concat(transform);
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
    SkMatrix transform =
        SkMatrix::MakeRectToRect(src, dst, SkMatrix::kFill_ScaleToFit);
    SkBitmap subsetBitmap;
    const SkBitmap* bitmapPtr = &bitmap;
    if (src != bounds) {
        SkIRect subset;
        src.roundOut(&subset);
        if (subset.isEmpty() ||
            !subset.intersect(bitmap.bounds()) ||
            !bitmap.extractSubset(&subsetBitmap, subset)) {
            return;
        }
        transform.preTranslate(SkIntToScalar(subset.x()),
                               SkIntToScalar(subset.y()));
        bitmapPtr = &subsetBitmap;
        if (SkRect::Make(subset) != src) {
            this->clipRect(dst);  // sub-pixel clipping
        }
        this->clipRect(dst);
    }
    this->concat(transform);
    this->drawBitmap(*bitmapPtr, 0, 0, paint);
}

void SkPDFCanvas::onDrawImageLattice(const SkImage* image,
                                     const Lattice& lattice,
                                     const SkRect& dst,
                                     const SkPaint* paint) {
    SkLatticeIter iter(image->width(), image->height(), lattice, dst);
    SkRect srcR, dstR;
    while (iter.next(&srcR, &dstR)) {
        this->drawImageRect(image, srcR, dstR, paint);
    }
}

void SkPDFCanvas::onDrawBitmapLattice(const SkBitmap& bitmap,
                                      const Lattice& lattice,
                                      const SkRect& dst,
                                      const SkPaint* paint) {
    SkLatticeIter iter(bitmap.width(), bitmap.height(), lattice, dst);
    SkRect srcR, dstR;
    while (iter.next(&srcR, &dstR)) {
        this->drawBitmapRect(bitmap, srcR, dstR, paint);
    }
}

