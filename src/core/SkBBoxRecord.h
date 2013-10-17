
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBBoxRecord_DEFINED
#define SkBBoxRecord_DEFINED

#include "SkPictureRecord.h"

/**
  * This is an abstract SkPictureRecord subclass that intercepts draw calls and computes an
  * axis-aligned bounding box for each draw that it sees, subclasses implement handleBBox()
  * which will be called every time we get a new bounding box.
  */
class SkBBoxRecord : public SkPictureRecord {
public:

    SkBBoxRecord(uint32_t recordFlags, SkBaseDevice* device)
            : INHERITED(recordFlags, device) { }
    virtual ~SkBBoxRecord() { }

    /**
     * This is called each time we get a bounding box, it will be axis-aligned,
     * in device coordinates, and expanded to include stroking, shadows, etc.
     */
    virtual void handleBBox(const SkRect& bbox) = 0;

    virtual void drawOval(const SkRect& rect, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRRect(const SkRRect& rrect, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRect(const SkRect& rect, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPath(const SkPath& path, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                            const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPaint(const SkPaint& paint) SK_OVERRIDE;
    virtual void clear(SkColor) SK_OVERRIDE;
    virtual void drawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                          const SkPaint& paint) SK_OVERRIDE;
    virtual void drawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                            const SkPaint* paint = NULL) SK_OVERRIDE;
    virtual void drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                      const SkRect& dst, const SkPaint* paint,
                                      DrawBitmapRectFlags flags) SK_OVERRIDE;
    virtual void drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& mat,
                                  const SkPaint* paint) SK_OVERRIDE;
    virtual void drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                const SkRect& dst, const SkPaint* paint) SK_OVERRIDE;
    virtual void drawPosText(const void* text, size_t byteLength,
                             const SkPoint pos[], const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPosTextH(const void* text, size_t byteLength,
                              const SkScalar xpos[], SkScalar constY,
                              const SkPaint& paint) SK_OVERRIDE;
    virtual void drawSprite(const SkBitmap& bitmap, int left, int top,
                            const SkPaint* paint) SK_OVERRIDE;
    virtual void drawTextOnPath(const void* text, size_t byteLength,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint& paint) SK_OVERRIDE;
    virtual void drawVertices(VertexMode mode, int vertexCount,
                              const SkPoint vertices[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xfer,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPicture(SkPicture& picture) SK_OVERRIDE;

private:
    /**
     * Takes a bounding box in current canvas view space, accounts for stroking and effects, and
     * computes an axis-aligned bounding box in device coordinates, then passes it to handleBBox()
     * returns false if the draw is completely clipped out, and may safely be ignored.
     **/
    bool transformBounds(const SkRect& bounds, const SkPaint* paint);

    typedef SkPictureRecord INHERITED;
};

#endif
