/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaintFilterCanvas.h"

#include "SkPaint.h"
#include "SkTLazy.h"

class SkPaintFilterCanvas::AutoPaintFilter {
public:
    AutoPaintFilter(const SkPaintFilterCanvas* canvas, Type type, const SkPaint* paint) {
        if (paint) {
            canvas->onFilterPaint(fLazyPaint.set(*paint), type);
        }
    }

    AutoPaintFilter(const SkPaintFilterCanvas* canvas, Type type, const SkPaint& paint) {
        canvas->onFilterPaint(fLazyPaint.set(paint), type);
    }

    const SkPaint* paint() const { return fLazyPaint.getMaybeNull(); }

private:
    SkTLazy<SkPaint> fLazyPaint;
};

SkPaintFilterCanvas::SkPaintFilterCanvas(int width, int height) : INHERITED(width, height) { }

void SkPaintFilterCanvas::onDrawPaint(const SkPaint& paint) {
    AutoPaintFilter apf(this, kPaint_Type, paint);
    this->INHERITED::onDrawPaint(*apf.paint());
}

void SkPaintFilterCanvas::onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                                       const SkPaint& paint) {
    AutoPaintFilter apf(this, kPoint_Type, paint);
    this->INHERITED::onDrawPoints(mode, count, pts, *apf.paint());
}

void SkPaintFilterCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    AutoPaintFilter apf(this, kRect_Type, paint);
    this->INHERITED::onDrawRect(rect, *apf.paint());
}

void SkPaintFilterCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    AutoPaintFilter apf(this, kRRect_Type, paint);
    this->INHERITED::onDrawRRect(rrect, *apf.paint());
}

void SkPaintFilterCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                                       const SkPaint& paint) {
    AutoPaintFilter apf(this, kDRRect_Type, paint);
    this->INHERITED::onDrawDRRect(outer, inner, *apf.paint());
}

void SkPaintFilterCanvas::onDrawOval(const SkRect& rect, const SkPaint& paint) {
    AutoPaintFilter apf(this, kOval_Type, paint);
    this->INHERITED::onDrawOval(rect, *apf.paint());
}

void SkPaintFilterCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    AutoPaintFilter apf(this, kPath_Type, paint);
    this->INHERITED::onDrawPath(path, *apf.paint());
}

void SkPaintFilterCanvas::onDrawBitmap(const SkBitmap& bm, SkScalar left, SkScalar top,
                                       const SkPaint* paint) {
    AutoPaintFilter apf(this, kBitmap_Type, paint);
    this->INHERITED::onDrawBitmap(bm, left, top, apf.paint());
}

void SkPaintFilterCanvas::onDrawBitmapRect(const SkBitmap& bm, const SkRect* src, const SkRect& dst,
                                           const SkPaint* paint, SrcRectConstraint constraint) {
    AutoPaintFilter apf(this, kBitmap_Type, paint);
    this->INHERITED::onDrawBitmapRect(bm, src, dst, apf.paint(), constraint);
}

void SkPaintFilterCanvas::onDrawImage(const SkImage* image, SkScalar left, SkScalar top,
                                      const SkPaint* paint) {
    AutoPaintFilter apf(this, kBitmap_Type, paint);
    this->INHERITED::onDrawImage(image, left, top, apf.paint());
}

void SkPaintFilterCanvas::onDrawImageRect(const SkImage* image, const SkRect* src,
                                          const SkRect& dst, const SkPaint* paint,
                                          SrcRectConstraint constraint) {
    AutoPaintFilter apf(this, kBitmap_Type, paint);
    this->INHERITED::onDrawImageRect(image, src, dst, apf.paint(), constraint);
}

void SkPaintFilterCanvas::onDrawBitmapNine(const SkBitmap& bm, const SkIRect& center,
                                           const SkRect& dst, const SkPaint* paint) {
    AutoPaintFilter apf(this, kBitmap_Type, paint);
    this->INHERITED::onDrawBitmapNine(bm, center, dst, apf.paint());
}

void SkPaintFilterCanvas::onDrawSprite(const SkBitmap& bm, int left, int top,
                                       const SkPaint* paint) {
    AutoPaintFilter apf(this, kBitmap_Type, paint);
    this->INHERITED::onDrawSprite(bm, left, top, apf.paint());
}

void SkPaintFilterCanvas::onDrawVertices(VertexMode vmode, int vertexCount,
                                         const SkPoint vertices[], const SkPoint texs[],
                                         const SkColor colors[], SkXfermode* xmode,
                                         const uint16_t indices[], int indexCount,
                                         const SkPaint& paint) {
    AutoPaintFilter apf(this, kVertices_Type, paint);
    this->INHERITED::onDrawVertices(vmode, vertexCount, vertices, texs, colors, xmode, indices,
                                    indexCount, *apf.paint());
}

void SkPaintFilterCanvas::onDrawPatch(const SkPoint cubics[], const SkColor colors[],
                                      const SkPoint texCoords[], SkXfermode* xmode,
                                      const SkPaint& paint) {
    AutoPaintFilter apf(this, kPatch_Type, paint);
    this->INHERITED::onDrawPatch(cubics, colors, texCoords, xmode, *apf.paint());
}

void SkPaintFilterCanvas::onDrawPicture(const SkPicture* picture, const SkMatrix* m,
                                        const SkPaint* paint) {
    AutoPaintFilter apf(this, kPicture_Type, paint);
    this->INHERITED::onDrawPicture(picture, m, apf.paint());
}

void SkPaintFilterCanvas::onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                                     const SkPaint& paint) {
    AutoPaintFilter apf(this, kText_Type, paint);
    this->INHERITED::onDrawText(text, byteLength, x, y, *apf.paint());
}

void SkPaintFilterCanvas::onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                                        const SkPaint& paint) {
    AutoPaintFilter apf(this, kText_Type, paint);
    this->INHERITED::onDrawPosText(text, byteLength, pos, *apf.paint());
}

void SkPaintFilterCanvas::onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                                         SkScalar constY, const SkPaint& paint) {
    AutoPaintFilter apf(this, kText_Type, paint);
    this->INHERITED::onDrawPosTextH(text, byteLength, xpos, constY, *apf.paint());
}

void SkPaintFilterCanvas::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                           const SkMatrix* matrix, const SkPaint& paint) {
    AutoPaintFilter apf(this, kText_Type, paint);
    this->INHERITED::onDrawTextOnPath(text, byteLength, path, matrix, *apf.paint());
}

void SkPaintFilterCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                         const SkPaint& paint) {
    AutoPaintFilter apf(this, kTextBlob_Type, paint);
    this->INHERITED::onDrawTextBlob(blob, x, y, *apf.paint());
}
