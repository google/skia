/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNoDrawCanvas_DEFINED
#define SkNoDrawCanvas_DEFINED

#include "SkCanvas.h"
#include "SkCanvasVirtualEnforcer.h"
#include "SkVertices.h"

struct SkIRect;

// SkNoDrawCanvas is a helper for SkCanvas subclasses which do not need to
// actually rasterize (e.g., analysis of the draw calls).
//
// It provides the following simplifications:
//
//   * not backed by any device/pixels
//   * conservative clipping (clipping calls only use rectangles)
//
class SK_API SkNoDrawCanvas : public SkCanvasVirtualEnforcer<SkCanvas> {
public:
    SkNoDrawCanvas(int width, int height);

    // TODO: investigate the users of this ctor.
    SkNoDrawCanvas(const SkIRect&);

    explicit SkNoDrawCanvas(sk_sp<SkBaseDevice> device);

    // Optimization to reset state to be the same as after construction.
    void resetCanvas(int width, int height) {
        resetForNextPicture(SkIRect::MakeWH(width, height));
    }

protected:
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override;
    bool onDoSaveBehind(const SkRect*) override;

    // No-op overrides for aborting rasterization earlier than SkNullBlitter.
    void onDrawAnnotation(const SkRect&, const char[], SkData*) override {}
    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override {}
    void onDrawDrawable(SkDrawable*, const SkMatrix*) override {}
    void onDrawTextBlob(const SkTextBlob*, SkScalar, SkScalar, const SkPaint&) override {}
    void onDrawPatch(const SkPoint[12], const SkColor[4], const SkPoint[4], SkBlendMode,
                     const SkPaint&) override {}

    void onDrawPaint(const SkPaint&) override {}
    void onDrawPoints(PointMode, size_t, const SkPoint[], const SkPaint&) override {}
    void onDrawRect(const SkRect&, const SkPaint&) override {}
    void onDrawEdgeAARect(const SkRect&, SkCanvas::QuadAAFlags, SkColor, SkBlendMode) override {}
    void onDrawRegion(const SkRegion&, const SkPaint&) override {}
    void onDrawOval(const SkRect&, const SkPaint&) override {}
    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override {}
    void onDrawRRect(const SkRRect&, const SkPaint&) override {}
    void onDrawPath(const SkPath&, const SkPaint&) override {}
    void onDrawBitmap(const SkBitmap&, SkScalar, SkScalar, const SkPaint*) override {}
    void onDrawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&, const SkPaint*,
                          SrcRectConstraint) override {}
    void onDrawImage(const SkImage*, SkScalar, SkScalar, const SkPaint*) override {}
    void onDrawImageRect(const SkImage*, const SkRect*, const SkRect&, const SkPaint*,
                         SrcRectConstraint) override {}
    void onDrawImageNine(const SkImage*, const SkIRect&, const SkRect&, const SkPaint*) override {}
    void onDrawBitmapNine(const SkBitmap&, const SkIRect&, const SkRect&,
                          const SkPaint*) override {}
    void onDrawImageLattice(const SkImage*, const Lattice&, const SkRect&,
                            const SkPaint*) override {}
    void onDrawImageSet(const SkCanvas::ImageSetEntry[], int, SkFilterQuality,
                        SkBlendMode) override {}
    void onDrawBitmapLattice(const SkBitmap&, const Lattice&, const SkRect&,
                             const SkPaint*) override {}
    void onDrawVerticesObject(const SkVertices*, const SkVertices::Bone[], int, SkBlendMode,
                              const SkPaint&) override {}
    void onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[],
                     int, SkBlendMode, const SkRect*, const SkPaint*) override {}
    void onDrawShadowRec(const SkPath&, const SkDrawShadowRec&) override {}
    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override {}

private:
    typedef SkCanvasVirtualEnforcer<SkCanvas> INHERITED;
};

#endif // SkNoDrawCanvas_DEFINED
