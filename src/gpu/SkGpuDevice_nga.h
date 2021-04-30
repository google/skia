/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGpuDevice_nga_DEFINED
#define SkGpuDevice_nga_DEFINED

#include "include/gpu/GrTypes.h"

#ifdef SK_NGA

#include "src/core/SkDevice.h"

/**
 */
class SkGpuDevice_nga : public SkBaseDevice  {
public:
    ~SkGpuDevice_nga() override;

    GrRecordingContext* recordingContext() const override { return fContext.get(); }
    GrSurfaceDrawContext* surfaceDrawContext() override { return nullptr; }

protected:
    void onSave() override;
    void onRestore() override;
    void onClipRect(const SkRect&, SkClipOp, bool aa) override;
    void onClipRRect(const SkRRect&, SkClipOp, bool aa) override;
    void onClipPath(const SkPath&, SkClipOp, bool aa) override;
    void onClipShader(sk_sp<SkShader>) override;
    void onClipRegion(const SkRegion&, SkClipOp) override;
    void onReplaceClip(const SkIRect& rect) override;
    void onSetDeviceClipRestriction(SkIRect* mutableClipRestriction) override;
    bool onClipIsAA() const override;
    bool onClipIsWideOpen() const override;
    void onAsRgnClip(SkRegion*) const override;
    ClipType onGetClipType() const override;
    SkIRect onDevClipBounds() const override;

    void drawPaint(const SkPaint&) override;
    void drawPoints(SkCanvas::PointMode, size_t count, const SkPoint[], const SkPaint&) override;
    void drawRect(const SkRect&, const SkPaint&) override;
    void drawRegion(const SkRegion&, const SkPaint&) override;
    void drawOval(const SkRect& oval, const SkPaint&) override;
    void drawArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                 bool useCenter, const SkPaint&) override;
    void drawRRect(const SkRRect&, const SkPaint&) override;
    void drawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint&) override;
    void drawPath(const SkPath&, const SkPaint&, bool pathIsMutable) override;
    void drawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                       const SkSamplingOptions&, const SkPaint&,
                       SkCanvas::SrcRectConstraint) override;
    void drawImageLattice(const SkImage*, const SkCanvas::Lattice&,
                          const SkRect& dst, SkFilterMode, const SkPaint&) override;

    void drawVertices(const SkVertices*, SkBlendMode, const SkPaint&) override;
    void drawShadow(const SkPath&, const SkDrawShadowRec&) override;
    /* drawPatch */
    void drawAtlas(const SkImage* atlas, const SkRSXform[], const SkRect[], const SkColor[],
                   int count, SkBlendMode, const SkSamplingOptions&, const SkPaint&) override;
    /* drawAnnotation */
    void drawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4], SkCanvas::QuadAAFlags aaFlags,
                        const SkColor4f& color, SkBlendMode) override;
    void drawEdgeAAImageSet(const SkCanvas::ImageSetEntry[], int count, const SkPoint dstClips[],
                            const SkMatrix[], const SkSamplingOptions&, const SkPaint&,
                            SkCanvas::SrcRectConstraint) override;
    void drawDrawable(SkDrawable*, const SkMatrix*, SkCanvas*) override;
    void onDrawGlyphRunList(const SkGlyphRunList&, const SkPaint&) override;
    void drawDevice(SkBaseDevice*, const SkSamplingOptions&, const SkPaint&) override;
    void drawSpecial(SkSpecialImage*, const SkMatrix& localToDevice, const SkSamplingOptions&,
                     const SkPaint&) override;
    /* drawFilteredImage */
    sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkImage*) override;
    /* snapSpecial() */
    sk_sp<SkSpecialImage> snapSpecial(const SkIRect& subset, bool forceCopy = false) override;
    /* setImmutable */
    sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&) override;
    /* onPeekPixels */
    bool onReadPixels(const SkPixmap&, int x, int y) override;
    bool onWritePixels(const SkPixmap&, int x, int y) override;
    bool onAccessPixels(SkPixmap*) override;
    SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) override;
    /* isNoPixelsDevice */

private:
    SkGpuDevice_nga(GrRecordingContext*, const SkImageInfo&, const SkSurfaceProps&);

    /* replaceBitmapBackendForRasterSurface */
    bool forceConservativeRasterClip() const override;
    SkImageFilterCache* getImageFilterCache() override;

    sk_sp<GrRecordingContext> fContext;

    using INHERITED = SkBaseDevice;
};

#endif // SK_NGA

#endif // SkGpuDevice_nga_DEFINED
