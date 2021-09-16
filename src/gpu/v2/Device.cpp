/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/v2/Device_v2.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkImageFilterCache.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/v2/SurfaceDrawContext_v2.h"

#define ASSERT_SINGLE_OWNER GR_ASSERT_SINGLE_OWNER(fContext->priv().singleOwner())

namespace skgpu::v2 {

sk_sp<BaseDevice> Device::Make(std::unique_ptr<SurfaceDrawContext> sdc,
                               SkAlphaType alphaType,
                               InitContents init) {
    if (!sdc) {
        return nullptr;
    }

    GrRecordingContext* rContext = sdc->recordingContext();
    if (rContext->abandoned()) {
        return nullptr;
    }

    SkColorType ct = GrColorTypeToSkColorType(sdc->colorInfo().colorType());

    DeviceFlags flags;
    if (!rContext->colorTypeSupportedAsSurface(ct) ||
        !CheckAlphaTypeAndGetFlags(alphaType, init, &flags)) {
        return nullptr;
    }
    return sk_sp<Device>(new Device(std::move(sdc), flags));
}

sk_sp<BaseDevice> Device::Make(GrRecordingContext* rContext,
                               GrColorType colorType,
                               sk_sp<GrSurfaceProxy> proxy,
                               sk_sp<SkColorSpace> colorSpace,
                               GrSurfaceOrigin origin,
                               const SkSurfaceProps& surfaceProps,
                               InitContents init) {
    if (!rContext || rContext->abandoned()) {
        return nullptr;
    }

    std::unique_ptr<SurfaceDrawContext> sdc = SurfaceDrawContext::Make(rContext,
                                                                       colorType,
                                                                       std::move(proxy),
                                                                       std::move(colorSpace),
                                                                       origin,
                                                                       surfaceProps);

    return Device::Make(std::move(sdc), kPremul_SkAlphaType, init);
}

sk_sp<BaseDevice> Device::Make(GrRecordingContext* rContext,
                               SkBudgeted budgeted,
                               const SkImageInfo& ii,
                               SkBackingFit fit,
                               int sampleCount,
                               GrMipmapped mipMapped,
                               GrProtected isProtected,
                               GrSurfaceOrigin origin,
                               const SkSurfaceProps& surfaceProps,
                               InitContents init) {
    if (!rContext || rContext->abandoned()) {
        return nullptr;
    }

    auto sdc = SurfaceDrawContext::Make(rContext,
                                        SkColorTypeToGrColorType(ii.colorType()),
                                        ii.refColorSpace(),
                                        fit,
                                        ii.dimensions(),
                                        surfaceProps,
                                        sampleCount,
                                        mipMapped,
                                        isProtected,
                                        origin,
                                        budgeted);

    return Device::Make(std::move(sdc), ii.alphaType(), init);
}

Device::Device(std::unique_ptr<SurfaceDrawContext> sdc, DeviceFlags flags)
        : BaseDevice(sk_ref_sp(sdc->recordingContext()),
                     MakeInfo(sdc.get(), flags),
                     sdc->surfaceProps())
        , fSurfaceDrawContext(std::move(sdc)) {
    if (flags & DeviceFlags::kNeedClear) {
        // TODO: re-enable this once we decide what the V2 SDC does with clearAll calls
//        this->clearAll();
    }
}

Device::~Device() {}

skgpu::SurfaceFillContext* Device::surfaceFillContext() { return fSurfaceDrawContext.get(); }

void Device::asyncRescaleAndReadPixels(const SkImageInfo& info,
                                       const SkIRect& srcRect,
                                       RescaleGamma rescaleGamma,
                                       RescaleMode rescaleMode,
                                       ReadPixelsCallback callback,
                                       ReadPixelsContext context) {
    // Context TODO: Elevate direct context requirement to public API.
    auto dContext = this->recordingContext()->asDirectContext();
    if (!dContext) {
        return;
    }
}

void Device::asyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                             sk_sp<SkColorSpace> dstColorSpace,
                                             const SkIRect& srcRect,
                                             SkISize dstSize,
                                             RescaleGamma rescaleGamma,
                                             RescaleMode,
                                             ReadPixelsCallback callback,
                                             ReadPixelsContext context) {
    // Context TODO: Elevate direct context requirement to public API.
    auto dContext = this->recordingContext()->asDirectContext();
    if (!dContext) {
        return;
    }
}

void Device::onSave() {
}

void Device::onRestore() {
}

void Device::onClipRect(const SkRect& rect, SkClipOp op, bool aa) {
}

void Device::onClipRRect(const SkRRect& rrect, SkClipOp op, bool aa) {
}

void Device::onClipPath(const SkPath& path, SkClipOp op, bool aa) {
    SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
}

void Device::onClipShader(sk_sp<SkShader> shader) {
}

void Device::onClipRegion(const SkRegion& globalRgn, SkClipOp op) {
    SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
}

void Device::onReplaceClip(const SkIRect& rect) {
}

bool Device::onClipIsAA() const {
    return false;
}

bool Device::onClipIsWideOpen() const {
    return false;
}

void Device::onAsRgnClip(SkRegion* region) const {
}

SkBaseDevice::ClipType Device::onGetClipType() const {
    return ClipType::kEmpty;
}

SkIRect Device::onDevClipBounds() const {
    return SkIRect::MakeEmpty();
}

void Device::drawPaint(const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawPaint", fContext.get());
}

void Device::drawPoints(SkCanvas::PointMode mode,
                                 size_t count,
                                 const SkPoint points[],
                                 const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawPoints", fContext.get());

}

void Device::drawRect(const SkRect& rect, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawRect", fContext.get());
}

void Device::drawRegion(const SkRegion& r, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawRegion", fContext.get());
}

void Device::drawOval(const SkRect& oval, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawOval", fContext.get());
}

void Device::drawArc(const SkRect& oval,
                     SkScalar startAngle,
                     SkScalar sweepAngle,
                     bool useCenter,
                     const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawArc", fContext.get());
}

void Device::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawRRect", fContext.get());
}

void Device::drawDRRect(const SkRRect& outer,
                        const SkRRect& inner,
                        const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawDRRect", fContext.get());
}

void Device::drawPath(const SkPath& path, const SkPaint& paint, bool pathIsMutable) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawPath", fContext.get());
}

void Device::drawImageRect(const SkImage* image,
                           const SkRect* src,
                           const SkRect& dst,
                           const SkSamplingOptions& sampling,
                           const SkPaint& paint,
                           SkCanvas::SrcRectConstraint constraint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawImageRect", fContext.get());
}

void Device::drawImageLattice(const SkImage* image,
                              const SkCanvas::Lattice& lattice,
                              const SkRect& dst,
                              SkFilterMode filter,
                              const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawImageLattice", fContext.get());
}

void Device::drawVertices(const SkVertices* vertices,
                          SkBlendMode mode,
                          const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawVertices", fContext.get());
}

void Device::drawShadow(const SkPath& path, const SkDrawShadowRec& rec) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawShadow", fContext.get());
}

void Device::drawAtlas(const SkImage* atlas,
                       const SkRSXform xform[],
                       const SkRect texRect[],
                       const SkColor colors[],
                       int count,
                       SkBlendMode mode,
                       const SkSamplingOptions& sampling,
                       const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawAtlas", fContext.get());
}

void Device::drawEdgeAAQuad(const SkRect& rect,
                            const SkPoint clip[4],
                            SkCanvas::QuadAAFlags aaFlags,
                            const SkColor4f& color,
                            SkBlendMode mode) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawEdgeAAQuad", fContext.get());
}

void Device::drawEdgeAAImageSet(const SkCanvas::ImageSetEntry set[],
                                int count,
                                const SkPoint dstClips[],
                                const SkMatrix preViewMatrices[],
                                const SkSamplingOptions& sampling,
                                const SkPaint& paint,
                                SkCanvas::SrcRectConstraint constraint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawEdgeAAImageSet", fContext.get());
}

void Device::drawDrawable(SkDrawable* drawable,
                          const SkMatrix* matrix,
                          SkCanvas* canvas) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawDrawable", fContext.get());

    this->INHERITED::drawDrawable(drawable, matrix, canvas);
}

void Device::onDrawGlyphRunList(const SkGlyphRunList& glyphRunList, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "onDrawGlyphRunList", fContext.get());
}

void Device::drawDevice(SkBaseDevice* device,
                        const SkSamplingOptions& sampling,
                        const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v2::Device", "drawDevice", fContext.get());
}

void Device::drawSpecial(SkSpecialImage* special,
                         const SkMatrix& localToDevice,
                         const SkSamplingOptions& sampling,
                         const SkPaint& paint) {
}

sk_sp<SkSpecialImage> Device::makeSpecial(const SkBitmap& bitmap) {
    ASSERT_SINGLE_OWNER
    return nullptr;
}

sk_sp<SkSpecialImage> Device::makeSpecial(const SkImage* image) {
    ASSERT_SINGLE_OWNER
    return nullptr;
}

sk_sp<SkSpecialImage> Device::snapSpecial(const SkIRect& subset, bool forceCopy) {
    ASSERT_SINGLE_OWNER
    return nullptr;
}

sk_sp<SkSurface> Device::makeSurface(const SkImageInfo& ii,
                                     const SkSurfaceProps& props) {
    ASSERT_SINGLE_OWNER
    return nullptr;
}

bool Device::onReadPixels(const SkPixmap& pm, int x, int y) {
    ASSERT_SINGLE_OWNER
    return false;
}

bool Device::onWritePixels(const SkPixmap& pm, int x, int y) {
    ASSERT_SINGLE_OWNER
    return false;
}

bool Device::onAccessPixels(SkPixmap*) {
    ASSERT_SINGLE_OWNER
    return false;
}

SkBaseDevice* Device::onCreateDevice(const CreateInfo& cinfo, const SkPaint*) {
    ASSERT_SINGLE_OWNER
    return nullptr;
}

bool Device::forceConservativeRasterClip() const {
    return true;
}

SkImageFilterCache* Device::getImageFilterCache() {
    ASSERT_SINGLE_OWNER

    // We always return a transient cache, so it is freed after each filter traversal.
    return SkImageFilterCache::Create(SkImageFilterCache::kDefaultTransientSize);
}

} // namespace skgpu::v2
