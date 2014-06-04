/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNulCanvas_DEFINED
#define SkNulCanvas_DEFINED

#include "SkCanvas.h"

/** \class SkNulCanvas
 *
 *   Nul Canvas is a canvas that does nothing. It is used to measure the perf of just parsing
 *   a pdf, without actually rendering anything.
 *
 */
class SK_API SkNulCanvas : public SkCanvas {
public:
    SK_DECLARE_INST_COUNT(SkNulCanvas);

    SkNulCanvas() {}
    explicit SkNulCanvas(SkBaseDevice* device) : SkCanvas(device) {}

    explicit SkNulCanvas(const SkBitmap& bitmap) : SkCanvas(bitmap) {}
    virtual ~SkNulCanvas() {}

    virtual bool isDrawingToLayer() const SK_OVERRIDE {return false;}
    virtual void clear(SkColor) SK_OVERRIDE {}
    virtual void drawPaint(const SkPaint& paint) SK_OVERRIDE {}
    virtual void drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                            const SkPaint& paint) SK_OVERRIDE {}
    virtual void drawRect(const SkRect& rect, const SkPaint& paint) SK_OVERRIDE {}
    virtual void drawOval(const SkRect& oval, const SkPaint&) SK_OVERRIDE {}
    virtual void drawRRect(const SkRRect& rrect, const SkPaint& paint) SK_OVERRIDE {}
    virtual void drawPath(const SkPath& path, const SkPaint& paint) SK_OVERRIDE {}
    virtual void drawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                            const SkPaint* paint = NULL) SK_OVERRIDE {}
    virtual void drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                      const SkRect& dst,
                                      const SkPaint* paint,
                                      DrawBitmapRectFlags flags) SK_OVERRIDE {}
    virtual void drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m,
                                  const SkPaint* paint = NULL) SK_OVERRIDE {}
    virtual void drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                const SkRect& dst, const SkPaint* paint = NULL) SK_OVERRIDE {}
    virtual void drawSprite(const SkBitmap& bitmap, int left, int top,
                            const SkPaint* paint = NULL) SK_OVERRIDE {}
    virtual void drawVertices(VertexMode vmode, int vertexCount,
                              const SkPoint vertices[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) SK_OVERRIDE {}
    virtual void drawData(const void* data, size_t length) SK_OVERRIDE {}
    virtual void beginCommentGroup(const char* description) SK_OVERRIDE {}
    virtual void addComment(const char* kywd, const char* value) SK_OVERRIDE {}
    virtual void endCommentGroup() SK_OVERRIDE {}
    virtual SkDrawFilter* setDrawFilter(SkDrawFilter* filter) SK_OVERRIDE {return NULL;}

    virtual bool isClipEmpty() const SK_OVERRIDE { return false; }
#ifdef SK_SUPPORT_LEGACY_GETCLIPTYPE
    virtual ClipType getClipType() const SK_OVERRIDE { return kRect_ClipType; }
#endif
    virtual bool getClipBounds(SkRect* bounds) const SK_OVERRIDE {
        if (NULL != bounds) {
            bounds->setXYWH(0, 0,
                            SkIntToScalar(this->imageInfo().fWidth),
                            SkIntToScalar(this->imageInfo().fHeight));
        }
        return true;
    }
    virtual bool getClipDeviceBounds(SkIRect* bounds) const SK_OVERRIDE {
        if (NULL != bounds) {
            bounds->setLargest();
        }
        return true;
    }

protected:
    virtual SkCanvas* canvasForDrawIter() {return NULL;}
    virtual SkBaseDevice* setDevice(SkBaseDevice* device) {return NULL;}

    virtual SaveLayerStrategy willSaveLayer(const SkRect* bounds, const SkPaint* paint,
                                            SaveFlags flags) SK_OVERRIDE {
        this->INHERITED::willSaveLayer(bounds, paint, flags);
        return kNoLayer_SaveLayerStrategy;
    }

    virtual void onDrawText(const void* text, size_t byteLength, SkScalar x,
                          SkScalar y, const SkPaint& paint) SK_OVERRIDE {}
    virtual void onDrawPosText(const void* text, size_t byteLength,
                             const SkPoint pos[], const SkPaint& paint) SK_OVERRIDE {}
    virtual void onDrawPosTextH(const void* text, size_t byteLength,
                              const SkScalar xpos[], SkScalar constY,
                              const SkPaint& paint) SK_OVERRIDE {}
    virtual void onDrawTextOnPath(const void* text, size_t byteLength,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint& paint) SK_OVERRIDE {}

    virtual void onClipRect(const SkRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE {}
    virtual void onClipRRect(const SkRRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE {}
    virtual void onClipPath(const SkPath&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE {}
    virtual void onClipRegion(const SkRegion&, SkRegion::Op)  SK_OVERRIDE {}

    virtual void onDrawPicture(const SkPicture* picture) SK_OVERRIDE {}
    
private:
    typedef SkCanvas INHERITED;
};

#endif  // SkNulCanvas_DEFINED
