/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_v1_Device_DEFINED
#define skgpu_v1_Device_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkDevice.h"
#include "src/core/SkMatrixPriv.h"
#include "src/gpu/ganesh/ClipStack.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/text/gpu/SDFTControl.h"

#include <cstddef>
#include <memory>
#include <utility>

class GrBackendSemaphore;
class GrClip;
class GrRecordingContext;
class GrRenderTargetProxy;
class GrSurfaceProxy;
class SkBitmap;
class SkBlender;
class SkColorSpace;
class SkDrawable;
class SkLatticeIter;
class SkMatrix;
class SkMesh;
class SkPaint;
class SkPath;
class SkPixmap;
class SkRRect;
class SkRegion;
class SkSpecialImage;
class SkSurfaceProps;
class SkSurface_Ganesh;
class SkVertices;
enum SkAlphaType : int;
enum SkColorType : int;
enum class GrAA : bool;
enum class GrColorType;
enum class SkBackingFit;
enum class SkBlendMode;
enum class SkTileMode;
struct SkDrawShadowRec;
struct SkISize;
struct SkPoint;
struct SkRSXform;
namespace skgpu {
enum class Budgeted : bool;
enum class Mipmapped : bool;
class TiledTextureUtils;
}
namespace skif {
class Backend;
}
namespace sktext {
class GlyphRunList;
namespace gpu {
    class Slug;
}}


namespace skgpu::ganesh {

class SurfaceContext;
class SurfaceFillContext;
class SurfaceDrawContext;

/**
 *  Subclass of SkDevice, which directs all drawing to the GrGpu owned by the canvas.
 */
class Device final : public SkDevice {
public:
    enum class InitContents {
        kClear,
        kUninit
    };

    GrSurfaceProxyView readSurfaceView();
    GrRenderTargetProxy* targetProxy();

    GrRecordingContext* recordingContext() const override { return fContext.get(); }

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
                                         bool readAlpha,
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
                              skgpu::Mipmapped,
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
    bool shouldDrawAsTiledImageRect() const override { return true; }
    bool drawAsTiledImageRect(SkCanvas*,
                              const SkImage*,
                              const SkRect* src,
                              const SkRect& dst,
                              const SkSamplingOptions&,
                              const SkPaint&,
                              SkCanvas::SrcRectConstraint) override;
    void drawImageLattice(const SkImage*, const SkCanvas::Lattice&,
                          const SkRect& dst, SkFilterMode, const SkPaint&) override;

    void drawDrawable(SkCanvas*, SkDrawable*, const SkMatrix*) override;

    void drawDevice(SkDevice*, const SkSamplingOptions&, const SkPaint&) override;
    void drawSpecial(SkSpecialImage*, const SkMatrix& localToDevice, const SkSamplingOptions&,
                     const SkPaint&, SkCanvas::SrcRectConstraint) override;

    void drawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4], SkCanvas::QuadAAFlags aaFlags,
                        const SkColor4f& color, SkBlendMode mode) override;
    void drawEdgeAAImageSet(const SkCanvas::ImageSetEntry[], int count, const SkPoint dstClips[],
                            const SkMatrix preViewMatrices[], const SkSamplingOptions&,
                            const SkPaint&, SkCanvas::SrcRectConstraint) override;

    // Assumes the src and dst rects have already been optimized to fit the proxy.
    // Only implemented by the gpu devices.
    // This method is the lowest level draw used for tiled bitmap draws. It doesn't attempt to
    // modify its parameters (e.g., adjust src & dst) but just draws the image however it can. It
    // could, almost, be replaced with a drawEdgeAAImageSet call for the tiled bitmap draw use
    // case but the extra tilemode requirement and the intermediate parameter processing (e.g.,
    // trying to alter the SrcRectConstraint) currently block that.
    void drawEdgeAAImage(const SkImage*,
                         const SkRect& src,
                         const SkRect& dst,
                         const SkPoint dstClip[4],
                         SkCanvas::QuadAAFlags,
                         const SkMatrix& localToDevice,
                         const SkSamplingOptions&,
                         const SkPaint&,
                         SkCanvas::SrcRectConstraint,
                         const SkMatrix& srcToDst,
                         SkTileMode);

    sk_sp<sktext::gpu::Slug> convertGlyphRunListToSlug(
            const sktext::GlyphRunList& glyphRunList,
            const SkPaint& initialPaint,
            const SkPaint& drawingPaint) override;

    void drawSlug(SkCanvas*, const sktext::gpu::Slug* slug, const SkPaint& drawingPaint) override;

    sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkImage*) override;
    sk_sp<SkSpecialImage> snapSpecial(const SkIRect& subset, bool forceCopy = false) override;
    sk_sp<SkSpecialImage> snapSpecialScaled(const SkIRect& subset, const SkISize& dstDims) override;

    sk_sp<SkDevice> createDevice(const CreateInfo&, const SkPaint*) override;

    sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&) override;

    Device* asGaneshDevice() override { return this; }

    SkIRect devClipBounds() const override { return fClip.getConservativeBounds(); }

    void pushClipStack() override { fClip.save(); }
    void popClipStack() override { fClip.restore(); }

    void clipRect(const SkRect& rect, SkClipOp op, bool aa) override {
        SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
        fClip.clipRect(this->localToDevice(), rect, GrAA(aa), op);
    }
    void clipRRect(const SkRRect& rrect, SkClipOp op, bool aa) override {
        SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
        fClip.clipRRect(this->localToDevice(), rrect, GrAA(aa), op);
    }
    void clipPath(const SkPath& path, SkClipOp op, bool aa) override;

    void replaceClip(const SkIRect& rect) override {
        // Transform from "global/canvas" coordinates to relative to this device
        SkRect deviceRect = SkMatrixPriv::MapRect(this->globalToDevice(), SkRect::Make(rect));
        fClip.replaceClip(deviceRect.round());
    }
    void clipRegion(const SkRegion& globalRgn, SkClipOp op) override;

    bool isClipAntiAliased() const override;

    bool isClipEmpty() const override {
        return fClip.clipState() == ClipStack::ClipState::kEmpty;
    }

    bool isClipRect() const override {
        return fClip.clipState() == ClipStack::ClipState::kDeviceRect ||
               fClip.clipState() == ClipStack::ClipState::kWideOpen;
    }

    bool isClipWideOpen() const override {
        return fClip.clipState() == ClipStack::ClipState::kWideOpen;
    }

    void android_utils_clipAsRgn(SkRegion*) const override;
    bool android_utils_clipWithStencil() override;

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

    void onDrawGlyphRunList(SkCanvas*,
                            const sktext::GlyphRunList&,
                            const SkPaint& initialPaint,
                            const SkPaint& drawingPaint) override;

    bool onReadPixels(const SkPixmap&, int, int) override;
    bool onWritePixels(const SkPixmap&, int, int) override;
    bool onAccessPixels(SkPixmap*) override;

    sk_sp<skif::Backend> createImageFilteringBackend(const SkSurfaceProps& surfaceProps,
                                                     SkColorType colorType) const override;

    void onClipShader(sk_sp<SkShader> shader) override {
        fClip.clipShader(std::move(shader));
    }

    const GrClip* clip() const { return &fClip; }

    // If not null, dstClip must be contained inside dst and will also respect the edge AA flags.
    // If 'preViewMatrix' is not null, final CTM will be this->ctm() * preViewMatrix.
    void drawImageQuadDirect(const SkImage*,
                             const SkRect& src,
                             const SkRect& dst,
                             const SkPoint dstClip[4],
                             SkCanvas::QuadAAFlags,
                             const SkMatrix* preViewMatrix,
                             const SkSamplingOptions&,
                             const SkPaint&,
                             SkCanvas::SrcRectConstraint);

    // FIXME(michaelludwig) - Should be removed in favor of using drawImageQuad with edge flags to
    // for every element in the SkLatticeIter.
    void drawViewLattice(GrSurfaceProxyView,
                         const GrColorInfo& colorInfo,
                         std::unique_ptr<SkLatticeIter>,
                         const SkRect& dst,
                         SkFilterMode,
                         const SkPaint&);

    friend class ::SkSurface_Ganesh;  // for access to surfaceProps
    friend class skgpu::TiledTextureUtils;   // for access to clip()
};

GR_MAKE_BITFIELD_CLASS_OPS(Device::DeviceFlags)

}  // namespace skgpu::ganesh

#endif // skgpu_v1_Device_DEFINED
