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
#include "src/gpu/BaseDevice.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/v1/ClipStack.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"

class SkSpecialImage;
class SkSurface;
class SkSurface_Gpu;
class SkVertices;

namespace skgpu::v1 {

/**
 *  Subclass of BaseDevice, which directs all drawing to the GrGpu owned by the canvas.
 */
class Device final : public BaseDevice  {
public:
    bool wait(int numSemaphores,
              const GrBackendSemaphore* waitSemaphores,
              bool deleteSemaphoresAfterWait) override;

    void discard() override {
        fSurfaceDrawContext->discard();
    }

    bool replaceBackingProxy(SkSurface::ContentChangeMode,
                             sk_sp<GrRenderTargetProxy>,
                             GrColorType,
                             sk_sp<SkColorSpace>,
                             GrSurfaceOrigin,
                             const SkSurfaceProps&) override;
    using BaseDevice::replaceBackingProxy;

    void asyncRescaleAndReadPixels(const SkImageInfo& info,
                                   const SkIRect& srcRect,
                                   RescaleGamma rescaleGamma,
                                   RescaleMode rescaleMode,
                                   ReadPixelsCallback callback,
                                   ReadPixelsContext context) override;

    void asyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                         sk_sp<SkColorSpace> dstColorSpace,
                                         const SkIRect& srcRect,
                                         SkISize dstSize,
                                         RescaleGamma rescaleGamma,
                                         RescaleMode,
                                         ReadPixelsCallback callback,
                                         ReadPixelsContext context) override;

    /**
     * This factory uses the color space, origin, surface properties, and initialization
     * method along with the provided proxy to create the gpu device.
     */
    static sk_sp<BaseDevice> Make(GrRecordingContext*,
                                  GrColorType,
                                  sk_sp<GrSurfaceProxy>,
                                  sk_sp<SkColorSpace>,
                                  GrSurfaceOrigin,
                                  const SkSurfaceProps&,
                                  InitContents);

    /**
     * This factory uses the budgeted, imageInfo, fit, sampleCount, mipmapped, and isProtected
     * parameters to create a proxy to back the gpu device. The color space (from the image info),
     * origin, surface properties, and initialization method are then used (with the created proxy)
     * to create the device.
     */
    static sk_sp<BaseDevice> Make(GrRecordingContext*,
                                  SkBudgeted,
                                  const SkImageInfo&,
                                  SkBackingFit,
                                  int sampleCount,
                                  GrMipmapped,
                                  GrProtected,
                                  GrSurfaceOrigin,
                                  const SkSurfaceProps&,
                                  InitContents);

    ~Device() override {}

    SurfaceDrawContext* surfaceDrawContext() override;
    const SurfaceDrawContext* surfaceDrawContext() const;
    skgpu::SurfaceFillContext* surfaceFillContext() override;

    // set all pixels to 0
    void clearAll();

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

    void drawVertices(const SkVertices*, sk_sp<SkBlender>, const SkPaint&) override;
    void drawShadow(const SkPath&, const SkDrawShadowRec&) override;
    void drawAtlas(const SkRSXform[], const SkRect[], const SkColor[], int count, sk_sp<SkBlender>,
                   const SkPaint&) override;

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

    bool onAccessPixels(SkPixmap*) override;

    bool android_utils_clipWithStencil() override;

protected:
    bool onReadPixels(const SkPixmap&, int, int) override;
    bool onWritePixels(const SkPixmap&, int, int) override;

    void onSave() override { fClip.save(); }
    void onRestore() override { fClip.restore(); }

    void onDrawGlyphRunList(const SkGlyphRunList& glyphRunList, const SkPaint& paint) override;

    sk_sp<GrSlug> convertGlyphRunListToSlug(
            const SkGlyphRunList& glyphRunList,
            const SkPaint& paint) const override;

    void drawSlug(GrSlug* slug) override;

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
        SkRect deviceRect = SkMatrixPriv::MapRect(this->globalToDevice(), SkRect::Make(rect));
        fClip.replaceClip(deviceRect.round());
    }
    void onClipRegion(const SkRegion& globalRgn, SkClipOp op) override;
    void onAsRgnClip(SkRegion*) const override;
    ClipType onGetClipType() const override;
    bool onClipIsAA() const override;

    bool onClipIsWideOpen() const override {
        return fClip.clipState() == ClipStack::ClipState::kWideOpen;
    }
    SkIRect onDevClipBounds() const override { return fClip.getConservativeBounds(); }

private:
    std::unique_ptr<SurfaceDrawContext> fSurfaceDrawContext;

    ClipStack fClip;

    static sk_sp<BaseDevice> Make(std::unique_ptr<SurfaceDrawContext>,
                                  SkAlphaType,
                                  InitContents);

    Device(std::unique_ptr<SurfaceDrawContext>, DeviceFlags);

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

    friend class ::SkSurface_Gpu;      // for access to surfaceProps
    using INHERITED = BaseDevice;
};

} // namespace skgpu::v1

#undef GR_CLIP_STACK

#endif
