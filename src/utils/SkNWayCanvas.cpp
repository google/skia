/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkShader.h"
#include "include/utils/SkNWayCanvas.h"
#include "src/core/SkCanvasPriv.h"

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

void SkNWayCanvas::didConcat44(const SkM44& m) {
    Iter iter(fList);
    while (iter.next()) {
        iter->concat(m);
    }
}

void SkNWayCanvas::didSetM44(const SkM44& matrix) {
    Iter iter(fList);
    while (iter.next()) {
        iter->setMatrix(matrix);
    }
}

void SkNWayCanvas::didTranslate(SkScalar x, SkScalar y) {
    Iter iter(fList);
    while (iter.next()) {
        iter->translate(x, y);
    }
}

void SkNWayCanvas::didScale(SkScalar x, SkScalar y) {
    Iter iter(fList);
    while (iter.next()) {
        iter->scale(x, y);
    }
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

void SkNWayCanvas::onClipShader(sk_sp<SkShader> sh, SkClipOp op) {
    Iter iter(fList);
    while (iter.next()) {
        iter->clipShader(sh, op);
    }
    this->INHERITED::onClipShader(std::move(sh), op);
}

void SkNWayCanvas::onClipRegion(const SkRegion& deviceRgn, SkClipOp op) {
    Iter iter(fList);
    while (iter.next()) {
        iter->clipRegion(deviceRgn, op);
    }
    this->INHERITED::onClipRegion(deviceRgn, op);
}

void SkNWayCanvas::onResetClip() {
    Iter iter(fList);
    while (iter.next()) {
        SkCanvasPriv::ResetClip(iter.get());
    }
    this->INHERITED::onResetClip();
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

void SkNWayCanvas::onDrawImage2(const SkImage* image, SkScalar left, SkScalar top,
                                const SkSamplingOptions& sampling, const SkPaint* paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawImage(image, left, top, sampling, paint);
    }
}

void SkNWayCanvas::onDrawImageRect2(const SkImage* image, const SkRect& src, const SkRect& dst,
                                    const SkSamplingOptions& sampling, const SkPaint* paint,
                                    SrcRectConstraint constraint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawImageRect(image, src, dst, sampling, paint, constraint);
    }
}

void SkNWayCanvas::onDrawImageLattice2(const SkImage* image, const Lattice& lattice,
                                       const SkRect& dst, SkFilterMode filter,
                                       const SkPaint* paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawImageLattice(image, lattice, dst, filter, paint);
    }
}

void SkNWayCanvas::onDrawAtlas2(const SkImage* image, const SkRSXform xform[], const SkRect tex[],
                                const SkColor colors[], int count, SkBlendMode bmode,
                                const SkSamplingOptions& sampling, const SkRect* cull,
                                const SkPaint* paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawAtlas(image, xform, tex, colors, count, bmode, sampling, cull, paint);
    }
}

void SkNWayCanvas::onDrawGlyphRunList(const SkGlyphRunList& list, const SkPaint &paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->onDrawGlyphRunList(list, paint);
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

void SkNWayCanvas::onDrawVerticesObject(const SkVertices* vertices,
                                        SkBlendMode bmode, const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawVertices(vertices, bmode, paint);
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

void SkNWayCanvas::onDrawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4],
                                    QuadAAFlags aa, const SkColor4f& color, SkBlendMode mode) {
    Iter iter(fList);
    while (iter.next()) {
        iter->experimental_DrawEdgeAAQuad(rect, clip, aa, color, mode);
    }
}

void SkNWayCanvas::onDrawEdgeAAImageSet2(const ImageSetEntry set[], int count,
                                         const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                                         const SkSamplingOptions& sampling, const SkPaint* paint,
                                         SrcRectConstraint constraint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->experimental_DrawEdgeAAImageSet(
                set, count, dstClips, preViewMatrices, sampling, paint, constraint);
    }
}

void SkNWayCanvas::onFlush() {
    Iter iter(fList);
    while (iter.next()) {
        iter->flush();
    }
}
