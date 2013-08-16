
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkNWayCanvas.h"

static SkBitmap make_noconfig_bm(int width, int height) {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kNo_Config, width, height);
    return bm;
}

SkNWayCanvas::SkNWayCanvas(int width, int height)
        : INHERITED(make_noconfig_bm(width, height)) {}

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

int SkNWayCanvas::save(SaveFlags flags) {
    Iter iter(fList);
    while (iter.next()) {
        iter->save(flags);
    }
    return this->INHERITED::save(flags);
}

int SkNWayCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint,
                                    SaveFlags flags) {
    Iter iter(fList);
    while (iter.next()) {
        iter->saveLayer(bounds, paint, flags);
    }
    return this->INHERITED::saveLayer(bounds, paint, flags);
}

void SkNWayCanvas::restore() {
    Iter iter(fList);
    while (iter.next()) {
        iter->restore();
    }
    this->INHERITED::restore();
}

bool SkNWayCanvas::translate(SkScalar dx, SkScalar dy) {
    Iter iter(fList);
    while (iter.next()) {
        iter->translate(dx, dy);
    }
    return this->INHERITED::translate(dx, dy);
}

bool SkNWayCanvas::scale(SkScalar sx, SkScalar sy) {
    Iter iter(fList);
    while (iter.next()) {
        iter->scale(sx, sy);
    }
    return this->INHERITED::scale(sx, sy);
}

bool SkNWayCanvas::rotate(SkScalar degrees) {
    Iter iter(fList);
    while (iter.next()) {
        iter->rotate(degrees);
    }
    return this->INHERITED::rotate(degrees);
}

bool SkNWayCanvas::skew(SkScalar sx, SkScalar sy) {
    Iter iter(fList);
    while (iter.next()) {
        iter->skew(sx, sy);
    }
    return this->INHERITED::skew(sx, sy);
}

bool SkNWayCanvas::concat(const SkMatrix& matrix) {
    Iter iter(fList);
    while (iter.next()) {
        iter->concat(matrix);
    }
    return this->INHERITED::concat(matrix);
}

void SkNWayCanvas::setMatrix(const SkMatrix& matrix) {
    Iter iter(fList);
    while (iter.next()) {
        iter->setMatrix(matrix);
    }
    this->INHERITED::setMatrix(matrix);
}

bool SkNWayCanvas::clipRect(const SkRect& rect, SkRegion::Op op, bool doAA) {
    Iter iter(fList);
    while (iter.next()) {
        iter->clipRect(rect, op, doAA);
    }
    return this->INHERITED::clipRect(rect, op, doAA);
}

bool SkNWayCanvas::clipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA) {
    Iter iter(fList);
    while (iter.next()) {
        iter->clipRRect(rrect, op, doAA);
    }
    return this->INHERITED::clipRRect(rrect, op, doAA);
}

bool SkNWayCanvas::clipPath(const SkPath& path, SkRegion::Op op, bool doAA) {
    Iter iter(fList);
    while (iter.next()) {
        iter->clipPath(path, op, doAA);
    }
    return this->INHERITED::clipPath(path, op, doAA);
}

bool SkNWayCanvas::clipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    Iter iter(fList);
    while (iter.next()) {
        iter->clipRegion(deviceRgn, op);
    }
    return this->INHERITED::clipRegion(deviceRgn, op);
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

void SkNWayCanvas::drawOval(const SkRect& rect, const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawOval(rect, paint);
    }
}

void SkNWayCanvas::drawRect(const SkRect& rect, const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawRect(rect, paint);
    }
}

void SkNWayCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    Iter iter(fList);
    while (iter.next()) {
        iter->drawRRect(rrect, paint);
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
