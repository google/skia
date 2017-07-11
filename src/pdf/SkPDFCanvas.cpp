/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage.h"
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

static bool is_integer(SkScalar x) {
    return x == SkScalarTruncToScalar(x);
}

static bool is_integral(const SkRect& r) {
    return is_integer(r.left()) &&
           is_integer(r.top()) &&
           is_integer(r.right()) &&
           is_integer(r.bottom());
}

void SkPDFCanvas::onDrawImageRect(const SkImage* image,
                                  const SkRect* src,
                                  const SkRect& dst,
                                  const SkPaint* paint,
                                  SkCanvas::SrcRectConstraint constraint) {
    SkASSERT(src);
    SkASSERT(image);
    if (paint && paint->getMaskFilter()) {
        SkPaint paintCopy(*paint);
        paintCopy.setAntiAlias(true);
        SkRect srcRect = src ? *src : SkRect::Make(image->bounds());
        SkMatrix m = SkMatrix::MakeRectToRect(srcRect, dst, SkMatrix::kFill_ScaleToFit);
        if (!src || *src == SkRect::Make(image->bounds()) ||
                SkCanvas::kFast_SrcRectConstraint == constraint) {
            paintCopy.setShader(image->makeShader(&m));
        } else {
            SkIRect subset = src->roundOut();
            m.preTranslate(subset.x(), subset.y());
            auto si = image->makeSubset(subset);
            if (!si) { return; }
            paintCopy.setShader(si->makeShader(&m));
        }
        this->drawRect(dst, paintCopy);
        return;
    }
    SkAutoCanvasRestore autoCanvasRestore(this, false);
    if (src && !is_integral(*src)) {
        this->save();
        this->clipRect(dst);
    }
    this->SkCanvas::onDrawImageRect(image, src, dst, paint, constraint);
}

void SkPDFCanvas::onDrawBitmapRect(const SkBitmap& bitmap,
                                   const SkRect* src,
                                   const SkRect& dst,
                                   const SkPaint* paint,
                                   SkCanvas::SrcRectConstraint constraint) {
    SkASSERT(src);
    if (paint && paint->getMaskFilter()) {
        if (sk_sp<SkImage> img = SkImage::MakeFromBitmap(bitmap)) {
            this->onDrawImageRect(img.get(), src, dst, paint, constraint);
        }
        return;
    }
    SkAutoCanvasRestore autoCanvasRestore(this, false);
    if (src && !is_integral(*src)) {
        this->save();
        this->clipRect(dst);
    }
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

