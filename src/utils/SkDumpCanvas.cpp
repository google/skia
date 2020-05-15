/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkShader.h"
#include "include/core/SkTextBlob.h"
#include "include/utils/SkDumpCanvas.h"

SkDumpCanvas::SkDumpCanvas(int width, int height) : INHERITED(width, height) {}

SkDumpCanvas::~SkDumpCanvas() {
}

static void missing() {
    SkDebugf("SkDebugCanvas missing\n");
    SkASSERT(false);
}

void SkDumpCanvas::willSave() {
    SkDebugf("save\n");

    this->INHERITED::willSave();
}

SkCanvas::SaveLayerStrategy SkDumpCanvas::getSaveLayerStrategy(const SaveLayerRec& rec) {
    SkDebugf("savelayer\n");

    this->INHERITED::getSaveLayerStrategy(rec);
    // No need for a layer.
    return kNoLayer_SaveLayerStrategy;
}

bool SkDumpCanvas::onDoSaveBehind(const SkRect* bounds) {
    missing();
    this->INHERITED::onDoSaveBehind(bounds);
    return false;
}

void SkDumpCanvas::willRestore() {
    SkDebugf("restore\n");

    this->INHERITED::willRestore();
}

void SkDumpCanvas::onMarkCTM(const char* name) {
    missing();
    this->INHERITED::onMarkCTM(name);
}

void SkDumpCanvas::didConcat44(const SkM44& m) {
    SkDebugf("didCocnat44\n");
}

void SkDumpCanvas::didConcat(const SkMatrix& matrix) {
    SkDebugf("didConcat\n");
}

void SkDumpCanvas::didSetMatrix(const SkMatrix& matrix) {
    missing();
}

void SkDumpCanvas::didTranslate(SkScalar x, SkScalar y) {
    SkDebugf("translate %g %g\n", x, y);
}

void SkDumpCanvas::didScale(SkScalar x, SkScalar y) {
    SkDebugf("scale %g %g\n", x, y);
}

void SkDumpCanvas::onClipRect(const SkRect& rect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    SkDebugf("clipRect [%g %g %g %g]\n",
             rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
    this->INHERITED::onClipRect(rect, op, edgeStyle);
}

void SkDumpCanvas::onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    missing();
    this->INHERITED::onClipRRect(rrect, op, edgeStyle);
}

void SkDumpCanvas::onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) {
    missing();
    this->INHERITED::onClipPath(path, op, edgeStyle);
}

void SkDumpCanvas::onClipShader(sk_sp<SkShader> sh, SkClipOp op) {
    missing();
    this->INHERITED::onClipShader(std::move(sh), op);
}

void SkDumpCanvas::onClipRegion(const SkRegion& deviceRgn, SkClipOp op) {
    missing();
    this->INHERITED::onClipRegion(deviceRgn, op);
}

void SkDumpCanvas::onDrawPaint(const SkPaint& paint) {
    missing();
}

void SkDumpCanvas::onDrawBehind(const SkPaint& paint) {
    missing();
}

void SkDumpCanvas::onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                                const SkPaint& paint) {
    SkDebugf("drawpoints %d\n", count);
}

void SkDumpCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    missing();
}

void SkDumpCanvas::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    missing();
}

void SkDumpCanvas::onDrawOval(const SkRect& rect, const SkPaint& paint) {
    missing();
}

void SkDumpCanvas::onDrawArc(const SkRect& rect, SkScalar startAngle, SkScalar sweepAngle,
                             bool useCenter, const SkPaint& paint) {
    missing();
}

void SkDumpCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    missing();
}

void SkDumpCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    missing();
}

void SkDumpCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    auto r = path.getBounds();
    SkDebugf("drawPath [%g %g %g %g] paint: alpha %d, width %g, pe %p\n",
             r.fLeft, r.fTop, r.fRight, r.fBottom,
             paint.getAlpha(), paint.getStrokeWidth(), paint.getPathEffect());
}

void SkDumpCanvas::onDrawImage(const SkImage* image, SkScalar left, SkScalar top,
                               const SkPaint* paint) {
    missing();
}

void SkDumpCanvas::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                   const SkPaint* paint, SrcRectConstraint constraint) {
    missing();
}

void SkDumpCanvas::onDrawImageNine(const SkImage* image, const SkIRect& center, const SkRect& dst,
                                   const SkPaint* paint) {
    missing();
}

void SkDumpCanvas::onDrawImageLattice(const SkImage* image, const Lattice& lattice,
                                      const SkRect& dst, const SkPaint* paint) {
    missing();
}

void SkDumpCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                  const SkPaint &paint) {
    auto r = blob->bounds();
    SkDebugf("textblob %g %g [%g %g %g %g]\n", x, y,
             r.fLeft, r.fTop, r.fRight, r.fBottom);
    blob->dump();
}

void SkDumpCanvas::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                                 const SkPaint* paint) {
    missing();
}

void SkDumpCanvas::onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    missing();
}

void SkDumpCanvas::onDrawVerticesObject(const SkVertices* vertices,
                                        SkBlendMode bmode, const SkPaint& paint) {
    missing();
}

void SkDumpCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                               const SkPoint texCoords[4], SkBlendMode bmode,
                               const SkPaint& paint) {
    missing();
}

void SkDumpCanvas::onDrawAtlas(const SkImage* image, const SkRSXform xform[], const SkRect tex[],
                               const SkColor colors[], int count, SkBlendMode bmode,
                               const SkRect* cull, const SkPaint* paint) {
    missing();
}

void SkDumpCanvas::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    missing();
}

void SkDumpCanvas::onDrawAnnotation(const SkRect& rect, const char key[], SkData* data) {
    missing();
}

void SkDumpCanvas::onDrawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4],
                                    QuadAAFlags aa, const SkColor4f& color, SkBlendMode mode) {
    missing();
}

void SkDumpCanvas::onDrawEdgeAAImageSet(const ImageSetEntry set[], int count,
                                        const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                                        const SkPaint* paint, SrcRectConstraint constraint) {
    missing();
}

void SkDumpCanvas::onFlush() {
    missing();
}
