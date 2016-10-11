/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShadowPaintFilterCanvas_DEFINED
#define SkShadowPaintFilterCanvas_DEFINED

#include "SkPaintFilterCanvas.h"

#ifdef SK_EXPERIMENTAL_SHADOWING

/** \class SkShadowPaintFilterCanvas
 *
 *  A utility proxy class for implementing shadow maps.
 *
 *  We override the onFilter method to draw depths into the canvas
 *  depending on the current draw depth of the canvas, throwing out
 *  the actual draw color.
 *
 *  Note that we can only do this for one light at a time!
 *  It is up to the user to set the 0th light in fLights to
 *  the light the want to render the depth map with.
 */
class SkShadowPaintFilterCanvas : public SkPaintFilterCanvas {
public:

    SkShadowPaintFilterCanvas(SkCanvas *canvas);

    // TODO use a shader instead
    bool onFilter(SkTCopyOnFirstWrite<SkPaint>* paint, Type type) const override;

    static SkISize ComputeDepthMapSize(const SkLights::Light& light, int maxDepth,
                                       int width, int height);

    void setShadowParams(const SkShadowParams &params);
protected:
    void updateMatrix();

    void onDrawPicture(const SkPicture *picture, const SkMatrix *matrix,
                       const SkPaint *paint) override;

    void onDrawPaint(const SkPaint &paint) override;

    void onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                      const SkPaint &paint) override;

    void onDrawRect(const SkRect &rect, const SkPaint &paint) override;

    void onDrawRRect(const SkRRect &rrect, const SkPaint &paint) override;

    void onDrawDRRect(const SkRRect &outer, const SkRRect &inner,
                      const SkPaint &paint) override;

    void onDrawOval(const SkRect &rect, const SkPaint &paint) override;

    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override;

    void onDrawPath(const SkPath &path, const SkPaint &paint) override;

    void onDrawBitmap(const SkBitmap &bm, SkScalar left, SkScalar top,
                      const SkPaint *paint) override;

    void onDrawBitmapRect(const SkBitmap &bm, const SkRect *src, const SkRect &dst,
                          const SkPaint *paint, SrcRectConstraint constraint) override;

    void onDrawBitmapNine(const SkBitmap &bm, const SkIRect &center,
                          const SkRect &dst, const SkPaint *paint) override;

    void onDrawImage(const SkImage *image, SkScalar left, SkScalar top,
                     const SkPaint *paint) override;

    void onDrawImageRect(const SkImage *image, const SkRect *src,
                         const SkRect &dst, const SkPaint *paint,
                         SrcRectConstraint constraint) override;

    void onDrawImageNine(const SkImage *image, const SkIRect &center,
                         const SkRect &dst, const SkPaint *paint) override;

    void onDrawVertices(VertexMode vmode, int vertexCount,
                        const SkPoint vertices[], const SkPoint texs[],
                        const SkColor colors[], SkXfermode *xmode,
                        const uint16_t indices[], int indexCount,
                        const SkPaint &paint) override;

    void onDrawPatch(const SkPoint cubics[], const SkColor colors[],
                     const SkPoint texCoords[], SkXfermode *xmode,
                     const SkPaint &paint) override;

    void onDrawText(const void *text, size_t byteLength, SkScalar x, SkScalar y,
                    const SkPaint &paint) override;

    void onDrawPosText(const void *text, size_t byteLength, const SkPoint pos[],
                       const SkPaint &paint) override;

    void onDrawPosTextH(const void *text, size_t byteLength, const SkScalar xpos[],
                        SkScalar constY, const SkPaint &paint) override;

    void onDrawTextOnPath(const void *text, size_t byteLength, const SkPath &path,
                          const SkMatrix *matrix, const SkPaint &paint) override;

    void onDrawTextRSXform(const void *text, size_t byteLength,
                           const SkRSXform xform[], const SkRect *cull,
                           const SkPaint &paint) override;

    void onDrawTextBlob(const SkTextBlob *blob, SkScalar x,
                        SkScalar y, const SkPaint &paint) override;
private:
    SkShadowParams fShadowParams;
    typedef SkPaintFilterCanvas INHERITED;
};


#endif
#endif
