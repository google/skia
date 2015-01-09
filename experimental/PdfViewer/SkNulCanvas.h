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

    void beginCommentGroup(const char* description) SK_OVERRIDE {}
    void addComment(const char* kywd, const char* value) SK_OVERRIDE {}
    void endCommentGroup() SK_OVERRIDE {}
    SkDrawFilter* setDrawFilter(SkDrawFilter* filter) SK_OVERRIDE {return NULL;}

    bool isClipEmpty() const SK_OVERRIDE { return false; }
    bool getClipBounds(SkRect* bounds) const SK_OVERRIDE {
        if (NULL != bounds) {
            bounds->setXYWH(0, 0,
                            SkIntToScalar(this->imageInfo().width()),
                            SkIntToScalar(this->imageInfo().height()));
        }
        return true;
    }
    bool getClipDeviceBounds(SkIRect* bounds) const SK_OVERRIDE {
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

    void onClipRect(const SkRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE {}
    void onClipRRect(const SkRRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE {}
    void onClipPath(const SkPath&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE {}
    void onClipRegion(const SkRegion&, SkRegion::Op)  SK_OVERRIDE {}

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) SK_OVERRIDE {}

    void onDrawPaint(const SkPaint& paint) SK_OVERRIDE {}
    void onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                      const SkPaint& paint) SK_OVERRIDE {}
    void onDrawRect(const SkRect& rect, const SkPaint& paint) SK_OVERRIDE {}
    void onDrawOval(const SkRect& oval, const SkPaint&) SK_OVERRIDE {}
    void onDrawRRect(const SkRRect& rrect, const SkPaint& paint) SK_OVERRIDE {}
    void onDrawPath(const SkPath& path, const SkPaint& paint) SK_OVERRIDE {}
    void onDrawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                      const SkPaint* paint = NULL) SK_OVERRIDE {}
    void onDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src,
                          const SkRect& dst,
                          const SkPaint* paint,
                          DrawBitmapRectFlags flags) SK_OVERRIDE {}
    void onDrawImage(const SkImage*, SkScalar left, SkScalar top, const SkPaint*) SK_OVERRIDE {}
    void onDrawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                         const SkPaint*) SK_OVERRIDE{}
    void onDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                          const SkRect& dst, const SkPaint* paint = NULL) SK_OVERRIDE {}
    void onDrawSprite(const SkBitmap& bitmap, int left, int top,
                      const SkPaint* paint = NULL) SK_OVERRIDE {}
    void onDrawVertices(VertexMode vmode, int vertexCount,
                        const SkPoint vertices[], const SkPoint texs[],
                        const SkColor colors[], SkXfermode* xmode,
                        const uint16_t indices[], int indexCount,
                        const SkPaint& paint) SK_OVERRIDE {}

    
private:
    typedef SkCanvas INHERITED;
};

#endif  // SkNulCanvas_DEFINED
