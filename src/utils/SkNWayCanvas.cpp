
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkNWayCanvas.h"

SkNWayCanvas::SkNWayCanvas(int width, int height)
        : INHERITED(width, height) {}

SkNWayCanvas::~SkNWayCanvas() {
    this->removeAll();
}

void SkNWayCanvas::addCanvas(SkCanvas* canvas) {
    if (canvas) {
        canvas->ref();
        *fList.append() = canvas;
    }
}

void SkNWayCanvas::removeCanvas(SkCanvas* canvas) {
    int index = fList.find(canvas);
    if (index >= 0) {
        canvas->unref();
        fList.removeShuffle(index);
    }
}

void SkNWayCanvas::removeAll() {
    fList.unrefAll();
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

private:
    const SkTDArray<SkCanvas*>& fList;
    int fIndex;
    SkCanvas* fCanvas;
};

void SkNWayCanvas::willSave(SaveFlags flags) {
    Iter iter(fList);
    while (iter.next()) {
        iter->save(flags);
    }

    this->INHERITED::willSave(flags);
}

SkCanvas::SaveLayerStrategy SkNWayCanvas::willSaveLayer(const SkRect* bounds, const SkPaint* paint,
                                                        SaveFlags flags) {
    Iter iter(fList);
    while (iter.next()) {
        iter->saveLayer(bounds, paint, flags);
    }

    this->INHERITED::willSaveLayer(bounds, paint, flags);
    // No need for a layer.
    return kNoLayer_SaveLayerStrategy;
}

void SkNWayCanvas::willRestore() {
    Iter iter(fList);
    while (iter.next()) {
        iter->restore();
    }
    this->INHERITED::willRestore();
}

void SkNWayCanvas::didTranslate(SkScalar dx, SkScalar dy) {
    Iter iter(fList);
    while (iter.next()) {
        iter->translate(dx, dy);
    }
    this->INHERITED::didTranslate(dx, dy);
}

void SkNWayCanvas::didScale(SkScalar sx, SkScalar sy) {
    Iter iter(fList);
    while (iter.next()) {
        iter->scale(sx, sy);
    }
    this->INHERITED::didScale(sx, sy);
}

void SkNWayCanvas::didRotate(SkScalar degrees) {
    Iter iter(fList);
    while (iter.next()) {
        iter->rotate(degrees);
    }
    this->INHERITED::didRotate(degrees);
}

void SkNWayCanvas::didSkew(SkScalar sx, SkScalar sy) {
    Iter iter(fList);
    while (iter.next()) {
        iter->skew(sx, sy);
    }
    this->INHERITED::didSkew(sx, sy);
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

void SkNWayCanvas::onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    Iter iter(fList);
    while (iter.next()) {
        iter->clipRect(rect, op, kSoft_ClipEdgeStyle == edgeStyle);
    }
    this->INHERITED::onClipRect(rect, op, edgeStyle);
}

void SkNWayCanvas::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    Iter iter(fList);
    while (iter.next()) {
        iter->clipRRect(rrect, op, kSoft_ClipEdgeStyle == edgeStyle);
    }
    this->INHERITED::onClipRRect(rrect, op, edgeStyle);
}

void SkNWayCanvas::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    Iter iter(fList);
    while (iter.next()) {
        iter->clipPath(path, op, kSoft_ClipEdgeStyle == edgeStyle);
    }
    this->INHERITED::onClipPath(path, op, edgeStyle);
}

void SkNWayCanvas::onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    Iter iter(fList);
    while (iter.next()) {
        iter->clipRegion(deviceRgn, op);
    }
    this->INHERITED::onClipRegion(deviceRgn, op);
}

void SkNWayCanvas::clear(SkColor color) {
    Iter iter(fList);
    while (iter.next()) {
        iter->clear(color);
    }
}

void SkNWayCanvas::drawPaint(const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawPaint(paint);
    }
}

void SkNWayCanvas::drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                        const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawPoints(mode, count, pts, paint);
    }
}

void SkNWayCanvas::drawRect(const SkRect& rect, const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawRect(rect, paint);
    }
}

void SkNWayCanvas::drawOval(const SkRect& rect, const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawOval(rect, paint);
    }
}

void SkNWayCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawRRect(rrect, paint);
    }
}

void SkNWayCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                                const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawDRRect(outer, inner, paint);
    }
}

void SkNWayCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawPath(path, paint);
    }
}

void SkNWayCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                              const SkPaint* paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawBitmap(bitmap, x, y, paint);
    }
}

void SkNWayCanvas::drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                  const SkRect& dst, const SkPaint* paint,
                                  DrawBitmapRectFlags flags) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawBitmapRectToRect(bitmap, src, dst, paint, flags);
    }
}

void SkNWayCanvas::drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m,
                                    const SkPaint* paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawBitmapMatrix(bitmap, m, paint);
    }
}

void SkNWayCanvas::drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                  const SkRect& dst, const SkPaint* paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawBitmapNine(bitmap, center, dst, paint);
    }
}

void SkNWayCanvas::drawSprite(const SkBitmap& bitmap, int x, int y,
                              const SkPaint* paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawSprite(bitmap, x, y, paint);
    }
}

void SkNWayCanvas::drawText(const void* text, size_t byteLength, SkScalar x,
                            SkScalar y, const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawText(text, byteLength, x, y, paint);
    }
}

void SkNWayCanvas::drawPosText(const void* text, size_t byteLength,
                               const SkPoint pos[], const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawPosText(text, byteLength, pos, paint);
    }
}

void SkNWayCanvas::drawPosTextH(const void* text, size_t byteLength,
                                const SkScalar xpos[], SkScalar constY,
                                const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawPosTextH(text, byteLength, xpos, constY, paint);
    }
}

void SkNWayCanvas::drawTextOnPath(const void* text, size_t byteLength,
                                  const SkPath& path, const SkMatrix* matrix,
                                  const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawTextOnPath(text, byteLength, path, matrix, paint);
    }
}

void SkNWayCanvas::drawPicture(SkPicture& picture) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawPicture(picture);
    }
}

void SkNWayCanvas::drawVertices(VertexMode vmode, int vertexCount,
                          const SkPoint vertices[], const SkPoint texs[],
                          const SkColor colors[], SkXfermode* xmode,
                          const uint16_t indices[], int indexCount,
                          const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawVertices(vmode, vertexCount, vertices, texs, colors, xmode,
                           indices, indexCount, paint);
    }
}

void SkNWayCanvas::drawData(const void* data, size_t length) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawData(data, length);
    }
}

SkBounder* SkNWayCanvas::setBounder(SkBounder* bounder) {
    Iter iter(fList);
    while (iter.next()) {
        iter->setBounder(bounder);
    }
    return this->INHERITED::setBounder(bounder);
}

SkDrawFilter* SkNWayCanvas::setDrawFilter(SkDrawFilter* filter) {
    Iter iter(fList);
    while (iter.next()) {
        iter->setDrawFilter(filter);
    }
    return this->INHERITED::setDrawFilter(filter);
}

void SkNWayCanvas::beginCommentGroup(const char* description) {
    Iter iter(fList);
    while (iter.next()) {
        iter->beginCommentGroup(description);
    }
}

void SkNWayCanvas::addComment(const char* kywd, const char* value) {
    Iter iter(fList);
    while (iter.next()) {
        iter->addComment(kywd, value);
    }
}

void SkNWayCanvas::endCommentGroup() {
    Iter iter(fList);
    while (iter.next()) {
        iter->endCommentGroup();
    }
}
