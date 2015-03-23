/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAndroidSDKCanvas_DEFINED
#define SkAndroidSDKCanvas_DEFINED

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRect.h"

/** SkDrawFilter is likely to be deprecated; this is a proxy
    canvas that does the same thing: alter SkPaint fields.

    onDraw*() functions may have their SkPaint modified, and are then
    passed on to the same function on proxyTarget. THIS BREAKS CONSTNESS!

    This still suffers one of the same architectural flaws as SkDrawFilter:
    TextBlob paints are incomplete when filter is called.
*/

class SkAndroidSDKCanvas : public SkCanvas {
public:
    SkAndroidSDKCanvas();
    void reset(SkCanvas* newTarget);

protected:

    // FILTERING

    void onDrawPaint(const SkPaint& paint) SK_OVERRIDE;
    void onDrawPoints(PointMode pMode, size_t count, const SkPoint pts[],
                      const SkPaint& paint) SK_OVERRIDE;
    void onDrawOval(const SkRect& r, const SkPaint& paint) SK_OVERRIDE;
    void onDrawRect(const SkRect& r, const SkPaint& paint) SK_OVERRIDE;
    void onDrawRRect(const SkRRect& r, const SkPaint& paint) SK_OVERRIDE;
    void onDrawPath(const SkPath& path, const SkPaint& paint) SK_OVERRIDE;
    void onDrawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                      const SkPaint* paint) SK_OVERRIDE;
    void onDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                          const SkPaint* paint, DrawBitmapRectFlags flags) SK_OVERRIDE;
    void onDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                          const SkRect& dst, const SkPaint* paint) SK_OVERRIDE;
    void onDrawSprite(const SkBitmap& bitmap, int left, int top,
                      const SkPaint* paint) SK_OVERRIDE;
    void onDrawVertices(VertexMode vMode, int vertexCount, const SkPoint vertices[],
                        const SkPoint texs[], const SkColor colors[], SkXfermode* xMode,
                        const uint16_t indices[], int indexCount,
                        const SkPaint& paint) SK_OVERRIDE;

    void onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                      const SkPaint& paint) SK_OVERRIDE;

    void onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                    const SkPaint& paint) SK_OVERRIDE;
    void onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                       const SkPaint& paint) SK_OVERRIDE;
    void onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                        SkScalar constY, const SkPaint& paint) SK_OVERRIDE;
    void onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                          const SkMatrix* matrix, const SkPaint& paint) SK_OVERRIDE;
    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                        const SkPaint& paint) SK_OVERRIDE;

    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                     const SkPoint texCoords[4], SkXfermode* xmode,
                     const SkPaint& paint) SK_OVERRIDE;

    void onDrawImage(const SkImage*, SkScalar, SkScalar, const SkPaint*) SK_OVERRIDE;
    void onDrawImageRect(const SkImage*, const SkRect*, const SkRect&, const SkPaint*)
        SK_OVERRIDE;
    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*);

    // PASS THROUGH

    void onDrawDrawable(SkDrawable*) SK_OVERRIDE;
    SkISize getBaseLayerSize() const SK_OVERRIDE;
    bool getClipBounds(SkRect*) const SK_OVERRIDE;
    bool getClipDeviceBounds(SkIRect*) const SK_OVERRIDE;
    bool isClipEmpty() const SK_OVERRIDE;
    bool isClipRect() const SK_OVERRIDE;
    SkSurface* onNewSurface(const SkImageInfo&, const SkSurfaceProps&) SK_OVERRIDE;
    const void* onPeekPixels(SkImageInfo*, size_t*) SK_OVERRIDE;
    void* onAccessTopLayerPixels(SkImageInfo*, size_t*) SK_OVERRIDE;
    void willSave() SK_OVERRIDE;
    void willRestore() SK_OVERRIDE;
    void didRestore() SK_OVERRIDE;
    void didConcat(const SkMatrix&) SK_OVERRIDE;
    void didSetMatrix(const SkMatrix&) SK_OVERRIDE;
    void onClipRect(const SkRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    void onClipRRect(const SkRRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    void onClipPath(const SkPath&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    void onClipRegion(const SkRegion&, SkRegion::Op) SK_OVERRIDE;
    void onDiscard() SK_OVERRIDE;

protected:
    SkCanvas* fProxyTarget;
};

#endif  // SkAndroidSDKCanvas_DEFINED

