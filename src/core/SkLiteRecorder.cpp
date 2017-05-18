/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLiteDL.h"
#include "SkLiteRecorder.h"
#include "SkSurface.h"

SkLiteRecorder::SkLiteRecorder()
    : INHERITED(1, 1)
    , fDL(nullptr) {}

void SkLiteRecorder::reset(SkLiteDL* dl, const SkIRect& bounds) {
    this->resetCanvas(bounds.right(), bounds.bottom());
    fDL = dl;
}

sk_sp<SkSurface> SkLiteRecorder::onNewSurface(const SkImageInfo&, const SkSurfaceProps&) {
    return nullptr;
}

#ifdef SK_SUPPORT_LEGACY_DRAWFILTER
SkDrawFilter* SkLiteRecorder::setDrawFilter(SkDrawFilter* df) {
    fDL->setDrawFilter(df);
    return this->INHERITED::setDrawFilter(df);
}
#endif

void SkLiteRecorder::willSave() { fDL->save(); }
SkCanvas::SaveLayerStrategy SkLiteRecorder::getSaveLayerStrategy(const SaveLayerRec& rec) {
    fDL->saveLayer(rec.fBounds, rec.fPaint, rec.fBackdrop, rec.fClipMask, rec.fClipMatrix,
                   rec.fSaveLayerFlags);
    return SkCanvas::kNoLayer_SaveLayerStrategy;
}
void SkLiteRecorder::willRestore() { fDL->restore(); }

void SkLiteRecorder::didConcat   (const SkMatrix& matrix)   { fDL->   concat(matrix); }
void SkLiteRecorder::didSetMatrix(const SkMatrix& matrix)   { fDL->setMatrix(matrix); }
void SkLiteRecorder::didTranslate(SkScalar dx, SkScalar dy) { fDL->translate(dx, dy); }

void SkLiteRecorder::onClipRect(const SkRect& rect, SkClipOp op, ClipEdgeStyle style) {
    fDL->clipRect(rect, op, style==kSoft_ClipEdgeStyle);
    this->INHERITED::onClipRect(rect, op, style);
}
void SkLiteRecorder::onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle style) {
    fDL->clipRRect(rrect, op, style==kSoft_ClipEdgeStyle);
    this->INHERITED::onClipRRect(rrect, op, style);
}
void SkLiteRecorder::onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle style) {
    fDL->clipPath(path, op, style==kSoft_ClipEdgeStyle);
    this->INHERITED::onClipPath(path, op, style);
}
void SkLiteRecorder::onClipRegion(const SkRegion& region, SkClipOp op) {
    fDL->clipRegion(region, op);
    this->INHERITED::onClipRegion(region, op);
}

void SkLiteRecorder::onDrawPaint(const SkPaint& paint) {
    fDL->drawPaint(paint);
}
void SkLiteRecorder::onDrawPath(const SkPath& path, const SkPaint& paint) {
    fDL->drawPath(path, paint);
}
void SkLiteRecorder::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    fDL->drawRect(rect, paint);
}
void SkLiteRecorder::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    fDL->drawRegion(region, paint);
}
void SkLiteRecorder::onDrawOval(const SkRect& oval, const SkPaint& paint) {
    fDL->drawOval(oval, paint);
}
void SkLiteRecorder::onDrawArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                               bool useCenter, const SkPaint& paint) {
    fDL->drawArc(oval, startAngle, sweepAngle, useCenter, paint);
}
void SkLiteRecorder::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    fDL->drawRRect(rrect, paint);
}
void SkLiteRecorder::onDrawDRRect(const SkRRect& out, const SkRRect& in, const SkPaint& paint) {
    fDL->drawDRRect(out, in, paint);
}

void SkLiteRecorder::onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    fDL->drawDrawable(drawable, matrix);
}
void SkLiteRecorder::onDrawPicture(const SkPicture* picture,
                                   const SkMatrix* matrix,
                                   const SkPaint* paint) {
    fDL->drawPicture(picture, matrix, paint);
}
void SkLiteRecorder::onDrawAnnotation(const SkRect& rect, const char key[], SkData* val) {
    fDL->drawAnnotation(rect, key, val);
}

void SkLiteRecorder::onDrawText(const void* text, size_t bytes,
                                SkScalar x, SkScalar y,
                                const SkPaint& paint) {
    fDL->drawText(text, bytes, x, y, paint);
}
void SkLiteRecorder::onDrawPosText(const void* text, size_t bytes,
                                   const SkPoint pos[],
                                   const SkPaint& paint) {
    fDL->drawPosText(text, bytes, pos, paint);
}
void SkLiteRecorder::onDrawPosTextH(const void* text, size_t bytes,
                                    const SkScalar xs[], SkScalar y,
                                    const SkPaint& paint) {
    fDL->drawPosTextH(text, bytes, xs, y, paint);
}
void SkLiteRecorder::onDrawTextOnPath(const void* text, size_t bytes,
                                      const SkPath& path, const SkMatrix* matrix,
                                      const SkPaint& paint) {
    fDL->drawTextOnPath(text, bytes, path, matrix, paint);
}
void SkLiteRecorder::onDrawTextRSXform(const void* text, size_t bytes,
                                       const SkRSXform xform[], const SkRect* cull,
                                       const SkPaint& paint) {
    fDL->drawTextRSXform(text, bytes, xform, cull, paint);
}
void SkLiteRecorder::onDrawTextBlob(const SkTextBlob* blob,
                                    SkScalar x, SkScalar y,
                                    const SkPaint& paint) {
    fDL->drawTextBlob(blob, x,y, paint);
}

void SkLiteRecorder::onDrawBitmap(const SkBitmap& bm,
                                  SkScalar x, SkScalar y,
                                  const SkPaint* paint) {
    fDL->drawImage(SkImage::MakeFromBitmap(bm), x,y, paint);
}
void SkLiteRecorder::onDrawBitmapNine(const SkBitmap& bm,
                                      const SkIRect& center, const SkRect& dst,
                                      const SkPaint* paint) {
    fDL->drawImageNine(SkImage::MakeFromBitmap(bm), center, dst, paint);
}
void SkLiteRecorder::onDrawBitmapRect(const SkBitmap& bm,
                                      const SkRect* src, const SkRect& dst,
                                      const SkPaint* paint, SrcRectConstraint constraint) {
    fDL->drawImageRect(SkImage::MakeFromBitmap(bm), src, dst, paint, constraint);
}
void SkLiteRecorder::onDrawBitmapLattice(const SkBitmap& bm,
                                         const SkCanvas::Lattice& lattice, const SkRect& dst,
                                         const SkPaint* paint) {
    fDL->drawImageLattice(SkImage::MakeFromBitmap(bm), lattice, dst, paint);
}

void SkLiteRecorder::onDrawImage(const SkImage* img,
                                  SkScalar x, SkScalar y,
                                  const SkPaint* paint) {
    fDL->drawImage(sk_ref_sp(img), x,y, paint);
}
void SkLiteRecorder::onDrawImageNine(const SkImage* img,
                                      const SkIRect& center, const SkRect& dst,
                                      const SkPaint* paint) {
    fDL->drawImageNine(sk_ref_sp(img), center, dst, paint);
}
void SkLiteRecorder::onDrawImageRect(const SkImage* img,
                                      const SkRect* src, const SkRect& dst,
                                      const SkPaint* paint, SrcRectConstraint constraint) {
    fDL->drawImageRect(sk_ref_sp(img), src, dst, paint, constraint);
}
void SkLiteRecorder::onDrawImageLattice(const SkImage* img,
                                        const SkCanvas::Lattice& lattice, const SkRect& dst,
                                        const SkPaint* paint) {
    fDL->drawImageLattice(sk_ref_sp(img), lattice, dst, paint);
}


void SkLiteRecorder::onDrawPatch(const SkPoint cubics[12],
                                 const SkColor colors[4], const SkPoint texCoords[4],
                                 SkBlendMode bmode, const SkPaint& paint) {
    fDL->drawPatch(cubics, colors, texCoords, bmode, paint);
}
void SkLiteRecorder::onDrawPoints(SkCanvas::PointMode mode,
                                  size_t count, const SkPoint pts[],
                                  const SkPaint& paint) {
    fDL->drawPoints(mode, count, pts, paint);
}
void SkLiteRecorder::onDrawVerticesObject(const SkVertices* vertices, SkBlendMode mode,
                                          const SkPaint& paint) {
    fDL->drawVertices(vertices, mode, paint);
}
void SkLiteRecorder::onDrawAtlas(const SkImage* atlas,
                                 const SkRSXform xforms[],
                                 const SkRect texs[],
                                 const SkColor colors[],
                                 int count,
                                 SkBlendMode bmode,
                                 const SkRect* cull,
                                 const SkPaint* paint) {
    fDL->drawAtlas(atlas, xforms, texs, colors, count, bmode, cull, paint);
}
void SkLiteRecorder::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    fDL->drawShadowRec(path, rec);
}
