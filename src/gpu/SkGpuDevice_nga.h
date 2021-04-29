/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGpuDevice_nga_DEFINED
#define SkGpuDevice_nga_DEFINED

#include "include/gpu/GrTypes.h"

#if 1 //def SK_NGA
#include "include/core/SkBitmap.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRegion.h"
#include "include/core/SkSurface.h"
#include "src/core/SkDevice.h"
#include "src/gpu/GrClipStack.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/SkGr.h"

class GrTextureMaker;
class GrTextureProducer;
struct GrCachedLayer;

class SkSpecialImage;
class SkSurface;
class SkVertices;

/**
 *  Subclass of SkFooDevice, which directs all drawing to the GrGpu owned by the
 *  canvas.
 */
class SkGpuDevice_nga : public SkBaseDevice  {
public:
    ~SkGpuDevice_nga() override {}

    GrRecordingContext* recordingContext() const override { return nullptr; }
    GrSurfaceDrawContext* surfaceDrawContext() override { return nullptr; }

protected:

    void onSave() override {}
    void onRestore() override {}
    void onClipRect(const SkRect& rect, SkClipOp op, bool aa) override {}
    void onClipRRect(const SkRRect& rrect, SkClipOp op, bool aa) override {}
    void onClipPath(const SkPath& path, SkClipOp op, bool aa) override {}
    void onClipShader(sk_sp<SkShader> shader) override {}
    void onClipRegion(const SkRegion& globalRgn, SkClipOp op) override {}
    void onReplaceClip(const SkIRect& rect) override {}
    void onSetDeviceClipRestriction(SkIRect* mutableClipRestriction) override {}
    bool onClipIsAA() const override { return false; }
    bool onClipIsWideOpen() const override { return false; }
    void onAsRgnClip(SkRegion*) const override {}
    ClipType onGetClipType() const override { return ClipType::kEmpty; }
    SkIRect onDevClipBounds() const override { return SkIRect::MakeEmpty(); }

    void drawPaint(const SkPaint& paint) override {}
    void drawPoints(SkCanvas::PointMode mode, size_t count, const SkPoint[],
                    const SkPaint& paint) override {}
    void drawRect(const SkRect& r, const SkPaint& paint) override {}
    void drawRegion(const SkRegion& r, const SkPaint& paint) override {}
    void drawOval(const SkRect& oval, const SkPaint& paint) override {}
    void drawArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                 bool useCenter, const SkPaint& paint) override {}
    void drawRRect(const SkRRect& r, const SkPaint& paint) override {}
    void drawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) override {}
    void drawPath(const SkPath& path, const SkPaint& paint, bool pathIsMutable) override {}

    void drawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                       const SkSamplingOptions&, const SkPaint&,
                       SkCanvas::SrcRectConstraint) override {}
    void drawImageLattice(const SkImage*, const SkCanvas::Lattice&,
                          const SkRect& dst, SkFilterMode, const SkPaint&) override {}

    void drawVertices(const SkVertices*, SkBlendMode, const SkPaint&) override {}
    void drawShadow(const SkPath&, const SkDrawShadowRec&) override {}
    /* drawPatch */
    void drawAtlas(const SkImage* atlas, const SkRSXform[], const SkRect[], const SkColor[],
                   int count, SkBlendMode, const SkSamplingOptions&, const SkPaint&) override {}
    /* drawAnnotation */
    void drawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4], SkCanvas::QuadAAFlags aaFlags,
                        const SkColor4f& color, SkBlendMode mode) override {}
    void drawEdgeAAImageSet(const SkCanvas::ImageSetEntry[], int count, const SkPoint dstClips[],
                            const SkMatrix[], const SkSamplingOptions&, const SkPaint&,
                            SkCanvas::SrcRectConstraint) override {}
    void drawDrawable(SkDrawable*, const SkMatrix*, SkCanvas* canvas) override {}
    void onDrawGlyphRunList(const SkGlyphRunList& glyphRunList, const SkPaint& paint) override {}
    void drawDevice(SkBaseDevice*, const SkSamplingOptions&, const SkPaint&) override {}
    void drawSpecial(SkSpecialImage*, const SkMatrix&, const SkSamplingOptions&,
                     const SkPaint&) override {}
    /* drawFilteredImage */
    sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&) override { return nullptr; }
    sk_sp<SkSpecialImage> makeSpecial(const SkImage*) override { return nullptr; }
    /* snapSpecial() */
    sk_sp<SkSpecialImage> snapSpecial(const SkIRect&, bool = false) override { return nullptr; }
    /* setImmutable */
    sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&) override {
        return nullptr;
    }
    /* onPeekPixels */
    bool onReadPixels(const SkPixmap&, int, int) override { return false; }
    bool onWritePixels(const SkPixmap&, int, int) override { return false; }
    bool onAccessPixels(SkPixmap*) override { return false; }
    SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) override { return nullptr; }
    /* isNoPixelsDevice */

private:
    SkGpuDevice_nga(GrRecordingContext*);

    /* replaceBitmapBackendForRasterSurface */
    bool forceConservativeRasterClip() const override { return true; }
    SkImageFilterCache* getImageFilterCache() override { return nullptr; }

    using INHERITED = SkBaseDevice;
};

#endif // SK_NGA

#endif // SkGpuDevice_nga_DEFINED
