
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

int SkProxyCanvas::save(SaveFlags flags) {
    return fProxy->save(flags);
}

int SkProxyCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint,
                             SaveFlags flags) {
    return fProxy->saveLayer(bounds, paint, flags);
}

void SkProxyCanvas::restore() {
    fProxy->restore();
}

bool SkProxyCanvas::translate(SkScalar dx, SkScalar dy) {
    return fProxy->translate(dx, dy);
}

bool SkProxyCanvas::scale(SkScalar sx, SkScalar sy) {
    return fProxy->scale(sx, sy);
}

bool SkProxyCanvas::rotate(SkScalar degrees) {
    return fProxy->rotate(degrees);
}

bool SkProxyCanvas::skew(SkScalar sx, SkScalar sy) {
    return fProxy->skew(sx, sy);
}

bool SkProxyCanvas::concat(const SkMatrix& matrix) {
    return fProxy->concat(matrix);
}

void SkProxyCanvas::setMatrix(const SkMatrix& matrix) {
    fProxy->setMatrix(matrix);
}

bool SkProxyCanvas::clipRect(const SkRect& rect, SkRegion::Op op, bool doAA) {
    return fProxy->clipRect(rect, op, doAA);
}

bool SkProxyCanvas::clipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA) {
    return fProxy->clipRRect(rrect, op, doAA);
}

bool SkProxyCanvas::clipPath(const SkPath& path, SkRegion::Op op, bool doAA) {
    return fProxy->clipPath(path, op, doAA);
}

bool SkProxyCanvas::clipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    return fProxy->clipRegion(deviceRgn, op);
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

void SkProxyCanvas::drawText(const void* text, size_t byteLength, SkScalar x,
                             SkScalar y, const SkPaint& paint) {
    fProxy->drawText(text, byteLength, x, y, paint);
}

void SkProxyCanvas::drawPosText(const void* text, size_t byteLength,
                                const SkPoint pos[], const SkPaint& paint) {
    fProxy->drawPosText(text, byteLength, pos, paint);
}

void SkProxyCanvas::drawPosTextH(const void* text, size_t byteLength,
                                 const SkScalar xpos[], SkScalar constY,
                                 const SkPaint& paint) {
    fProxy->drawPosTextH(text, byteLength, xpos, constY, paint);
}

void SkProxyCanvas::drawTextOnPath(const void* text, size_t byteLength,
                                   const SkPath& path, const SkMatrix* matrix,
                                   const SkPaint& paint) {
    fProxy->drawTextOnPath(text, byteLength, path, matrix, paint);
}

void SkProxyCanvas::drawPicture(SkPicture& picture) {
    fProxy->drawPicture(picture);
}

void SkProxyCanvas::drawVertices(VertexMode vmode, int vertexCount,
                                 const SkPoint vertices[], const SkPoint texs[],
                                 const SkColor colors[], SkXfermode* xmode,
                                 const uint16_t indices[], int indexCount,
                                 const SkPaint& paint) {
    fProxy->drawVertices(vmode, vertexCount, vertices, texs, colors,
                                     xmode, indices, indexCount, paint);
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

SkBounder* SkProxyCanvas::setBounder(SkBounder* bounder) {
    return fProxy->setBounder(bounder);
}

SkDrawFilter* SkProxyCanvas::setDrawFilter(SkDrawFilter* filter) {
    return fProxy->setDrawFilter(filter);
}
