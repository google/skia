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

    void onDrawPaint(const SkPaint& paint) override;
    void onDrawPoints(PointMode pMode, size_t count, const SkPoint pts[],
                      const SkPaint& paint) override;
    void onDrawOval(const SkRect& r, const SkPaint& paint) override;
    void onDrawRect(const SkRect& r, const SkPaint& paint) override;
    void onDrawRRect(const SkRRect& r, const SkPaint& paint) override;
    void onDrawPath(const SkPath& path, const SkPaint& paint) override;
    void onDrawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                      const SkPaint* paint) override;
    void onDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                          const SkPaint* paint, SkCanvas::SrcRectConstraint) override;
    void onDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                          const SkRect& dst, const SkPaint* paint) override;
    void onDrawVertices(VertexMode vMode, int vertexCount, const SkPoint vertices[],
                        const SkPoint texs[], const SkColor colors[], SkXfermode* xMode,
                        const uint16_t indices[], int indexCount,
                        const SkPaint& paint) override;

    void onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                      const SkPaint& paint) override;

    void onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                    const SkPaint& paint) override;
    void onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                       const SkPaint& paint) override;
    void onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                        SkScalar constY, const SkPaint& paint) override;
    void onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                          const SkMatrix* matrix, const SkPaint& paint) override;
    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                        const SkPaint& paint) override;

    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                     const SkPoint texCoords[4], SkXfermode* xmode,
                     const SkPaint& paint) override;

    void onDrawImage(const SkImage*, SkScalar, SkScalar, const SkPaint*) override;
    void onDrawImageRect(const SkImage*, const SkRect*, const SkRect&, const SkPaint*,
                         SrcRectConstraint) override;
    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*);
    void onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[],
                     const SkColor[], int count, SkXfermode::Mode,
                     const SkRect* cull, const SkPaint*) override;
    void onDrawImageNine(const SkImage*, const SkIRect& center,
                         const SkRect& dst, const SkPaint*) override;

    // PASS THROUGH

    void onDrawDrawable(SkDrawable*, const SkMatrix*) override;
    SkISize getBaseLayerSize() const override;
    bool getClipBounds(SkRect*) const override;
    bool getClipDeviceBounds(SkIRect*) const override;
    bool isClipEmpty() const override;
    bool isClipRect() const override;
    SkSurface* onNewSurface(const SkImageInfo&, const SkSurfaceProps&) override;
    bool onPeekPixels(SkPixmap*) override;
    bool onAccessTopLayerPixels(SkPixmap*) override;
    void willSave() override;
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
    void willRestore() override;
    void didRestore() override;
    void didConcat(const SkMatrix&) override;
    void didSetMatrix(const SkMatrix&) override;
    void onClipRect(const SkRect&, SkRegion::Op, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect&, SkRegion::Op, ClipEdgeStyle) override;
    void onClipPath(const SkPath&, SkRegion::Op, ClipEdgeStyle) override;
    void onClipRegion(const SkRegion&, SkRegion::Op) override;
    void onDiscard() override;

protected:
    SkCanvas* fProxyTarget;
};

#endif  // SkAndroidSDKCanvas_DEFINED

