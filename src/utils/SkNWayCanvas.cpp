/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkNWayCanvas.h"
#include "SkCanvasPriv.h"

SkNWayCanvas::SkNWayCanvas(int width, int height) : INHERITED(width, height) {}

SkNWayCanvas::~SkNWayCanvas() {
    this->removeAll();
}

void SkNWayCanvas::addCanvas(SkCanvas* canvas) {
    if (canvas) {
        *fList.append() = canvas;
    }
}

void SkNWayCanvas::removeCanvas(SkCanvas* canvas) {
    int index = fList.find(canvas);
    if (index >= 0) {
        fList.removeShuffle(index);
    }
}

void SkNWayCanvas::removeAll() {
    fList.reset();
}

///////////////////////////////////////////////////////////////////////////
// These are forwarded to the N canvases we're referencing

class SkNWayCanvas::Iter {
public:
    Iter(const SkTDArray<SkCanvas*>& list) : fList(list) {
        fIndex = 0;
    }
    bool next() {
        if (fIndex < fList.count()) {
            fCanvas = fList[fIndex++];
            return true;
        }
        return false;
    }
    SkCanvas* operator->() { return fCanvas; }
    SkCanvas* get() const { return fCanvas; }

private:
    const SkTDArray<SkCanvas*>& fList;
    int fIndex;
    SkCanvas* fCanvas;
};

void SkNWayCanvas::willSave() {
    Iter iter(fList);
    while (iter.next()) {
        iter->save();
    }

    this->INHERITED::willSave();
}

SkCanvas::SaveLayerStrategy SkNWayCanvas::getSaveLayerStrategy(const SaveLayerRec& rec) {
    Iter iter(fList);
    while (iter.next()) {
        iter->saveLayer(rec);
    }

    this->INHERITED::getSaveLayerStrategy(rec);
    // No need for a layer.
    return kNoLayer_SaveLayerStrategy;
}

bool SkNWayCanvas::onDoSaveBehind(const SkRect* bounds) {
    Iter iter(fList);
    while (iter.next()) {
        SkCanvasPriv::SaveBehind(iter.get(), bounds);
    }
    this->INHERITED::onDoSaveBehind(bounds);
    return false;
}

void SkNWayCanvas::willRestore() {
    Iter iter(fList);
    while (iter.next()) {
        iter->restore();
    }
    this->INHERITED::willRestore();
}

void SkNWayCanvas::didConcat(const SkMatrix& matrix) {
    Iter iter(fList);
    while (iter.next()) {
        iter->concat(matrix);
    }
    this->INHERITED::didConcat(matrix);
}

void SkNWayCanvas::didSetMatrix(const SkMatrix& matrix) {
    Iter iter(fList);
    while (iter.next()) {
        iter->setMatrix(matrix);
    }
    this->INHERITED::didSetMatrix(matrix);
}

void SkNWayCanvas::onClipRect(const SkRect& rect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    Iter iter(fList);
    while (iter.next()) {
        iter->clipRect(rect, op, kSoft_ClipEdgeStyle == edgeStyle);
    }
    this->INHERITED::onClipRect(rect, op, edgeStyle);
}

void SkNWayCanvas::onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    Iter iter(fList);
    while (iter.next()) {
        iter->clipRRect(rrect, op, kSoft_ClipEdgeStyle == edgeStyle);
    }
    this->INHERITED::onClipRRect(rrect, op, edgeStyle);
}

void SkNWayCanvas::onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) {
    Iter iter(fList);
    while (iter.next()) {
        iter->clipPath(path, op, kSoft_ClipEdgeStyle == edgeStyle);
    }
    this->INHERITED::onClipPath(path, op, edgeStyle);
}

void SkNWayCanvas::onClipRegion(const SkRegion& deviceRgn, SkClipOp op) {
    Iter iter(fList);
    while (iter.next()) {
        iter->clipRegion(deviceRgn, op);
    }
    this->INHERITED::onClipRegion(deviceRgn, op);
}

void SkNWayCanvas::onDrawPaint(const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawPaint(paint);
    }
}

void SkNWayCanvas::onDrawBehind(const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        SkCanvasPriv::DrawBehind(iter.get(), paint);
    }
}

void SkNWayCanvas::onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                                const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawPoints(mode, count, pts, paint);
    }
}

void SkNWayCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawRect(rect, paint);
    }
}

void SkNWayCanvas::onDrawEdgeAARect(const SkRect& rect, SkCanvas::QuadAAFlags aa, SkColor color,
                                    SkBlendMode mode) {
    Iter iter(fList);
    while (iter.next()) {
        iter->experimental_DrawEdgeAARectV1(rect, aa, color, mode);
    }
}

void SkNWayCanvas::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawRegion(region, paint);
    }
}

void SkNWayCanvas::onDrawOval(const SkRect& rect, const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawOval(rect, paint);
    }
}

void SkNWayCanvas::onDrawArc(const SkRect& rect, SkScalar startAngle, SkScalar sweepAngle,
                             bool useCenter, const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawArc(rect, startAngle, sweepAngle, useCenter, paint);
    }
}

void SkNWayCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawRRect(rrect, paint);
    }
}

void SkNWayCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawDRRect(outer, inner, paint);
    }
}

void SkNWayCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawPath(path, paint);
    }
}

void SkNWayCanvas::onDrawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                                const SkPaint* paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawBitmap(bitmap, x, y, paint);
    }
}

void SkNWayCanvas::onDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                                    const SkPaint* paint, SrcRectConstraint constraint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->legacy_drawBitmapRect(bitmap, src, dst, paint, (SrcRectConstraint)constraint);
    }
}

void SkNWayCanvas::onDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                    const SkRect& dst, const SkPaint* paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawBitmapNine(bitmap, center, dst, paint);
    }
}

void SkNWayCanvas::onDrawBitmapLattice(const SkBitmap& bitmap, const Lattice& lattice,
                                       const SkRect& dst, const SkPaint* paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawBitmapLattice(bitmap, lattice, dst, paint);
    }
}

void SkNWayCanvas::onDrawImage(const SkImage* image, SkScalar left, SkScalar top,
                               const SkPaint* paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawImage(image, left, top, paint);
    }
}

void SkNWayCanvas::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                   const SkPaint* paint, SrcRectConstraint constraint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->legacy_drawImageRect(image, src, dst, paint, constraint);
    }
}

void SkNWayCanvas::onDrawImageNine(const SkImage* image, const SkIRect& center, const SkRect& dst,
                                   const SkPaint* paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawImageNine(image, center, dst, paint);
    }
}

void SkNWayCanvas::onDrawImageLattice(const SkImage* image, const Lattice& lattice,
                                      const SkRect& dst, const SkPaint* paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawImageLattice(image, lattice, dst, paint);
    }
}

void SkNWayCanvas::onDrawImageSet(const SkCanvas::ImageSetEntry set[], int count,
                                  SkFilterQuality filterQuality, SkBlendMode mode) {
    Iter iter(fList);
    while (iter.next()) {
        iter->experimental_DrawImageSetV1(set, count, filterQuality, mode);
    }
}

void SkNWayCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                  const SkPaint &paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawTextBlob(blob, x, y, paint);
    }
}

void SkNWayCanvas::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                                 const SkPaint* paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawPicture(picture, matrix, paint);
    }
}

void SkNWayCanvas::onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawDrawable(drawable, matrix);
    }
}

void SkNWayCanvas::onDrawVerticesObject(const SkVertices* vertices, const SkVertices::Bone bones[],
                                        int boneCount, SkBlendMode bmode, const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawVertices(vertices, bones, boneCount, bmode, paint);
    }
}

void SkNWayCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                               const SkPoint texCoords[4], SkBlendMode bmode,
                               const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawPatch(cubics, colors, texCoords, bmode, paint);
    }
}

void SkNWayCanvas::onDrawAtlas(const SkImage* image, const SkRSXform xform[], const SkRect tex[],
                               const SkColor colors[], int count, SkBlendMode bmode,
                               const SkRect* cull, const SkPaint* paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawAtlas(image, xform, tex, colors, count, bmode, cull, paint);
    }
}

void SkNWayCanvas::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    Iter iter(fList);
    while (iter.next()) {
        iter->private_draw_shadow_rec(path, rec);
    }
}

void SkNWayCanvas::onDrawAnnotation(const SkRect& rect, const char key[], SkData* data) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawAnnotation(rect, key, data);
    }
}

void SkNWayCanvas::onFlush() {
    Iter iter(fList);
    while (iter.next()) {
        iter->flush();
    }
}
