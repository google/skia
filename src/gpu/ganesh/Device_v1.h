/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_v1_Device_DEFINED
#define skgpu_v1_Device_DEFINED

#include "include/core/SkSurface.h"
#include "include/gpu/GrTypes.h"
#include "src/core/SkDevice.h"
#include "src/gpu/ganesh/ClipStack.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/text/gpu/SDFTControl.h"

class SkBitmap;
class SkLatticeIter;
class SkRegion;
class SkSpecialImage;
class SkSurface;
class SkSurface_Gpu;
class SkVertices;

namespace skgpu::v1 {

class SurfaceContext;
class SurfaceFillContext;

/**
 *  Subclass of SkBaseDevice, which directs all drawing to the GrGpu owned by the canvas.
 */
class Device final : public SkBaseDevice  {
public:
    enum class InitContents {
        kClear,
        kUninit
    };

    GrSurfaceProxyView readSurfaceView();
    GrRenderTargetProxy* targetProxy();

    GrRecordingContext* recordingContext() const { return fContext.get(); }

    bool wait(int numSemaphores,
              const GrBackendSemaphore* waitSemaphores,
              bool deleteSemaphoresAfterWait);

    void discard();
    void resolveMSAA();

    bool replaceBackingProxy(SkSurface::ContentChangeMode,
                             sk_sp<GrRenderTargetProxy>,
                             GrColorType,
                             sk_sp<SkColorSpace>,
                             GrSurfaceOrigin,
                             const SkSurfaceProps&);
    bool replaceBackingProxy(SkSurface::ContentChangeMode);

    using RescaleGamma       = SkImage::RescaleGamma;
    using RescaleMode        = SkImage::RescaleMode;
    using ReadPixelsCallback = SkImage::ReadPixelsCallback;
    using ReadPixelsContext  = SkImage::ReadPixelsContext;

    void asyncRescaleAndReadPixels(const SkImageInfo& info,
                                   const SkIRect& srcRect,
                                   RescaleGamma rescaleGamma,
                                   RescaleMode rescaleMode,
                                   ReadPixelsCallback callback,
                                   ReadPixelsContext context);

    void asyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                         sk_sp<SkColorSpace> dstColorSpace,
                                         const SkIRect& srcRect,
                                         SkISize dstSize,
                                         RescaleGamma rescaleGamma,
                                         RescaleMode,
                                         ReadPixelsCallback callback,
                                         ReadPixelsContext context);

    /**
     * This factory uses the color space, origin, surface properties, and initialization
     * method along with the provided proxy to create the gpu device.
     */
    static sk_sp<Device> Make(GrRecordingContext*,
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
    static sk_sp<Device> Make(GrRecordingContext*,
                              skgpu::Budgeted,
                              const SkImageInfo&,
                              SkBackingFit,
                              int sampleCount,
                              GrMipmapped,
                              GrProtected,
                              GrSurfaceOrigin,
                              const SkSurfaceProps&,
                              InitContents);

    ~Device() override;

    SurfaceDrawContext* surfaceDrawContext();
    const SurfaceDrawContext* surfaceDrawContext() const;
    SurfaceFillContext* surfaceFillContext();

    SkStrikeDeviceInfo strikeDeviceInfo() const override;

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

    void drawVertices(const SkVertices*, sk_sp<SkBlender>, const SkPaint&, bool) override;
    void drawMesh(const SkMesh&, sk_sp<SkBlender>, const SkPaint&) override;
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
    void drawShadow(const SkPath&, const SkDrawShadowRec&) override;
#endif
    void drawAtlas(const SkRSXform[], const SkRect[], const SkColor[], int count, sk_sp<SkBlender>,
                   const SkPaint&) override;

    void drawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                       const SkSamplingOptions&, const SkPaint&,
                       SkCanvas::SrcRectConstraint) override;
    void drawImageLattice(const SkImage*, const SkCanvas::Lattice&,
                          const SkRect& dst, SkFilterMode, const SkPaint&) override;

    void drawDrawable(SkCanvas*, SkDrawable*, const SkMatrix*) override;

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
    sk_sp<SkSpecialImage> snapSpecialScaled(const SkIRect& subset, const SkISize& dstDims) override;

    bool onAccessPixels(SkPixmap*) override;

    bool android_utils_clipWithStencil() override;

    Device* asGaneshDevice() override { return this; }

protected:
    bool onReadPixels(const SkPixmap&, int, int) override;
    bool onWritePixels(const SkPixmap&, int, int) override;

    void onSave() override { fClip.save(); }
    void onRestore() override { fClip.restore(); }

    void onDrawGlyphRunList(SkCanvas*,
                            const sktext::GlyphRunList&,
                            const SkPaint& initialPaint,
                            const SkPaint& drawingPaint) override;

    sk_sp<sktext::gpu::Slug> convertGlyphRunListToSlug(
            const sktext::GlyphRunList& glyphRunList,
            const SkPaint& initialPaint,
            const SkPaint& drawingPaint) override;

    void drawSlug(SkCanvas*, const sktext::gpu::Slug* slug, const SkPaint& drawingPaint) override;

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
    enum class DeviceFlags {
        kNone      = 0,
        kNeedClear = 1 << 0,  //!< Surface requires an initial clear
        kIsOpaque  = 1 << 1,  //!< Hint from client that rendering to this device will be
        //   opaque even if the config supports alpha.
    };
    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(DeviceFlags);

    static SkImageInfo MakeInfo(SurfaceContext*,  DeviceFlags);
    static bool CheckAlphaTypeAndGetFlags(SkAlphaType, InitContents, DeviceFlags*);

    sk_sp<GrRecordingContext> fContext;

    const sktext::gpu::SDFTControl fSDFTControl;

    std::unique_ptr<SurfaceDrawContext> fSurfaceDrawContext;

    ClipStack fClip;

    static sk_sp<Device> Make(std::unique_ptr<SurfaceDrawContext>,
                              SkAlphaType,
                              InitContents);

    Device(std::unique_ptr<SurfaceDrawContext>, DeviceFlags);

    SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) override;

    sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&) override;

    SkImageFilterCache* getImageFilterCache() override;

    bool forceConservativeRasterClip() const override { return true; }

    const GrClip* clip() const { return &fClip; }
#if defined(SK_EXPERIMENTAL_SIMULATE_DRAWGLYPHRUNLIST_WITH_SLUG)
    void testingOnly_drawGlyphRunListWithSlug(SkCanvas* canvas,
                                              const sktext::GlyphRunList& glyphRunList,
                                              const SkPaint& initialPaint,
                                              const SkPaint& drawingPaint);
#endif

#if defined(SK_EXPERIMENTAL_SIMULATE_DRAWGLYPHRUNLIST_WITH_SLUG_SERIALIZE)
    void testingOnly_drawGlyphRunListWithSerializedSlug(SkCanvas* canvas,
                                                        const sktext::GlyphRunList& glyphRunList,
                                                        const SkPaint& initialPaint,
                                                        const SkPaint& drawingPaint);
#endif

#if defined(SK_EXPERIMENTAL_SIMULATE_DRAWGLYPHRUNLIST_WITH_SLUG_STRIKE_SERIALIZE)
    void testingOnly_drawGlyphRunListWithSerializedSlugAndStrike(
            SkCanvas* canvas,
            const sktext::GlyphRunList& glyphRunList,
            const SkPaint& initialPaint,
            const SkPaint& drawingPaint);
#endif

    // If not null, dstClip must be contained inside dst and will also respect the edge AA flags.
    // If 'preViewMatrix' is not null, final CTM will be this->ctm() * preViewMatrix.
    void drawImageQuad(const SkImage*, const SkRect* src, const SkRect* dst,
                       const SkPoint dstClip[4], GrQuadAAFlags aaFlags,
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
};

GR_MAKE_BITFIELD_CLASS_OPS(Device::DeviceFlags)

} // namespace skgpu::v1

#endif // skgpu_v1_Device_DEFINED
