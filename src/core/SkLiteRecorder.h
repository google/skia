/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLiteRecorder_DEFINED
#define SkLiteRecorder_DEFINED

#include "SkNoDrawCanvas.h"

class SkLiteDL;

class SkLiteRecorder final : public SkNoDrawCanvas {
public:
    SkLiteRecorder();
    void reset(SkLiteDL*, const SkIRect& bounds);

    sk_sp<SkSurface> onNewSurface(const SkImageInfo&, const SkSurfaceProps&) override;

#ifdef SK_SUPPORT_LEGACY_DRAWFILTER
    SkDrawFilter* setDrawFilter(SkDrawFilter*) override;
#endif

    void willSave() override;
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
    void willRestore() override;

    void didConcat(const SkMatrix&) override;
    void didSetMatrix(const SkMatrix&) override;
    void didTranslate(SkScalar, SkScalar) override;

    void onClipRect  (const   SkRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect (const  SkRRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipPath  (const   SkPath&, SkClipOp, ClipEdgeStyle) override;
    void onClipRegion(const SkRegion&, SkClipOp) override;

    void onDrawPaint (const SkPaint&) override;
    void onDrawPath  (const SkPath&, const SkPaint&) override;
    void onDrawRect  (const SkRect&, const SkPaint&) override;
    void onDrawRegion(const SkRegion&, const SkPaint&) override;
    void onDrawOval  (const SkRect&, const SkPaint&) override;
    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override;
    void onDrawRRect (const SkRRect&, const SkPaint&) override;
    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;

    void onDrawDrawable(SkDrawable*, const SkMatrix*) override;
    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;
    void onDrawAnnotation(const SkRect&, const char[], SkData*) override;

    void onDrawText      (const void*, size_t, SkScalar x, SkScalar y, const SkPaint&) override;
    void onDrawPosText   (const void*, size_t, const SkPoint[], const SkPaint&) override;
    void onDrawPosTextH  (const void*, size_t, const SkScalar[], SkScalar, const SkPaint&) override;
    void onDrawTextOnPath(const void*, size_t,
                          const SkPath&, const SkMatrix*, const SkPaint&) override;
    void onDrawTextRSXform(const void*, size_t,
                           const SkRSXform[], const SkRect*, const SkPaint&) override;
    void onDrawTextBlob(const SkTextBlob*, SkScalar, SkScalar, const SkPaint&) override;

    void onDrawBitmap(const SkBitmap&, SkScalar, SkScalar, const SkPaint*) override;
    void onDrawBitmapLattice(const SkBitmap&, const Lattice&, const SkRect&,
                             const SkPaint*) override;
    void onDrawBitmapNine(const SkBitmap&, const SkIRect&, const SkRect&, const SkPaint*) override;
    void onDrawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&, const SkPaint*,
                          SrcRectConstraint) override;

    void onDrawImage(const SkImage*, SkScalar, SkScalar, const SkPaint*) override;
    void onDrawImageLattice(const SkImage*, const Lattice&, const SkRect&, const SkPaint*) override;
    void onDrawImageNine(const SkImage*, const SkIRect&, const SkRect&, const SkPaint*) override;
    void onDrawImageRect(const SkImage*, const SkRect*, const SkRect&, const SkPaint*,
                         SrcRectConstraint) override;

    void onDrawPatch(const SkPoint[12], const SkColor[4],
                     const SkPoint[4], SkBlendMode, const SkPaint&) override;
    void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&) override;
    void onDrawVerticesObject(const SkVertices*, SkBlendMode, const SkPaint&) override;
    void onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[],
                     int, SkBlendMode, const SkRect*, const SkPaint*) override;

#ifdef SK_EXPERIMENTAL_SHADOWING
    void didTranslateZ(SkScalar) override;
    void onDrawShadowedPicture(const SkPicture*, const SkMatrix*,
                               const SkPaint*, const SkShadowParams& params) override;
#else
    void didTranslateZ(SkScalar);
    void onDrawShadowedPicture(const SkPicture*, const SkMatrix*,
                               const SkPaint*, const SkShadowParams& params);
#endif

private:
    typedef SkNoDrawCanvas INHERITED;

    SkLiteDL* fDL;
};

#endif//SkLiteRecorder_DEFINED
