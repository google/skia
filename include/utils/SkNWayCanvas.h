
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNWayCanvas_DEFINED
#define SkNWayCanvas_DEFINED

#include "SkCanvas.h"

class SkNWayCanvas : public SkCanvas {
public:
    SkNWayCanvas(int width, int height);
    virtual ~SkNWayCanvas();

    void addCanvas(SkCanvas*);
    void removeCanvas(SkCanvas*);
    void removeAll();

    ///////////////////////////////////////////////////////////////////////////
    // These are forwarded to the N canvases we're referencing

    virtual int save(SaveFlags) SK_OVERRIDE;
    virtual int saveLayer(const SkRect* bounds, const SkPaint*,
                          SaveFlags) SK_OVERRIDE;
    virtual void restore() SK_OVERRIDE;
    virtual bool translate(SkScalar dx, SkScalar dy) SK_OVERRIDE;
    virtual bool scale(SkScalar sx, SkScalar sy) SK_OVERRIDE;
    virtual bool rotate(SkScalar degrees) SK_OVERRIDE;
    virtual bool skew(SkScalar sx, SkScalar sy) SK_OVERRIDE;
    virtual bool concat(const SkMatrix& matrix) SK_OVERRIDE;
    virtual void setMatrix(const SkMatrix& matrix) SK_OVERRIDE;
    virtual bool clipRect(const SkRect&, SkRegion::Op, bool) SK_OVERRIDE;
    virtual bool clipPath(const SkPath&, SkRegion::Op, bool) SK_OVERRIDE;
    virtual bool clipRegion(const SkRegion& deviceRgn,
                            SkRegion::Op) SK_OVERRIDE;

    virtual void drawPaint(const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                            const SkPaint&) SK_OVERRIDE;
    virtual void drawRect(const SkRect& rect, const SkPaint&) SK_OVERRIDE;
    virtual void drawPath(const SkPath& path, const SkPaint&) SK_OVERRIDE;
    virtual void drawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                            const SkPaint*) SK_OVERRIDE;
    virtual void drawBitmapRect(const SkBitmap& bitmap, const SkIRect* src,
                                const SkRect& dst, const SkPaint*) SK_OVERRIDE;
    virtual void drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m,
                                  const SkPaint*) SK_OVERRIDE;
    virtual void drawSprite(const SkBitmap& bitmap, int left, int top,
                            const SkPaint*) SK_OVERRIDE;
    virtual void drawText(const void* text, size_t byteLength, SkScalar x,
                          SkScalar y, const SkPaint&) SK_OVERRIDE;
    virtual void drawPosText(const void* text, size_t byteLength,
                             const SkPoint pos[], const SkPaint&) SK_OVERRIDE;
    virtual void drawPosTextH(const void* text, size_t byteLength,
                              const SkScalar xpos[], SkScalar constY,
                              const SkPaint&) SK_OVERRIDE;
    virtual void drawTextOnPath(const void* text, size_t byteLength,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint&) SK_OVERRIDE;
    virtual void drawPicture(SkPicture&) SK_OVERRIDE;
    virtual void drawVertices(VertexMode vmode, int vertexCount,
                              const SkPoint vertices[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint&) SK_OVERRIDE;

    virtual SkBounder* setBounder(SkBounder*) SK_OVERRIDE;
    virtual SkDrawFilter* setDrawFilter(SkDrawFilter*) SK_OVERRIDE;

private:
    SkTDArray<SkCanvas*> fList;

    class Iter;

    typedef SkCanvas INHERITED;
};


#endif

