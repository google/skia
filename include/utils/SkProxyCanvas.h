
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkProxyCanvas_DEFINED
#define SkProxyCanvas_DEFINED

#include "SkCanvas.h"

/** This class overrides all virtual methods on SkCanvas, and redirects them
    to a "proxy", another SkCanvas instance. It can be the basis for
    intercepting (and possibly modifying) calls to a canvas.

    There must be a proxy installed before the proxycanvas can be used (i.e.
    before its virtual methods can be called).
 */
class SkProxyCanvas : public SkCanvas {
public:
    SkProxyCanvas() : fProxy(NULL) {}
    SkProxyCanvas(SkCanvas* proxy);
    virtual ~SkProxyCanvas();

    SkCanvas*   getProxy() const { return fProxy; }
    void        setProxy(SkCanvas* proxy);

    virtual int save(SaveFlags flags = kMatrixClip_SaveFlag) SK_OVERRIDE;
    virtual int saveLayer(const SkRect* bounds, const SkPaint* paint,
                          SaveFlags flags = kARGB_ClipLayer_SaveFlag) SK_OVERRIDE;
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
                            SkRegion::Op op = SkRegion::kIntersect_Op) SK_OVERRIDE;

    virtual void drawPaint(const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                            const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRect(const SkRect& rect, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPath(const SkPath& path, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                            const SkPaint* paint = NULL) SK_OVERRIDE;
    virtual void drawBitmapRect(const SkBitmap& bitmap, const SkIRect* src,
                                const SkRect& dst, const SkPaint* paint = NULL) SK_OVERRIDE;
    virtual void drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m,
                                  const SkPaint* paint = NULL) SK_OVERRIDE;
    virtual void drawSprite(const SkBitmap& bitmap, int left, int top,
                            const SkPaint* paint = NULL) SK_OVERRIDE;
    virtual void drawText(const void* text, size_t byteLength, SkScalar x,
                          SkScalar y, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPosText(const void* text, size_t byteLength,
                             const SkPoint pos[], const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPosTextH(const void* text, size_t byteLength,
                              const SkScalar xpos[], SkScalar constY,
                              const SkPaint& paint) SK_OVERRIDE;
    virtual void drawTextOnPath(const void* text, size_t byteLength,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPicture(SkPicture&) SK_OVERRIDE;
    virtual void drawVertices(VertexMode vmode, int vertexCount,
                              const SkPoint vertices[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) SK_OVERRIDE;
    virtual void drawData(const void* data, size_t length) SK_OVERRIDE;

    virtual SkBounder* setBounder(SkBounder* bounder) SK_OVERRIDE;
    virtual SkDrawFilter* setDrawFilter(SkDrawFilter* filter) SK_OVERRIDE;

private:
    SkCanvas*   fProxy;

    typedef SkCanvas INHERITED;
};

#endif
