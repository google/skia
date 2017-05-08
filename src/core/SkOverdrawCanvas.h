/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOverdrawCanvas_DEFINED
#define SkOverdrawCanvas_DEFINED

#include "SkNWayCanvas.h"

/**
 *  Captures all drawing commands.  Rather than draw the actual content, this device
 *  increments the alpha channel of each pixel every time it would have been touched
 *  by a draw call.  This is useful for detecting overdraw.
 */
class SkOverdrawCanvas : public SkNWayCanvas {
public:
    /* Does not take ownership of canvas */
    SkOverdrawCanvas(SkCanvas*);

    void onDrawText(const void*, size_t, SkScalar, SkScalar, const SkPaint&) override;
    void onDrawPosText(const void*, size_t, const SkPoint[], const SkPaint&) override;
    void onDrawPosTextH(const void*, size_t, const SkScalar[], SkScalar, const SkPaint&) override;
    void onDrawTextOnPath(const void*, size_t, const SkPath&, const SkMatrix*,
                          const SkPaint&) override;
    void onDrawTextRSXform(const void*, size_t, const SkRSXform[], const SkRect*,
                           const SkPaint&) override;
    void onDrawTextBlob(const SkTextBlob*, SkScalar, SkScalar, const SkPaint&) override;
    void onDrawPatch(const SkPoint[12], const SkColor[4], const SkPoint[4], SkBlendMode,
                     const SkPaint&) override;
    void onDrawPaint(const SkPaint&) override;
    void onDrawRect(const SkRect&, const SkPaint&) override;
    void onDrawRegion(const SkRegion&, const SkPaint&) override;
    void onDrawOval(const SkRect&, const SkPaint&) override;
    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override;
    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;
    void onDrawRRect(const SkRRect&, const SkPaint&) override;
    void onDrawPoints(PointMode, size_t, const SkPoint[], const SkPaint&) override;
    void onDrawVerticesObject(const SkVertices*, SkBlendMode, const SkPaint&) override;
    void onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[],
                     int, SkBlendMode, const SkRect*, const SkPaint*) override;
    void onDrawPath(const SkPath&, const SkPaint&) override;
    void onDrawImage(const SkImage*, SkScalar, SkScalar, const SkPaint*) override;
    void onDrawImageRect(const SkImage*, const SkRect*, const SkRect&, const SkPaint*,
                         SrcRectConstraint) override;
    void onDrawImageNine(const SkImage*, const SkIRect&, const SkRect&, const SkPaint*) override;
    void onDrawImageLattice(const SkImage*, const Lattice&, const SkRect&, const SkPaint*) override;
    void onDrawBitmap(const SkBitmap&, SkScalar, SkScalar, const SkPaint*) override;
    void onDrawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&, const SkPaint*,
                          SrcRectConstraint) override;
    void onDrawBitmapNine(const SkBitmap&, const SkIRect&, const SkRect&, const SkPaint*) override;
    void onDrawBitmapLattice(const SkBitmap&, const Lattice&, const SkRect&,
                             const SkPaint*) override;
    void onDrawDrawable(SkDrawable*, const SkMatrix*) override;
    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;

private:
    void drawPosTextCommon(const void*, size_t, const SkScalar[], int, const SkPoint&,
                           const SkPaint&);

    inline SkPaint overdrawPaint(const SkPaint& paint);

    SkPaint   fPaint;

    typedef SkNWayCanvas INHERITED;
};

#endif
