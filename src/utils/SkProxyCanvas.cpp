
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkProxyCanvas.h"

SkProxyCanvas::SkProxyCanvas(SkCanvas* proxy) : fProxy(proxy) {
    SkSafeRef(fProxy);
}

SkProxyCanvas::~SkProxyCanvas() {
    SkSafeUnref(fProxy);
}

void SkProxyCanvas::setProxy(SkCanvas* proxy) {
    SkRefCnt_SafeAssign(fProxy, proxy);
}

///////////////////////////////// Overrides ///////////

void SkProxyCanvas::willSave() {
    fProxy->save();
    this->INHERITED::willSave();
}

SkCanvas::SaveLayerStrategy SkProxyCanvas::willSaveLayer(const SkRect* bounds, const SkPaint* paint,
                                                         SaveFlags flags) {
    fProxy->saveLayer(bounds, paint, flags);
    this->INHERITED::willSaveLayer(bounds, paint, flags);
    // No need for a layer.
    return kNoLayer_SaveLayerStrategy;
}

void SkProxyCanvas::willRestore() {
    fProxy->restore();
    this->INHERITED::willRestore();
}

void SkProxyCanvas::didConcat(const SkMatrix& matrix) {
    fProxy->concat(matrix);
    this->INHERITED::didConcat(matrix);
}

void SkProxyCanvas::didSetMatrix(const SkMatrix& matrix) {
    fProxy->setMatrix(matrix);
    this->INHERITED::didSetMatrix(matrix);
}

void SkProxyCanvas::onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    fProxy->clipRect(rect, op, kSoft_ClipEdgeStyle == edgeStyle);
}

void SkProxyCanvas::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    fProxy->clipRRect(rrect, op, kSoft_ClipEdgeStyle == edgeStyle);
}

void SkProxyCanvas::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    fProxy->clipPath(path, op, kSoft_ClipEdgeStyle == edgeStyle);
}

void SkProxyCanvas::onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    fProxy->clipRegion(deviceRgn, op);
}

void SkProxyCanvas::drawPaint(const SkPaint& paint) {
    fProxy->drawPaint(paint);
}

void SkProxyCanvas::drawPoints(PointMode mode, size_t count,
                               const SkPoint pts[], const SkPaint& paint) {
    fProxy->drawPoints(mode, count, pts, paint);
}

void SkProxyCanvas::drawOval(const SkRect& rect, const SkPaint& paint) {
    fProxy->drawOval(rect, paint);
}

void SkProxyCanvas::drawRect(const SkRect& rect, const SkPaint& paint) {
    fProxy->drawRect(rect, paint);
}

void SkProxyCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    fProxy->drawRRect(rrect, paint);
}

void SkProxyCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                                 const SkPaint& paint) {
    fProxy->drawDRRect(outer, inner, paint);
}

void SkProxyCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    fProxy->drawPath(path, paint);
}

void SkProxyCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                               const SkPaint* paint) {
    fProxy->drawBitmap(bitmap, x, y, paint);
}

void SkProxyCanvas::drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                   const SkRect& dst, const SkPaint* paint,
                                   DrawBitmapRectFlags flags) {
    fProxy->drawBitmapRectToRect(bitmap, src, dst, paint, flags);
}

void SkProxyCanvas::drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m,
                                     const SkPaint* paint) {
    fProxy->drawBitmapMatrix(bitmap, m, paint);
}

void SkProxyCanvas::drawSprite(const SkBitmap& bitmap, int x, int y,
                               const SkPaint* paint) {
    fProxy->drawSprite(bitmap, x, y, paint);
}

void SkProxyCanvas::onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                               const SkPaint& paint) {
    fProxy->drawText(text, byteLength, x, y, paint);
}

void SkProxyCanvas::onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                                  const SkPaint& paint) {
    fProxy->drawPosText(text, byteLength, pos, paint);
}

void SkProxyCanvas::onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                                   SkScalar constY, const SkPaint& paint) {
    fProxy->drawPosTextH(text, byteLength, xpos, constY, paint);
}

void SkProxyCanvas::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                     const SkMatrix* matrix, const SkPaint& paint) {
    fProxy->drawTextOnPath(text, byteLength, path, matrix, paint);
}

void SkProxyCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                   const SkPaint& paint) {
    fProxy->drawTextBlob(blob, x, y, paint);
}

void SkProxyCanvas::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                                  const SkPaint* paint) {
    fProxy->drawPicture(picture, matrix, paint);
}

void SkProxyCanvas::drawVertices(VertexMode vmode, int vertexCount,
                                 const SkPoint vertices[], const SkPoint texs[],
                                 const SkColor colors[], SkXfermode* xmode,
                                 const uint16_t indices[], int indexCount,
                                 const SkPaint& paint) {
    fProxy->drawVertices(vmode, vertexCount, vertices, texs, colors,
                                     xmode, indices, indexCount, paint);
}

void SkProxyCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                                const SkPoint texCoords[4], SkXfermode* xmode,
                                const SkPaint& paint) {
    fProxy->drawPatch(cubics, colors, texCoords, xmode, paint);
}

void SkProxyCanvas::drawData(const void* data, size_t length) {
    fProxy->drawData(data, length);
}

void SkProxyCanvas::beginCommentGroup(const char* description) {
    fProxy->beginCommentGroup(description);
}

void SkProxyCanvas::addComment(const char* kywd, const char* value) {
    fProxy->addComment(kywd, value);
}

void SkProxyCanvas::endCommentGroup() {
    fProxy->endCommentGroup();
}

SkDrawFilter* SkProxyCanvas::setDrawFilter(SkDrawFilter* filter) {
    return fProxy->setDrawFilter(filter);
}
