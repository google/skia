
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
class SK_API SkProxyCanvas : public SkCanvas {
public:
    SkProxyCanvas() : fProxy(NULL) {}
    SkProxyCanvas(SkCanvas* proxy);
    virtual ~SkProxyCanvas();

    SkCanvas*   getProxy() const { return fProxy; }
    void        setProxy(SkCanvas* proxy);

    virtual void drawPaint(const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                            const SkPaint& paint) SK_OVERRIDE;
    virtual void drawOval(const SkRect&, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRect(const SkRect&, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRRect(const SkRRect&, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPath(const SkPath& path, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                            const SkPaint* paint = NULL) SK_OVERRIDE;
    virtual void drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                      const SkRect& dst, const SkPaint* paint,
                                      DrawBitmapRectFlags flags) SK_OVERRIDE;
    virtual void drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m,
                                  const SkPaint* paint = NULL) SK_OVERRIDE;
    virtual void drawSprite(const SkBitmap& bitmap, int left, int top,
                            const SkPaint* paint = NULL) SK_OVERRIDE;
    virtual void drawVertices(VertexMode vmode, int vertexCount,
                              const SkPoint vertices[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) SK_OVERRIDE;
    virtual void drawData(const void* data, size_t length) SK_OVERRIDE;

    virtual void beginCommentGroup(const char* description) SK_OVERRIDE;
    virtual void addComment(const char* kywd, const char* value) SK_OVERRIDE;
    virtual void endCommentGroup() SK_OVERRIDE;

    virtual SkDrawFilter* setDrawFilter(SkDrawFilter* filter) SK_OVERRIDE;

protected:
    virtual void willSave() SK_OVERRIDE;
    virtual SaveLayerStrategy willSaveLayer(const SkRect*, const SkPaint*, SaveFlags) SK_OVERRIDE;
    virtual void willRestore() SK_OVERRIDE;

    virtual void didConcat(const SkMatrix&) SK_OVERRIDE;
    virtual void didSetMatrix(const SkMatrix&) SK_OVERRIDE;

    virtual void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) SK_OVERRIDE;
    virtual void onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                            const SkPaint&) SK_OVERRIDE;
    virtual void onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                               const SkPaint&) SK_OVERRIDE;
    virtual void onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                                SkScalar constY, const SkPaint&) SK_OVERRIDE;
    virtual void onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                  const SkMatrix* matrix, const SkPaint&) SK_OVERRIDE;

    virtual void onClipRect(const SkRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipRRect(const SkRRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipPath(const SkPath&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipRegion(const SkRegion&, SkRegion::Op) SK_OVERRIDE;

    virtual void onDrawPicture(const SkPicture*) SK_OVERRIDE;

private:
    SkCanvas*   fProxy;

    typedef SkCanvas INHERITED;
};

#endif
