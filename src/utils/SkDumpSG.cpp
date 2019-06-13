/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkDumpSG.h"
#include "include/core/SkPath.h"

static uint32_t gXformID;

void Xform::addChild(Xform* child) {
    SkASSERT(child);
    SkASSERT(child->fParent.get() == this);
    fChildren.push_back(child);
}

sk_sp<Xform> make_xform(sk_sp<Xform> parent = nullptr) {
    auto xf = sk_sp<Xform>(new Xform);
    xf->fParent = parent;
    xf->fID = ++gXformID;

    if (parent) {
        parent->addChild(xf.get());
    }
    return xf;
}

sk_sp<Shape> SkDumpSG::append(const char content[]) {
    auto sh = this->append(sk_sp<Shape>(new Shape));
    sh->fContent.set(content);
    return sh;
}

void SkDumpSG::dump() {
    SkTArray<Xform*> roots;
}

/////////////////

SkDumpSG::SkDumpSG(int width, int height) : INHERITED(width, height) {
    fStack.push_back(nullptr);
}

void SkDumpSG::willSave() {
    auto top = fStack.back();
    fStack.push_back(top);

    this->INHERITED::willSave();
}

void SkDumpSG::willRestore() {
    fStack.pop_back();
}

void SkDumpSG::didConcat(const SkMatrix& matrix) {
    auto xf = make_xform(fStack.back());
    xf->fContent.set("concat %d", matrix.getType());
    fStack.push_back(xf);

    this->INHERITED::didConcat(matrix);
}

void SkDumpSG::didSetMatrix(const SkMatrix& matrix) {
    SkASSERT(false);
    this->INHERITED::didSetMatrix(matrix);
}

void SkDumpSG::onClipRect(const SkRect& r, SkClipOp op, ClipEdgeStyle edgeStyle) {
    auto xf = make_xform(fStack.back());
    xf->fContent.printf("clipRect {%g %g %g %g}", r.fLeft, r.fTop, r.fRight, r.fBottom);
    fStack.push_back(xf);

    this->INHERITED::onClipRect(r, op, edgeStyle);
}

void SkDumpSG::onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    auto xf = make_xform(fStack.back());
    SkRect r = rrect.getBounds();
    xf->fContent.printf("clipRRect {%g %g %g %g}", r.fLeft, r.fTop, r.fRight, r.fBottom);
    fStack.push_back(xf);

    this->INHERITED::onClipRRect(rrect, op, edgeStyle);
}

void SkDumpSG::onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) {
    auto xf = make_xform(fStack.back());
    SkRect r = path.getBounds();
    xf->fContent.printf("clipPath {%g %g %g %g}", r.fLeft, r.fTop, r.fRight, r.fBottom);
    fStack.push_back(xf);

    this->INHERITED::onClipPath(path, op, edgeStyle);
}

void SkDumpSG::onClipRegion(const SkRegion& deviceRgn, SkClipOp op) {
    SkASSERT(false);
    this->INHERITED::onClipRegion(deviceRgn, op);
}

void SkDumpSG::onDrawPaint(const SkPaint& paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawBehind(const SkPaint& paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                                const SkPaint& paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawOval(const SkRect& rect, const SkPaint& paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawArc(const SkRect& rect, SkScalar startAngle, SkScalar sweepAngle,
                             bool useCenter, const SkPaint& paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawPath(const SkPath& path, const SkPaint& paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                                const SkPaint* paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                                    const SkPaint* paint, SrcRectConstraint constraint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                    const SkRect& dst, const SkPaint* paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawBitmapLattice(const SkBitmap& bitmap, const Lattice& lattice,
                                       const SkRect& dst, const SkPaint* paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawImage(const SkImage* image, SkScalar left, SkScalar top,
                               const SkPaint* paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                   const SkPaint* paint, SrcRectConstraint constraint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawImageNine(const SkImage* image, const SkIRect& center, const SkRect& dst,
                                   const SkPaint* paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawImageLattice(const SkImage* image, const Lattice& lattice,
                                      const SkRect& dst, const SkPaint* paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                  const SkPaint &paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                                 const SkPaint* paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawVerticesObject(const SkVertices* vertices, const SkVertices::Bone bones[],
                                        int boneCount, SkBlendMode bmode, const SkPaint& paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                               const SkPoint texCoords[4], SkBlendMode bmode,
                               const SkPaint& paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawAtlas(const SkImage* image, const SkRSXform xform[], const SkRect tex[],
                               const SkColor colors[], int count, SkBlendMode bmode,
                               const SkRect* cull, const SkPaint* paint) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawAnnotation(const SkRect& rect, const char key[], SkData* data) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4],
                                    QuadAAFlags aa, SkColor color, SkBlendMode mode) {
    this->append("drawPaint");
}

void SkDumpSG::onDrawEdgeAAImageSet(const ImageSetEntry set[], int count,
                                        const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                                        const SkPaint* paint, SrcRectConstraint constraint) {
    this->append("drawPaint");
}
