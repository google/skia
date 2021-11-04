/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/v1/Device_v1.h"

#include "include/core/SkImageFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPicture.h"
#include "include/core/SkSurface.h"
#include "include/core/SkVertices.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/SkShadowFlags.h"
#include "include/private/SkTo.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkClipStack.h"
#include "src/core/SkDraw.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkLatticeIter.h"
#include "src/core/SkPictureData.h"
#include "src/core/SkRRectPriv.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkRecord.h"
#include "src/core/SkStroke.h"
#include "src/core/SkTLazy.h"
#include "src/core/SkVerticesPriv.h"
#include "src/gpu/GrBlurUtils.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrDisableColorXP.h"
#include "src/gpu/effects/GrRRectEffect.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkReadPixelsRec.h"
#include "src/image/SkSurface_Gpu.h"
#include "src/utils/SkUTF.h"

#define ASSERT_SINGLE_OWNER GR_ASSERT_SINGLE_OWNER(fContext->priv().singleOwner())


///////////////////////////////////////////////////////////////////////////////

namespace {

bool force_aa_clip(const skgpu::v1::SurfaceDrawContext* sdc) {
    return sdc->numSamples() > 1 || sdc->alwaysAntialias();
}

inline GrPrimitiveType point_mode_to_primitive_type(SkCanvas::PointMode mode) {
    switch (mode) {
        case SkCanvas::kPoints_PointMode:
            return GrPrimitiveType::kPoints;
        case SkCanvas::kLines_PointMode:
            return GrPrimitiveType::kLines;
        case SkCanvas::kPolygon_PointMode:
            return GrPrimitiveType::kLineStrip;
    }
    SK_ABORT("Unexpected mode");
}

std::unique_ptr<GrFragmentProcessor> make_inverse_rrect_fp(const SkMatrix& viewMatrix,
                                                           const SkRRect& rrect, GrAA aa,
                                                           const GrShaderCaps& shaderCaps) {
    SkTCopyOnFirstWrite<SkRRect> devRRect(rrect);
    if (viewMatrix.isIdentity() || rrect.transform(viewMatrix, devRRect.writable())) {
        auto edgeType = (aa == GrAA::kYes) ? GrClipEdgeType::kInverseFillAA
                                           : GrClipEdgeType::kInverseFillBW;
        auto [success, fp] = GrRRectEffect::Make(/*inputFP=*/nullptr, edgeType, *devRRect,
                                                 shaderCaps);
        return (success) ? std::move(fp) : nullptr;
    }
    return nullptr;
}

bool init_vertices_paint(GrRecordingContext* rContext,
                         const GrColorInfo& colorInfo,
                         const SkPaint& skPaint,
                         const SkMatrixProvider& matrixProvider,
                         SkBlendMode bmode,
                         bool hasColors,
                         GrPaint* grPaint) {
    if (hasColors) {
        // When there are colors and a shader, the shader and colors are combined using bmode.
        // With no shader, we just use the colors (kDst).
        return SkPaintToGrPaintWithBlend(rContext,
                                         colorInfo,
                                         skPaint,
                                         matrixProvider,
                                         skPaint.getShader() ? bmode : SkBlendMode::kDst,
                                         grPaint);
    } else {
        return SkPaintToGrPaint(rContext, colorInfo, skPaint, matrixProvider, grPaint);
    }
}

} // anonymous namespace

namespace skgpu::v1 {

sk_sp<BaseDevice> Device::Make(GrRecordingContext* rContext,
                               GrColorType colorType,
                               sk_sp<GrSurfaceProxy> proxy,
                               sk_sp<SkColorSpace> colorSpace,
                               GrSurfaceOrigin origin,
                               const SkSurfaceProps& surfaceProps,
                               InitContents init) {
    auto sdc = SurfaceDrawContext::Make(rContext,
                                        colorType,
                                        std::move(proxy),
                                        std::move(colorSpace),
                                        origin,
                                        surfaceProps);

    return Device::Make(std::move(sdc), kPremul_SkAlphaType, init);
}

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
                               SkBudgeted budgeted,
                               const SkImageInfo& ii,
                               SkBackingFit fit,
                               int sampleCount,
                               GrMipmapped mipMapped,
                               GrProtected isProtected,
                               GrSurfaceOrigin origin,
                               const SkSurfaceProps& props,
                               InitContents init) {
    if (!rContext) {
        return nullptr;
    }

    auto sdc = SurfaceDrawContext::Make(rContext,
                                        SkColorTypeToGrColorType(ii.colorType()),
                                        ii.refColorSpace(),
                                        fit,
                                        ii.dimensions(),
                                        props,
                                        sampleCount,
                                        mipMapped,
                                        isProtected,
                                        origin,
                                        budgeted);

    return Device::Make(std::move(sdc), ii.alphaType(), init);
}

Device::Device(std::unique_ptr<SurfaceDrawContext> sdc, DeviceFlags flags)
        : INHERITED(sk_ref_sp(sdc->recordingContext()),
                    MakeInfo(sdc.get(), flags),
                    sdc->surfaceProps())
        , fSurfaceDrawContext(std::move(sdc))
        , fClip(SkIRect::MakeSize(fSurfaceDrawContext->dimensions()),
                &this->asMatrixProvider(),
                force_aa_clip(fSurfaceDrawContext.get())) {
    if (flags & DeviceFlags::kNeedClear) {
        this->clearAll();
    }
}

///////////////////////////////////////////////////////////////////////////////

bool Device::onReadPixels(const SkPixmap& pm, int x, int y) {
    ASSERT_SINGLE_OWNER

    // Context TODO: Elevate direct context requirement to public API
    auto dContext = fContext->asDirectContext();
    if (!dContext || !SkImageInfoValidConversion(pm.info(), this->imageInfo())) {
        return false;
    }

    return fSurfaceDrawContext->readPixels(dContext, pm, {x, y});
}

bool Device::onWritePixels(const SkPixmap& pm, int x, int y) {
    ASSERT_SINGLE_OWNER

    // Context TODO: Elevate direct context requirement to public API
    auto dContext = fContext->asDirectContext();
    if (!dContext || !SkImageInfoValidConversion(this->imageInfo(), pm.info())) {
        return false;
    }

    return fSurfaceDrawContext->writePixels(dContext, pm, {x, y});
}

bool Device::onAccessPixels(SkPixmap* pmap) {
    ASSERT_SINGLE_OWNER
    return false;
}

SurfaceDrawContext* Device::surfaceDrawContext() {
    ASSERT_SINGLE_OWNER
    return fSurfaceDrawContext.get();
}

const SurfaceDrawContext* Device::surfaceDrawContext() const {
    ASSERT_SINGLE_OWNER
    return fSurfaceDrawContext.get();
}

skgpu::SurfaceFillContext* Device::surfaceFillContext() {
    ASSERT_SINGLE_OWNER
    return fSurfaceDrawContext.get();
}

void Device::clearAll() {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v1::Device", "clearAll", fContext.get());

    SkIRect rect = SkIRect::MakeWH(this->width(), this->height());
    fSurfaceDrawContext->clearAtLeast(rect, SK_PMColor4fTRANSPARENT);
}

///////////////////////////////////////////////////////////////////////////////

void Device::onClipPath(const SkPath& path, SkClipOp op, bool aa) {
#if GR_TEST_UTILS
    if (fContext->priv().options().fAllPathsVolatile && !path.isVolatile()) {
        this->onClipPath(SkPath(path).setIsVolatile(true), op, aa);
        return;
    }
#endif
    SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
    fClip.clipPath(this->localToDevice(), path, GrAA(aa), op);
}

void Device::onClipRegion(const SkRegion& globalRgn, SkClipOp op) {
    SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);

    // Regions don't actually need AA, but in DMSAA mode every clip element is antialiased.
    GrAA aa = GrAA(fSurfaceDrawContext->alwaysAntialias());

    if (globalRgn.isEmpty()) {
        fClip.clipRect(SkMatrix::I(), SkRect::MakeEmpty(), aa, op);
    } else if (globalRgn.isRect()) {
        fClip.clipRect(this->globalToDevice().asM33(), SkRect::Make(globalRgn.getBounds()), aa, op);
    } else {
        SkPath path;
        globalRgn.getBoundaryPath(&path);
        fClip.clipPath(this->globalToDevice().asM33(), path, aa, op);
    }
}

void Device::onAsRgnClip(SkRegion* region) const {
    SkIRect bounds = fClip.getConservativeBounds();
    // Assume wide open and then perform intersect/difference operations reducing the region
    region->setRect(bounds);
    const SkRegion deviceBounds(bounds);
    for (const ClipStack::Element& e : fClip) {
        SkRegion tmp;
        if (e.fShape.isRect() && e.fLocalToDevice.isIdentity()) {
            tmp.setRect(e.fShape.rect().roundOut());
        } else {
            SkPath tmpPath;
            e.fShape.asPath(&tmpPath);
            tmpPath.transform(e.fLocalToDevice);
            tmp.setPath(tmpPath, deviceBounds);
        }

        region->op(tmp, (SkRegion::Op) e.fOp);
    }
}

bool Device::onClipIsAA() const {
    for (const ClipStack::Element& e : fClip) {
        if (e.fAA == GrAA::kYes) {
            return true;
        }
        SkASSERT(!fSurfaceDrawContext->alwaysAntialias());
    }
    return false;
}

SkBaseDevice::ClipType Device::onGetClipType() const {
    ClipStack::ClipState state = fClip.clipState();
    if (state == ClipStack::ClipState::kEmpty) {
        return ClipType::kEmpty;
    } else if (state == ClipStack::ClipState::kDeviceRect ||
               state == ClipStack::ClipState::kWideOpen) {
        return ClipType::kRect;
    } else {
        return ClipType::kComplex;
    }
}

///////////////////////////////////////////////////////////////////////////////

void Device::drawPaint(const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v1::Device", "drawPaint", fContext.get());

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->recordingContext(), fSurfaceDrawContext->colorInfo(), paint,
                          this->asMatrixProvider(), &grPaint)) {
        return;
    }

    fSurfaceDrawContext->drawPaint(this->clip(), std::move(grPaint), this->localToDevice());
}

void Device::drawPoints(SkCanvas::PointMode mode,
                        size_t count,
                        const SkPoint pts[],
                        const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v1::Device", "drawPoints", fContext.get());
    SkScalar width = paint.getStrokeWidth();
    if (width < 0) {
        return;
    }

    GrAA aa = fSurfaceDrawContext->chooseAA(paint);

    if (paint.getPathEffect() && 2 == count && SkCanvas::kLines_PointMode == mode) {
        GrStyle style(paint, SkPaint::kStroke_Style);
        GrPaint grPaint;
        if (!SkPaintToGrPaint(this->recordingContext(), fSurfaceDrawContext->colorInfo(), paint,
                              this->asMatrixProvider(), &grPaint)) {
            return;
        }
        SkPath path;
        path.setIsVolatile(true);
        path.moveTo(pts[0]);
        path.lineTo(pts[1]);
        fSurfaceDrawContext->drawPath(this->clip(), std::move(grPaint), aa, this->localToDevice(),
                                      path, style);
        return;
    }

    SkScalar scales[2];
    bool isHairline = (0 == width) ||
                       (1 == width && this->localToDevice().getMinMaxScales(scales) &&
                        SkScalarNearlyEqual(scales[0], 1.f) && SkScalarNearlyEqual(scales[1], 1.f));
    // we only handle non-coverage-aa hairlines and paints without path effects or mask filters,
    // else we let the SkDraw call our drawPath()
    if (!isHairline ||
        paint.getPathEffect() ||
        paint.getMaskFilter() ||
        fSurfaceDrawContext->chooseAAType(aa) == GrAAType::kCoverage) {
        SkRasterClip rc(this->devClipBounds());
        SkDraw draw;
        draw.fDst = SkPixmap(SkImageInfo::MakeUnknown(this->width(), this->height()), nullptr, 0);
        draw.fMatrixProvider = this;
        draw.fRC = &rc;
        draw.drawPoints(mode, count, pts, paint, this);
        return;
    }

    GrPrimitiveType primitiveType = point_mode_to_primitive_type(mode);

    const SkMatrixProvider* matrixProvider = this;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    SkTLazy<SkPostTranslateMatrixProvider> postTranslateMatrixProvider;
    // This offsetting in device space matches the expectations of the Android framework for non-AA
    // points and lines.
    if (GrIsPrimTypeLines(primitiveType) || GrPrimitiveType::kPoints == primitiveType) {
        static const SkScalar kOffset = 0.063f; // Just greater than 1/16.
        matrixProvider = postTranslateMatrixProvider.init(*matrixProvider, kOffset, kOffset);
    }
#endif

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->recordingContext(), fSurfaceDrawContext->colorInfo(), paint,
                          *matrixProvider, &grPaint)) {
        return;
    }

    static constexpr SkVertices::VertexMode kIgnoredMode = SkVertices::kTriangles_VertexMode;
    sk_sp<SkVertices> vertices = SkVertices::MakeCopy(kIgnoredMode, SkToS32(count), pts, nullptr,
                                                      nullptr);

    fSurfaceDrawContext->drawVertices(this->clip(), std::move(grPaint), *matrixProvider,
                                      std::move(vertices), &primitiveType);
}

///////////////////////////////////////////////////////////////////////////////

void Device::drawRect(const SkRect& rect, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v1::Device", "drawRect", fContext.get());

    GrStyle style(paint);

    // A couple reasons we might need to call drawPath.
    if (paint.getMaskFilter() || paint.getPathEffect()) {
        GrStyledShape shape(rect, style);

        GrBlurUtils::drawShapeWithMaskFilter(fContext.get(), fSurfaceDrawContext.get(),
                                             this->clip(), paint, this->asMatrixProvider(), shape);
        return;
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->recordingContext(), fSurfaceDrawContext->colorInfo(), paint,
                          this->asMatrixProvider(), &grPaint)) {
        return;
    }

    fSurfaceDrawContext->drawRect(this->clip(), std::move(grPaint),
                                  fSurfaceDrawContext->chooseAA(paint), this->localToDevice(), rect,
                                  &style);
}

void Device::drawEdgeAAQuad(const SkRect& rect,
                            const SkPoint clip[4],
                            SkCanvas::QuadAAFlags aaFlags,
                            const SkColor4f& color,
                            SkBlendMode mode) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v1::Device", "drawEdgeAAQuad", fContext.get());

    SkPMColor4f dstColor = SkColor4fPrepForDst(color, fSurfaceDrawContext->colorInfo()).premul();

    GrPaint grPaint;
    grPaint.setColor4f(dstColor);
    if (mode != SkBlendMode::kSrcOver) {
        grPaint.setXPFactory(SkBlendMode_AsXPFactory(mode));
    }

    // This is exclusively meant for tiling operations, so keep AA enabled to handle MSAA seaming
    GrQuadAAFlags grAA = SkToGrQuadAAFlags(aaFlags);
    if (clip) {
        // Use fillQuadWithEdgeAA
        fSurfaceDrawContext->fillQuadWithEdgeAA(this->clip(), std::move(grPaint), GrAA::kYes, grAA,
                                                this->localToDevice(), clip, nullptr);
    } else {
        // Use fillRectWithEdgeAA to preserve mathematical properties of dst being rectangular
        fSurfaceDrawContext->fillRectWithEdgeAA(this->clip(), std::move(grPaint), GrAA::kYes, grAA,
                                                this->localToDevice(), rect);
    }
}

///////////////////////////////////////////////////////////////////////////////

void Device::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v1::Device", "drawRRect", fContext.get());

    SkMaskFilterBase* mf = as_MFB(paint.getMaskFilter());
    if (mf) {
        if (mf->hasFragmentProcessor()) {
            mf = nullptr; // already handled in SkPaintToGrPaint
        }
    }

    GrStyle style(paint);

    if (mf || style.pathEffect()) {
        // A path effect will presumably transform this rrect into something else.
        GrStyledShape shape(rrect, style);

        GrBlurUtils::drawShapeWithMaskFilter(fContext.get(), fSurfaceDrawContext.get(),
                                             this->clip(), paint, this->asMatrixProvider(), shape);
        return;
    }

    SkASSERT(!style.pathEffect());

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->recordingContext(), fSurfaceDrawContext->colorInfo(), paint,
                          this->asMatrixProvider(), &grPaint)) {
        return;
    }

    fSurfaceDrawContext->drawRRect(this->clip(), std::move(grPaint),
                                   fSurfaceDrawContext->chooseAA(paint), this->localToDevice(),
                                   rrect, style);
}

void Device::drawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v1::Device", "drawDRRect", fContext.get());
    if (outer.isEmpty()) {
       return;
    }

    if (inner.isEmpty()) {
        return this->drawRRect(outer, paint);
    }

    SkStrokeRec stroke(paint);

    if (stroke.isFillStyle() && !paint.getMaskFilter() && !paint.getPathEffect()) {
        // For axis-aligned filled DRRects, just draw a regular rrect with inner clipped out using a
        // coverage FP instead of using path rendering.
        if (auto fp = make_inverse_rrect_fp(this->localToDevice(), inner,
                                            fSurfaceDrawContext->chooseAA(paint),
                                            *fSurfaceDrawContext->caps()->shaderCaps())) {
            GrPaint grPaint;
            if (!SkPaintToGrPaint(this->recordingContext(), fSurfaceDrawContext->colorInfo(), paint,
                                  this->asMatrixProvider(), &grPaint)) {
                return;
            }
            SkASSERT(!grPaint.hasCoverageFragmentProcessor());
            grPaint.setCoverageFragmentProcessor(std::move(fp));
            fSurfaceDrawContext->drawRRect(this->clip(), std::move(grPaint),
                                           fSurfaceDrawContext->chooseAA(paint),
                                           this->localToDevice(), outer, GrStyle());
            return;
        }
    }

    SkPath path;
    path.setIsVolatile(true);
    path.addRRect(outer);
    path.addRRect(inner);
    path.setFillType(SkPathFillType::kEvenOdd);

    // TODO: We are losing the possible mutability of the path here but this should probably be
    // fixed by upgrading GrStyledShape to handle DRRects.
    GrStyledShape shape(path, paint);

    GrBlurUtils::drawShapeWithMaskFilter(fContext.get(), fSurfaceDrawContext.get(), this->clip(),
                                         paint, this->asMatrixProvider(), shape);
}

/////////////////////////////////////////////////////////////////////////////

void Device::drawRegion(const SkRegion& region, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER

    if (paint.getMaskFilter()) {
        SkPath path;
        region.getBoundaryPath(&path);
        path.setIsVolatile(true);
        return this->drawPath(path, paint, true);
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->recordingContext(), fSurfaceDrawContext->colorInfo(), paint,
                          this->asMatrixProvider(), &grPaint)) {
        return;
    }

    fSurfaceDrawContext->drawRegion(this->clip(), std::move(grPaint),
                                    fSurfaceDrawContext->chooseAA(paint), this->localToDevice(),
                                    region, GrStyle(paint));
}

void Device::drawOval(const SkRect& oval, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v1::Device", "drawOval", fContext.get());

    if (paint.getMaskFilter()) {
        // The RRect path can handle special case blurring
        SkRRect rr = SkRRect::MakeOval(oval);
        return this->drawRRect(rr, paint);
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->recordingContext(), fSurfaceDrawContext->colorInfo(), paint,
                          this->asMatrixProvider(), &grPaint)) {
        return;
    }

    fSurfaceDrawContext->drawOval(this->clip(), std::move(grPaint),
                                  fSurfaceDrawContext->chooseAA(paint), this->localToDevice(), oval,
                                  GrStyle(paint));
}

void Device::drawArc(const SkRect& oval,
                     SkScalar startAngle,
                     SkScalar sweepAngle,
                     bool useCenter,
                     const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v1::Device", "drawArc", fContext.get());
    if (paint.getMaskFilter()) {
        this->INHERITED::drawArc(oval, startAngle, sweepAngle, useCenter, paint);
        return;
    }
    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->recordingContext(), fSurfaceDrawContext->colorInfo(), paint,
                          this->asMatrixProvider(), &grPaint)) {
        return;
    }

    fSurfaceDrawContext->drawArc(this->clip(), std::move(grPaint),
                                 fSurfaceDrawContext->chooseAA(paint), this->localToDevice(), oval,
                                 startAngle, sweepAngle, useCenter, GrStyle(paint));
}

///////////////////////////////////////////////////////////////////////////////

void Device::drawPath(const SkPath& origSrcPath, const SkPaint& paint, bool pathIsMutable) {
#if GR_TEST_UTILS
    if (fContext->priv().options().fAllPathsVolatile && !origSrcPath.isVolatile()) {
        this->drawPath(SkPath(origSrcPath).setIsVolatile(true), paint, true);
        return;
    }
#endif
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v1::Device", "drawPath", fContext.get());
    if (!paint.getMaskFilter()) {
        GrPaint grPaint;
        if (!SkPaintToGrPaint(this->recordingContext(), fSurfaceDrawContext->colorInfo(), paint,
                              this->asMatrixProvider(), &grPaint)) {
            return;
        }
        fSurfaceDrawContext->drawPath(this->clip(), std::move(grPaint),
                                      fSurfaceDrawContext->chooseAA(paint), this->localToDevice(),
                                      origSrcPath, GrStyle(paint));
        return;
    }

    // TODO: losing possible mutability of 'origSrcPath' here
    GrStyledShape shape(origSrcPath, paint);

    GrBlurUtils::drawShapeWithMaskFilter(fContext.get(), fSurfaceDrawContext.get(), this->clip(),
                                         paint, this->asMatrixProvider(), shape);
}

sk_sp<SkSpecialImage> Device::makeSpecial(const SkBitmap& bitmap) {
    ASSERT_SINGLE_OWNER

    // TODO: this makes a tight copy of 'bitmap' but it doesn't have to be (given SkSpecialImage's
    // semantics). Since this is cached we would have to bake the fit into the cache key though.
    auto view = std::get<0>(GrMakeCachedBitmapProxyView(fContext.get(), bitmap));
    if (!view) {
        return nullptr;
    }

    const SkIRect rect = SkIRect::MakeSize(view.proxy()->dimensions());

    // GrMakeCachedBitmapProxyView creates a tight copy of 'bitmap' so we don't have to subset
    // the special image
    return SkSpecialImage::MakeDeferredFromGpu(fContext.get(),
                                               rect,
                                               bitmap.getGenerationID(),
                                               std::move(view),
                                               SkColorTypeToGrColorType(bitmap.colorType()),
                                               bitmap.refColorSpace(),
                                               this->surfaceProps());
}

sk_sp<SkSpecialImage> Device::makeSpecial(const SkImage* image) {
    ASSERT_SINGLE_OWNER

    SkPixmap pm;
    if (image->isTextureBacked()) {
        auto [view, ct] = as_IB(image)->asView(this->recordingContext(), GrMipmapped::kNo);
        SkASSERT(view);

        return SkSpecialImage::MakeDeferredFromGpu(fContext.get(),
                                                   SkIRect::MakeWH(image->width(), image->height()),
                                                   image->uniqueID(),
                                                   std::move(view),
                                                   ct,
                                                   image->refColorSpace(),
                                                   this->surfaceProps());
    } else if (image->peekPixels(&pm)) {
        SkBitmap bm;

        bm.installPixels(pm);
        return this->makeSpecial(bm);
    } else {
        return nullptr;
    }
}

sk_sp<SkSpecialImage> Device::snapSpecial(const SkIRect& subset, bool forceCopy) {
    ASSERT_SINGLE_OWNER

    auto sdc = fSurfaceDrawContext.get();

    // If we are wrapping a vulkan secondary command buffer, then we can't snap off a special image
    // since it would require us to make a copy of the underlying VkImage which we don't have access
    // to. Additionaly we can't stop and start the render pass that is used with the secondary
    // command buffer.
    if (sdc->wrapsVkSecondaryCB()) {
        return nullptr;
    }

    SkASSERT(sdc->asSurfaceProxy());

    SkIRect finalSubset = subset;
    GrSurfaceProxyView view = sdc->readSurfaceView();
    if (forceCopy || !view.asTextureProxy()) {
        // When the device doesn't have a texture, or a copy is requested, we create a temporary
        // texture that matches the device contents
        view = GrSurfaceProxyView::Copy(fContext.get(),
                                        std::move(view),
                                        GrMipmapped::kNo,  // Don't auto generate mips
                                        subset,
                                        SkBackingFit::kApprox,
                                        SkBudgeted::kYes);  // Always budgeted
        if (!view) {
            return nullptr;
        }
        // Since this copied only the requested subset, the special image wrapping the proxy no
        // longer needs the original subset.
        finalSubset = SkIRect::MakeSize(view.dimensions());
    }

    GrColorType ct = SkColorTypeToGrColorType(this->imageInfo().colorType());

    return SkSpecialImage::MakeDeferredFromGpu(fContext.get(),
                                               finalSubset,
                                               kNeedNewImageUniqueID_SpecialImage,
                                               std::move(view),
                                               ct,
                                               this->imageInfo().refColorSpace(),
                                               this->surfaceProps());
}

void Device::drawDevice(SkBaseDevice* device,
                        const SkSamplingOptions& sampling,
                        const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    // clear of the source device must occur before CHECK_SHOULD_DRAW
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v1::Device", "drawDevice", fContext.get());
    this->INHERITED::drawDevice(device, sampling, paint);
}

void Device::drawImageRect(const SkImage* image,
                           const SkRect* src,
                           const SkRect& dst,
                           const SkSamplingOptions& sampling,
                           const SkPaint& paint,
                           SkCanvas::SrcRectConstraint constraint) {
    ASSERT_SINGLE_OWNER
    GrAA aa = fSurfaceDrawContext->chooseAA(paint);
    GrQuadAAFlags aaFlags = (aa == GrAA::kYes) ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone;
    this->drawImageQuad(image, src, &dst, nullptr, aa, aaFlags, nullptr, sampling, paint,
                        constraint);
}

void Device::drawViewLattice(GrSurfaceProxyView view,
                             const GrColorInfo& info,
                             std::unique_ptr<SkLatticeIter> iter,
                             const SkRect& dst,
                             SkFilterMode filter,
                             const SkPaint& origPaint) {
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v1::Device", "drawViewLattice", fContext.get());
    SkASSERT(view);

    SkTCopyOnFirstWrite<SkPaint> paint(&origPaint);

    if (!info.isAlphaOnly() && (paint->getColor() & 0x00FFFFFF) != 0x00FFFFFF) {
        paint.writable()->setColor(SkColorSetARGB(origPaint.getAlpha(), 0xFF, 0xFF, 0xFF));
    }
    GrPaint grPaint;
    if (!SkPaintToGrPaintWithPrimitiveColor(this->recordingContext(),
                                            fSurfaceDrawContext->colorInfo(), *paint,
                                            this->asMatrixProvider(), &grPaint)) {
        return;
    }

    if (info.isAlphaOnly()) {
        // If we were doing this with an FP graph we'd use a kDstIn blend between the texture and
        // the paint color.
        view.concatSwizzle(GrSwizzle("aaaa"));
    }
    auto csxf = GrColorSpaceXform::Make(info, fSurfaceDrawContext->colorInfo());

    fSurfaceDrawContext->drawImageLattice(this->clip(), std::move(grPaint), this->localToDevice(),
                                          std::move(view), info.alphaType(), std::move(csxf),
                                          filter, std::move(iter), dst);
}

void Device::drawImageLattice(const SkImage* image,
                              const SkCanvas::Lattice& lattice,
                              const SkRect& dst,
                              SkFilterMode filter,
                              const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    auto iter = std::make_unique<SkLatticeIter>(lattice, dst);
    if (auto [view, ct] = as_IB(image)->asView(this->recordingContext(), GrMipmapped::kNo); view) {
        GrColorInfo colorInfo(ct, image->alphaType(), image->refColorSpace());
        this->drawViewLattice(std::move(view),
                              std::move(colorInfo),
                              std::move(iter),
                              dst,
                              filter,
                              paint);
    }
}

void Device::drawVertices(const SkVertices* vertices, SkBlendMode mode, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v1::Device", "drawVertices", fContext.get());
    SkASSERT(vertices);

    SkVerticesPriv info(vertices->priv());

    GrPaint grPaint;
    if (!init_vertices_paint(fContext.get(), fSurfaceDrawContext->colorInfo(), paint,
                             this->asMatrixProvider(), mode, info.hasColors(), &grPaint)) {
        return;
    }
    fSurfaceDrawContext->drawVertices(this->clip(),
                                      std::move(grPaint),
                                      this->asMatrixProvider(),
                                      sk_ref_sp(const_cast<SkVertices*>(vertices)),
                                      nullptr);
}

///////////////////////////////////////////////////////////////////////////////

void Device::drawShadow(const SkPath& path, const SkDrawShadowRec& rec) {
#if GR_TEST_UTILS
    if (fContext->priv().options().fAllPathsVolatile && !path.isVolatile()) {
        this->drawShadow(SkPath(path).setIsVolatile(true), rec);
        return;
    }
#endif
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v1::Device", "drawShadow", fContext.get());

    if (!fSurfaceDrawContext->drawFastShadow(this->clip(), this->localToDevice(), path, rec)) {
        // failed to find an accelerated case
        this->INHERITED::drawShadow(path, rec);
    }
}

///////////////////////////////////////////////////////////////////////////////

void Device::drawAtlas(const SkRSXform xform[],
                       const SkRect texRect[],
                       const SkColor colors[],
                       int count,
                       SkBlendMode mode,
                       const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v1::Device", "drawAtlas", fContext.get());

    GrPaint grPaint;
    if (colors) {
        if (!SkPaintToGrPaintWithBlend(this->recordingContext(), fSurfaceDrawContext->colorInfo(),
                                       paint, this->asMatrixProvider(), mode, &grPaint)) {
            return;
        }
    } else {
        if (!SkPaintToGrPaint(this->recordingContext(), fSurfaceDrawContext->colorInfo(),
                              paint, this->asMatrixProvider(), &grPaint)) {
            return;
        }
    }

    fSurfaceDrawContext->drawAtlas(this->clip(), std::move(grPaint), this->localToDevice(), count,
                                   xform, texRect, colors);
}

///////////////////////////////////////////////////////////////////////////////

void Device::onDrawGlyphRunList(const SkGlyphRunList& glyphRunList, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::v1::Device", "drawGlyphRunList", fContext.get());
    SkASSERT(!glyphRunList.hasRSXForm());

    fSurfaceDrawContext->drawGlyphRunList(
        this->clip(), this->asMatrixProvider(), glyphRunList, paint);
}

///////////////////////////////////////////////////////////////////////////////

void Device::drawDrawable(SkDrawable* drawable, const SkMatrix* matrix, SkCanvas* canvas) {
    ASSERT_SINGLE_OWNER

    GrBackendApi api = this->recordingContext()->backend();
    if (GrBackendApi::kVulkan == api) {
        const SkMatrix& ctm = this->localToDevice();
        const SkMatrix& combinedMatrix = matrix ? SkMatrix::Concat(ctm, *matrix) : ctm;
        std::unique_ptr<SkDrawable::GpuDrawHandler> gpuDraw =
                drawable->snapGpuDrawHandler(api, combinedMatrix, this->devClipBounds(),
                                             this->imageInfo());
        if (gpuDraw) {
            fSurfaceDrawContext->drawDrawable(
                    std::move(gpuDraw), combinedMatrix.mapRect(drawable->getBounds()));
            return;
        }
    }
    this->INHERITED::drawDrawable(drawable, matrix, canvas);
}


///////////////////////////////////////////////////////////////////////////////

bool Device::wait(int numSemaphores,
                  const GrBackendSemaphore* waitSemaphores,
                  bool deleteSemaphoresAfterWait) {
    ASSERT_SINGLE_OWNER

    return fSurfaceDrawContext->waitOnSemaphores(numSemaphores, waitSemaphores,
                                                 deleteSemaphoresAfterWait);
}

bool Device::replaceBackingProxy(SkSurface::ContentChangeMode mode,
                                 sk_sp<GrRenderTargetProxy> newRTP,
                                 GrColorType grColorType,
                                 sk_sp<SkColorSpace> colorSpace,
                                 GrSurfaceOrigin origin,
                                 const SkSurfaceProps& props) {
    auto sdc = SurfaceDrawContext::Make(fContext.get(), grColorType, std::move(newRTP),
                                        std::move(colorSpace), origin, props);
    if (!sdc) {
        return false;
    }

    SkASSERT(sdc->dimensions() == fSurfaceDrawContext->dimensions());
    SkASSERT(sdc->numSamples() == fSurfaceDrawContext->numSamples());
    SkASSERT(sdc->asSurfaceProxy()->priv().isExact());
    if (mode == SkSurface::kRetain_ContentChangeMode) {
        if (fContext->abandoned()) {
            return false;
        }

        SkASSERT(fSurfaceDrawContext->asTextureProxy());
        SkAssertResult(sdc->blitTexture(fSurfaceDrawContext->readSurfaceView(),
                                        SkIRect::MakeWH(this->width(), this->height()),
                                        SkIPoint::Make(0, 0)));
    }

    fSurfaceDrawContext = std::move(sdc);
    return true;
}

void Device::asyncRescaleAndReadPixels(const SkImageInfo& info,
                                       const SkIRect& srcRect,
                                       RescaleGamma rescaleGamma,
                                       RescaleMode rescaleMode,
                                       ReadPixelsCallback callback,
                                       ReadPixelsContext context) {
    auto* sdc = fSurfaceDrawContext.get();
    // Context TODO: Elevate direct context requirement to public API.
    auto dContext = sdc->recordingContext()->asDirectContext();
    if (!dContext) {
        return;
    }
    sdc->asyncRescaleAndReadPixels(dContext, info, srcRect, rescaleGamma, rescaleMode, callback,
                                   context);
}

void Device::asyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                             sk_sp<SkColorSpace> dstColorSpace,
                                             const SkIRect& srcRect,
                                             SkISize dstSize,
                                             RescaleGamma rescaleGamma,
                                             RescaleMode rescaleMode,
                                             ReadPixelsCallback callback,
                                             ReadPixelsContext context) {
    auto* sdc = fSurfaceDrawContext.get();
    // Context TODO: Elevate direct context requirement to public API.
    auto dContext = sdc->recordingContext()->asDirectContext();
    if (!dContext) {
        return;
    }
    sdc->asyncRescaleAndReadPixelsYUV420(dContext,
                                         yuvColorSpace,
                                         std::move(dstColorSpace),
                                         srcRect,
                                         dstSize,
                                         rescaleGamma,
                                         rescaleMode,
                                         callback,
                                         context);
}

///////////////////////////////////////////////////////////////////////////////

SkBaseDevice* Device::onCreateDevice(const CreateInfo& cinfo, const SkPaint*) {
    ASSERT_SINGLE_OWNER

    SkSurfaceProps props(this->surfaceProps().flags(), cinfo.fPixelGeometry);

    // layers are never drawn in repeat modes, so we can request an approx
    // match and ignore any padding.
    SkBackingFit fit = kNever_TileUsage == cinfo.fTileUsage ? SkBackingFit::kApprox
                                                            : SkBackingFit::kExact;

    SkASSERT(cinfo.fInfo.colorType() != kRGBA_1010102_SkColorType);

    auto sdc = SurfaceDrawContext::MakeWithFallback(
            fContext.get(), SkColorTypeToGrColorType(cinfo.fInfo.colorType()),
            fSurfaceDrawContext->colorInfo().refColorSpace(), fit, cinfo.fInfo.dimensions(), props,
            fSurfaceDrawContext->numSamples(), GrMipmapped::kNo,
            fSurfaceDrawContext->asSurfaceProxy()->isProtected(), kBottomLeft_GrSurfaceOrigin,
            SkBudgeted::kYes);
    if (!sdc) {
        return nullptr;
    }

    // Skia's convention is to only clear a device if it is non-opaque.
    InitContents init = cinfo.fInfo.isOpaque() ? InitContents::kUninit : InitContents::kClear;

    return Device::Make(std::move(sdc), cinfo.fInfo.alphaType(), init).release();
}

sk_sp<SkSurface> Device::makeSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    ASSERT_SINGLE_OWNER
    // TODO: Change the signature of newSurface to take a budgeted parameter.
    static const SkBudgeted kBudgeted = SkBudgeted::kNo;
    return SkSurface::MakeRenderTarget(fContext.get(), kBudgeted, info,
                                       fSurfaceDrawContext->numSamples(),
                                       fSurfaceDrawContext->origin(), &props);
}

SkImageFilterCache* Device::getImageFilterCache() {
    ASSERT_SINGLE_OWNER
    // We always return a transient cache, so it is freed after each
    // filter traversal.
    return SkImageFilterCache::Create(SkImageFilterCache::kDefaultTransientSize);
}

////////////////////////////////////////////////////////////////////////////////////

bool Device::android_utils_clipWithStencil() {
    SkRegion clipRegion;
    this->onAsRgnClip(&clipRegion);
    if (clipRegion.isEmpty()) {
        return false;
    }
    auto sdc = fSurfaceDrawContext.get();
    SkASSERT(sdc);
    GrPaint grPaint;
    grPaint.setXPFactory(GrDisableColorXPFactory::Get());
    static constexpr GrUserStencilSettings kDrawToStencil(
        GrUserStencilSettings::StaticInit<
            0x1,
            GrUserStencilTest::kAlways,
            0x1,
            GrUserStencilOp::kReplace,
            GrUserStencilOp::kReplace,
            0x1>()
    );
    // Regions don't actually need AA, but in DMSAA mode everything is antialiased.
    GrAA aa = GrAA(fSurfaceDrawContext->alwaysAntialias());
    sdc->drawRegion(nullptr, std::move(grPaint), aa, SkMatrix::I(), clipRegion,
                    GrStyle::SimpleFill(), &kDrawToStencil);
    return true;
}

} // namespace skgpu::v1
