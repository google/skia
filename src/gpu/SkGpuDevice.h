/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGpuDevice_DEFINED
#define SkGpuDevice_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRegion.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrTypes.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/SkBaseGpuDevice.h"
#include "src/gpu/SkGr.h"

class SkSpecialImage;
class SkSurface;
class SkVertices;

#if !defined(SK_DISABLE_NEW_GR_CLIP_STACK)
    #include "src/gpu/GrClipStack.h"
    #define GR_CLIP_STACK GrClipStack
#else
    #include "src/gpu/GrClipStackClip.h"
    #define GR_CLIP_STACK GrClipStackClip
#endif

/**
 *  Subclass of SkBaseGpuDevice, which directs all drawing to the GrGpu owned by the canvas.
 */
class SkGpuDevice : public SkBaseGpuDevice  {
public:
    GrSurfaceProxyView readSurfaceView() override {
        return this->surfaceDrawContext()->readSurfaceView();
    }

    GrImageInfo grImageInfo() const override {
        return this->surfaceDrawContext()->imageInfo();
    }

    enum InitContents {
        kClear_InitContents,
        kUninit_InitContents
    };

    /**
     * Creates an SkGpuDevice from a GrSurfaceDrawContext whose backing width/height is
     * different than its actual width/height (e.g., approx-match scratch texture).
     */
    static sk_sp<SkGpuDevice> Make(std::unique_ptr<GrSurfaceDrawContext>, InitContents);

    /**
     * New device that will create an offscreen renderTarget based on the ImageInfo and
     * sampleCount. The mipMapped flag tells the gpu to create the underlying render target with
     * mips. The Budgeted param controls whether the device's backing store counts against the
     * resource cache budget. On failure, returns nullptr.
     * This entry point creates a kExact backing store. It is used when creating SkGpuDevices
     * for SkSurfaces.
     */
    static sk_sp<SkGpuDevice> Make(GrRecordingContext*, SkBudgeted, const SkImageInfo&,
                                   int sampleCount, GrSurfaceOrigin, const SkSurfaceProps*,
                                   GrMipmapped mipMapped, InitContents);

    ~SkGpuDevice() override {}

    GrRecordingContext* recordingContext() const override { return fContext.get(); }
    GrSurfaceDrawContext* surfaceDrawContext() override;
    const GrSurfaceDrawContext* surfaceDrawContext() const;

    // set all pixels to 0
    void clearAll();

    void replaceSurfaceDrawContext(SkSurface::ContentChangeMode mode);
    void replaceSurfaceDrawContext(std::unique_ptr<GrSurfaceDrawContext>,
                                   SkSurface::ContentChangeMode mode);

    void drawPaint(const SkPaint& paint) override;
    void drawPoints(SkCanvas::PointMode mode, size_t count, const SkPoint[],
                    const SkPaint& paint) override;
    void drawRect(const SkRect& r, const SkPaint& paint) override;
    void drawRRect(const SkRRect& r, const SkPaint& paint) override;
    void drawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) override;
    void drawRegion(const SkRegion& r, const SkPaint& paint) override;
    void drawOval(const SkRect& oval, const SkPaint& paint) override;
    void drawArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                 bool useCenter, const SkPaint& paint) override;
    void drawPath(const SkPath& path, const SkPaint& paint, bool pathIsMutable) override;

    void onDrawGlyphRunList(const SkGlyphRunList& glyphRunList, const SkPaint& paint) override;
    void drawVertices(const SkVertices*, SkBlendMode, const SkPaint&) override;
    void drawShadow(const SkPath&, const SkDrawShadowRec&) override;
    void drawAtlas(const SkImage* atlas, const SkRSXform[], const SkRect[], const SkColor[],
                   int count, SkBlendMode, const SkSamplingOptions&, const SkPaint&) override;

    void drawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                       const SkSamplingOptions&, const SkPaint&,
                       SkCanvas::SrcRectConstraint) override;
    void drawImageLattice(const SkImage*, const SkCanvas::Lattice&,
                          const SkRect& dst, SkFilterMode, const SkPaint&) override;

    void drawDrawable(SkDrawable*, const SkMatrix*, SkCanvas* canvas) override;

    void drawDevice(SkBaseDevice*, const SkSamplingOptions&, const SkPaint&) override;
    void drawSpecial(SkSpecialImage*, const SkMatrix& localToDevice, const SkSamplingOptions&,
                     const SkPaint&) override;

    void drawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4], SkCanvas::QuadAAFlags aaFlags,
                        const SkColor4f& color, SkBlendMode mode) override;
    void drawEdgeAAImageSet(const SkCanvas::ImageSetEntry[], int count, const SkPoint dstClips[],
                            const SkMatrix[], const SkSamplingOptions&, const SkPaint&,
                            SkCanvas::SrcRectConstraint) override;

    sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkImage*) override;
    sk_sp<SkSpecialImage> snapSpecial(const SkIRect& subset, bool forceCopy = false) override;

    bool wait(int numSemaphores, const GrBackendSemaphore* waitSemaphores,
              bool deleteSemaphoresAfterWait);

    bool onAccessPixels(SkPixmap*) override;

    bool android_utils_clipWithStencil() override;

protected:
    bool onReadPixels(const SkPixmap&, int, int) override;
    bool onWritePixels(const SkPixmap&, int, int) override;

#if !defined(SK_DISABLE_NEW_GR_CLIP_STACK)
    void onSave() override { fClip.save(); }
    void onRestore() override { fClip.restore(); }

    void onClipRect(const SkRect& rect, SkClipOp op, bool aa) override {
        SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
        fClip.clipRect(this->localToDevice(), rect, GrAA(aa), op);
    }
    void onClipRRect(const SkRRect& rrect, SkClipOp op, bool aa) override {
        SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
        fClip.clipRRect(this->localToDevice(), rrect, GrAA(aa), op);
    }
    void onClipPath(const SkPath& path, SkClipOp op, bool aa) override;
    void onClipShader(sk_sp<SkShader> shader) override {
        fClip.clipShader(std::move(shader));
    }
    void onReplaceClip(const SkIRect& rect) override {
        // Transform from "global/canvas" coordinates to relative to this device
        SkIRect deviceRect = this->globalToDevice().mapRect(SkRect::Make(rect)).round();
        fClip.replaceClip(deviceRect);
    }
    void onClipRegion(const SkRegion& globalRgn, SkClipOp op) override;
    void onAsRgnClip(SkRegion*) const override;
    ClipType onGetClipType() const override;
    bool onClipIsAA() const override;

    void onSetDeviceClipRestriction(SkIRect* mutableClipRestriction) override {
        SkASSERT(mutableClipRestriction->isEmpty());
    }
    bool onClipIsWideOpen() const override {
        return fClip.clipState() == GrClipStack::ClipState::kWideOpen;
    }
    SkIRect onDevClipBounds() const override { return fClip.getConservativeBounds(); }
#endif

private:
    // We want these unreffed in SurfaceDrawContext, GrContext order.
    sk_sp<GrRecordingContext> fContext;
    std::unique_ptr<GrSurfaceDrawContext> fSurfaceDrawContext;

    GR_CLIP_STACK fClip;

    enum Flags {
        kNeedClear_Flag = 1 << 0,  //!< Surface requires an initial clear
        kIsOpaque_Flag  = 1 << 1,  //!< Hint from client that rendering to this device will be
                                   //   opaque even if the config supports alpha.
    };
    static bool CheckAlphaTypeAndGetFlags(const SkImageInfo* info, InitContents init,
                                          unsigned* flags);

    SkGpuDevice(std::unique_ptr<GrSurfaceDrawContext>, unsigned flags);

    SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) override;

    sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&) override;

    SkImageFilterCache* getImageFilterCache() override;

    bool forceConservativeRasterClip() const override { return true; }

    const GrClip* clip() const { return &fClip; }

    // If not null, dstClip must be contained inside dst and will also respect the edge AA flags.
    // If 'preViewMatrix' is not null, final CTM will be this->ctm() * preViewMatrix.
    void drawImageQuad(const SkImage*, const SkRect* src, const SkRect* dst,
                       const SkPoint dstClip[4], GrAA aa, GrQuadAAFlags aaFlags,
                       const SkMatrix* preViewMatrix, const SkSamplingOptions&,
                       const SkPaint&, SkCanvas::SrcRectConstraint);

    // FIXME(michaelludwig) - Should be removed in favor of using drawImageQuad with edge flags to
    // for every element in the SkLatticeIter.
    void drawViewLattice(GrSurfaceProxyView,
                         const GrColorInfo& colorInfo,
                         std::unique_ptr<SkLatticeIter>,
                         const SkRect& dst,
                         SkFilterMode,
                         const SkPaint&);

    static std::unique_ptr<GrSurfaceDrawContext> MakeSurfaceDrawContext(GrRecordingContext*,
                                                                        SkBudgeted,
                                                                        const SkImageInfo&,
                                                                        int sampleCount,
                                                                        GrSurfaceOrigin,
                                                                        const SkSurfaceProps*,
                                                                        GrMipmapped);

    friend class SkSurface_Gpu;      // for access to surfaceProps
    using INHERITED = SkBaseGpuDevice;
};

#undef GR_CLIP_STACK

#endif
