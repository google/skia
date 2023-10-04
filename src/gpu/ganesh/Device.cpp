/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/Device.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkMesh.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkVertices.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SingleOwner.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "include/private/chromium/Slug.h"  // IWYU pragma: keep
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkDevice.h"
#include "src/core/SkDrawBase.h"
#include "src/core/SkImageFilterTypes.h"  // IWYU pragma: keep
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkLatticeIter.h"
#include "src/core/SkMeshPriv.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkVerticesPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/TiledTextureUtils.h"
#include "src/gpu/ganesh/ClipStack.h"
#include "src/gpu/ganesh/GrAuditTrail.h"
#include "src/gpu/ganesh/GrBlurUtils.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrFragmentProcessors.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/GrTracing.h"
#include "src/gpu/ganesh/GrUserStencilSettings.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/effects/GrDisableColorXP.h"
#include "src/gpu/ganesh/effects/GrRRectEffect.h"
#include "src/gpu/ganesh/geometry/GrShape.h"
#include "src/gpu/ganesh/geometry/GrStyledShape.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "src/gpu/ganesh/image/SkSpecialImage_Ganesh.h"
#include "src/text/GlyphRun.h"
#include "src/text/gpu/SlugImpl.h"
#include "src/text/gpu/SubRunContainer.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <tuple>
#include <utility>

class GrBackendSemaphore;
struct GrShaderCaps;
struct SkDrawShadowRec;

using namespace skia_private;

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(fContext->priv().singleOwner())

///////////////////////////////////////////////////////////////////////////////

namespace {

bool force_aa_clip(const skgpu::ganesh::SurfaceDrawContext* sdc) {
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
                         const SkMatrix& ctm,
                         sk_sp<SkBlender> blender,
                         bool hasColors,
                         const SkSurfaceProps& props,
                         GrPaint* grPaint) {
    if (hasColors) {
        return SkPaintToGrPaintWithBlend(rContext,
                                         colorInfo,
                                         skPaint,
                                         ctm,
                                         blender.get(),
                                         props,
                                         grPaint);
    } else {
        return SkPaintToGrPaint(rContext, colorInfo, skPaint, ctm, props, grPaint);
    }
}

bool init_mesh_child_effects(GrRecordingContext* rContext,
                             const GrColorInfo& colorInfo,
                             const SkSurfaceProps& surfaceProps,
                             const SkMesh& mesh,
                             TArray<std::unique_ptr<GrFragmentProcessor>>* meshChildFPs) {
    // We use `Scope::kRuntimeEffect` here to ensure that mesh shaders get the same "raw" sampling
    // behavior from alpha-only image shaders as a Runtime Effect would, rather than the unexpected
    // tinting-by-input-color.
    GrFPArgs fpArgs(rContext, &colorInfo, surfaceProps, GrFPArgs::Scope::kRuntimeEffect);

    for (const SkRuntimeEffect::ChildPtr& child : mesh.children()) {
        auto [success, childFP] = GrFragmentProcessors::MakeChildFP(child, fpArgs);
        if (!success) {
            return false;
        }
        meshChildFPs->push_back(std::move(childFP));
    }
    return true;
}

} // anonymous namespace

namespace skgpu::ganesh {

sk_sp<Device> Device::Make(GrRecordingContext* rContext,
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

SkImageInfo Device::MakeInfo(SurfaceContext* sc, DeviceFlags flags) {
    SkColorType colorType = GrColorTypeToSkColorType(sc->colorInfo().colorType());
    return SkImageInfo::Make(sc->width(), sc->height(), colorType,
                             flags & DeviceFlags::kIsOpaque ? kOpaque_SkAlphaType
                                                            : kPremul_SkAlphaType,
                             sc->colorInfo().refColorSpace());
}


/** Checks that the alpha type is legal and gets constructor flags. Returns false if device creation
    should fail. */
bool Device::CheckAlphaTypeAndGetFlags(SkAlphaType alphaType,
                                       InitContents init,
                                       DeviceFlags* flags) {
    *flags = DeviceFlags::kNone;
    switch (alphaType) {
        case kPremul_SkAlphaType:
            break;
        case kOpaque_SkAlphaType:
            *flags |= DeviceFlags::kIsOpaque;
            break;
        default: // If it is unpremul or unknown don't try to render
            return false;
    }
    if (InitContents::kClear == init) {
        *flags |= DeviceFlags::kNeedClear;
    }
    return true;
}

sk_sp<Device> Device::Make(std::unique_ptr<SurfaceDrawContext> sdc,
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

sk_sp<Device> Device::Make(GrRecordingContext* rContext,
                           skgpu::Budgeted budgeted,
                           const SkImageInfo& ii,
                           SkBackingFit fit,
                           int sampleCount,
                           skgpu::Mipmapped mipmapped,
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
                                        /*label=*/"MakeDevice",
                                        sampleCount,
                                        mipmapped,
                                        isProtected,
                                        origin,
                                        budgeted);

    return Device::Make(std::move(sdc), ii.alphaType(), init);
}

Device::Device(std::unique_ptr<SurfaceDrawContext> sdc, DeviceFlags flags)
        : SkDevice(MakeInfo(sdc.get(), flags), sdc->surfaceProps())
        , fContext(sk_ref_sp(sdc->recordingContext()))
        , fSDFTControl(sdc->recordingContext()->priv().getSDFTControl(
                       sdc->surfaceProps().isUseDeviceIndependentFonts()))
        , fSurfaceDrawContext(std::move(sdc))
        , fClip(SkIRect::MakeSize(fSurfaceDrawContext->dimensions()),
                &this->localToDevice(),
                force_aa_clip(fSurfaceDrawContext.get())) {
    if (flags & DeviceFlags::kNeedClear) {
        this->clearAll();
    }
}

Device::~Device() = default;

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

SurfaceFillContext* Device::surfaceFillContext() {
    ASSERT_SINGLE_OWNER
    return fSurfaceDrawContext.get();
}

void Device::clearAll() {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::ganesh::Device", "clearAll", fContext.get());

    SkIRect rect = SkIRect::MakeWH(this->width(), this->height());
    fSurfaceDrawContext->clearAtLeast(rect, SK_PMColor4fTRANSPARENT);
}

///////////////////////////////////////////////////////////////////////////////

void Device::clipPath(const SkPath& path, SkClipOp op, bool aa) {
#if defined(GR_TEST_UTILS)
    if (fContext->priv().options().fAllPathsVolatile && !path.isVolatile()) {
        this->clipPath(SkPath(path).setIsVolatile(true), op, aa);
        return;
    }
#endif
    SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
    fClip.clipPath(this->localToDevice(), path, GrAA(aa), op);
}

void Device::clipRegion(const SkRegion& globalRgn, SkClipOp op) {
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

void Device::android_utils_clipAsRgn(SkRegion* region) const {
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

bool Device::isClipAntiAliased() const {
    for (const ClipStack::Element& e : fClip) {
        if (e.fAA == GrAA::kYes) {
            return true;
        }
        SkASSERT(!fSurfaceDrawContext->alwaysAntialias());
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void Device::drawPaint(const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::ganesh::Device", "drawPaint", fContext.get());

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->recordingContext(),
                          fSurfaceDrawContext->colorInfo(),
                          paint,
                          this->localToDevice(),
                          fSurfaceDrawContext->surfaceProps(),
                          &grPaint)) {
        return;
    }

    fSurfaceDrawContext->drawPaint(this->clip(), std::move(grPaint), this->localToDevice());
}

void Device::drawPoints(SkCanvas::PointMode mode,
                        size_t count,
                        const SkPoint pts[],
                        const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::ganesh::Device", "drawPoints", fContext.get());
    SkScalar width = paint.getStrokeWidth();
    if (width < 0) {
        return;
    }

    GrAA aa = fSurfaceDrawContext->chooseAA(paint);

    if (count == 2 && mode == SkCanvas::kLines_PointMode) {
        if (paint.getPathEffect()) {
            // Probably a dashed line. Draw as a path.
            GrPaint grPaint;
            if (SkPaintToGrPaint(this->recordingContext(),
                                 fSurfaceDrawContext->colorInfo(),
                                 paint,
                                 this->localToDevice(),
                                 fSurfaceDrawContext->surfaceProps(),
                                 &grPaint)) {
                SkPath path;
                path.setIsVolatile(true);
                path.moveTo(pts[0]);
                path.lineTo(pts[1]);
                fSurfaceDrawContext->drawPath(this->clip(),
                                              std::move(grPaint),
                                              aa,
                                              this->localToDevice(),
                                              path,
                                              GrStyle(paint, SkPaint::kStroke_Style));
            }
            return;
        }
        if (!paint.getMaskFilter() &&
            paint.getStrokeWidth() > 0 &&  // drawStrokedLine doesn't support hairlines.
            paint.getStrokeCap() != SkPaint::kRound_Cap) { // drawStrokedLine doesn't do round caps.
            // Simple stroked line. Bypass path rendering.
            GrPaint grPaint;
            if (SkPaintToGrPaint(this->recordingContext(),
                                 fSurfaceDrawContext->colorInfo(),
                                 paint,
                                 this->localToDevice(),
                                 fSurfaceDrawContext->surfaceProps(),
                                 &grPaint)) {
                fSurfaceDrawContext->drawStrokedLine(this->clip(),
                                                     std::move(grPaint),
                                                     aa,
                                                     this->localToDevice(),
                                                     pts,
                                                     SkStrokeRec(paint, SkPaint::kStroke_Style));
            }
            return;
        }
    }

    const GrCaps* caps = fContext->priv().caps();
    SkScalar scales[2];
    bool isHairline =
            ((0 == width) ||
             (1 == width && this->localToDevice().getMinMaxScales(scales) &&
              SkScalarNearlyEqual(scales[0], 1.f) && SkScalarNearlyEqual(scales[1], 1.f))) &&

            // Don't do this as a hairline draw, which will emit line primitives, if
            // lines are not permitted by caps.
            !((mode == SkCanvas::kLines_PointMode || mode == SkCanvas::kPolygon_PointMode) &&
              caps->avoidLineDraws());

    // we only handle non-coverage-aa hairlines and paints without path effects or mask filters,
    // else we let the SkDraw call our drawPath()
    if (!isHairline ||
        paint.getPathEffect() ||
        paint.getMaskFilter() ||
        fSurfaceDrawContext->chooseAAType(aa) == GrAAType::kCoverage) {
        SkRasterClip rc(this->devClipBounds());
        SkDrawBase draw;
        // don't need to set fBlitterChoose, as it should never get used
        draw.fDst = SkPixmap(SkImageInfo::MakeUnknown(this->width(), this->height()), nullptr, 0);
        draw.fCTM = &this->localToDevice();
        draw.fRC = &rc;
        draw.drawDevicePoints(mode, count, pts, paint, this);
        return;
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->recordingContext(),
                          fSurfaceDrawContext->colorInfo(),
                          paint,
                          this->localToDevice(),
                          fSurfaceDrawContext->surfaceProps(),
                          &grPaint)) {
        return;
    }

    static constexpr SkVertices::VertexMode kIgnoredMode = SkVertices::kTriangles_VertexMode;
    sk_sp<SkVertices> vertices = SkVertices::MakeCopy(kIgnoredMode, SkToS32(count), pts, nullptr,
                                                      nullptr);

    GrPrimitiveType primitiveType = point_mode_to_primitive_type(mode);
    fSurfaceDrawContext->drawVertices(this->clip(), std::move(grPaint), this->localToDevice(),
                                      std::move(vertices), &primitiveType);
}

///////////////////////////////////////////////////////////////////////////////

void Device::drawRect(const SkRect& rect, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::ganesh::Device", "drawRect", fContext.get());

    GrStyle style(paint);

    // A couple reasons we might need to call drawPath.
    if (paint.getMaskFilter() || paint.getPathEffect()) {
        GrStyledShape shape(rect, style);

        GrBlurUtils::DrawShapeWithMaskFilter(fContext.get(), fSurfaceDrawContext.get(),
                                             this->clip(), paint, this->localToDevice(), shape);
        return;
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->recordingContext(),
                          fSurfaceDrawContext->colorInfo(),
                          paint,
                          this->localToDevice(),
                          fSurfaceDrawContext->surfaceProps(),
                          &grPaint)) {
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
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::ganesh::Device", "drawEdgeAAQuad", fContext.get());

    SkPMColor4f dstColor = SkColor4fPrepForDst(color, fSurfaceDrawContext->colorInfo()).premul();

    GrPaint grPaint;
    grPaint.setColor4f(dstColor);
    if (mode != SkBlendMode::kSrcOver) {
        grPaint.setXPFactory(GrXPFactory::FromBlendMode(mode));
    }

    if (clip) {
        // Use fillQuadWithEdgeAA
        fSurfaceDrawContext->fillQuadWithEdgeAA(this->clip(),
                                                std::move(grPaint),
                                                SkToGrQuadAAFlags(aaFlags),
                                                this->localToDevice(),
                                                clip,
                                                nullptr);
    } else {
        // Use fillRectWithEdgeAA to preserve mathematical properties of dst being rectangular
        fSurfaceDrawContext->fillRectWithEdgeAA(this->clip(),
                                                std::move(grPaint),
                                                SkToGrQuadAAFlags(aaFlags),
                                                this->localToDevice(),
                                                rect);
    }
}

///////////////////////////////////////////////////////////////////////////////

void Device::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::ganesh::Device", "drawRRect", fContext.get());

    auto mf = paint.getMaskFilter();
    if (mf) {
        if (GrFragmentProcessors::IsSupported(mf)) {
            mf = nullptr; // already handled in SkPaintToGrPaint
        }
    }

    GrStyle style(paint);

    if (mf || style.pathEffect()) {
        // A path effect will presumably transform this rrect into something else.
        GrStyledShape shape(rrect, style);

        GrBlurUtils::DrawShapeWithMaskFilter(fContext.get(), fSurfaceDrawContext.get(),
                                             this->clip(), paint, this->localToDevice(), shape);
        return;
    }

    SkASSERT(!style.pathEffect());

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->recordingContext(),
                          fSurfaceDrawContext->colorInfo(),
                          paint,
                          this->localToDevice(),
                          fSurfaceDrawContext->surfaceProps(),
                          &grPaint)) {
        return;
    }

    fSurfaceDrawContext->drawRRect(this->clip(), std::move(grPaint),
                                   fSurfaceDrawContext->chooseAA(paint), this->localToDevice(),
                                   rrect, style);
}

void Device::drawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::ganesh::Device", "drawDRRect", fContext.get());
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
            if (!SkPaintToGrPaint(this->recordingContext(),
                                  fSurfaceDrawContext->colorInfo(),
                                  paint,
                                  this->localToDevice(),
                                  fSurfaceDrawContext->surfaceProps(),
                                  &grPaint)) {
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

    GrBlurUtils::DrawShapeWithMaskFilter(fContext.get(), fSurfaceDrawContext.get(), this->clip(),
                                         paint, this->localToDevice(), shape);
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
    if (!SkPaintToGrPaint(this->recordingContext(),
                          fSurfaceDrawContext->colorInfo(),
                          paint,
                          this->localToDevice(),
                          fSurfaceDrawContext->surfaceProps(),
                          &grPaint)) {
        return;
    }

    fSurfaceDrawContext->drawRegion(this->clip(), std::move(grPaint),
                                    fSurfaceDrawContext->chooseAA(paint), this->localToDevice(),
                                    region, GrStyle(paint));
}

void Device::drawOval(const SkRect& oval, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::ganesh::Device", "drawOval", fContext.get());

    if (paint.getMaskFilter()) {
        // The RRect path can handle special case blurring
        SkRRect rr = SkRRect::MakeOval(oval);
        return this->drawRRect(rr, paint);
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->recordingContext(),
                          fSurfaceDrawContext->colorInfo(),
                          paint,
                          this->localToDevice(),
                          fSurfaceDrawContext->surfaceProps(),
                          &grPaint)) {
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
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::ganesh::Device", "drawArc", fContext.get());
    if (paint.getMaskFilter()) {
        this->SkDevice::drawArc(oval, startAngle, sweepAngle, useCenter, paint);
        return;
    }
    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->recordingContext(),
                          fSurfaceDrawContext->colorInfo(),
                          paint,
                          this->localToDevice(),
                          fSurfaceDrawContext->surfaceProps(),
                          &grPaint)) {
        return;
    }

    fSurfaceDrawContext->drawArc(this->clip(), std::move(grPaint),
                                 fSurfaceDrawContext->chooseAA(paint), this->localToDevice(), oval,
                                 startAngle, sweepAngle, useCenter, GrStyle(paint));
}

///////////////////////////////////////////////////////////////////////////////

void Device::drawPath(const SkPath& origSrcPath, const SkPaint& paint, bool pathIsMutable) {
#if defined(GR_TEST_UTILS)
    if (fContext->priv().options().fAllPathsVolatile && !origSrcPath.isVolatile()) {
        this->drawPath(SkPath(origSrcPath).setIsVolatile(true), paint, true);
        return;
    }
#endif
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::ganesh::Device", "drawPath", fContext.get());
    if (!paint.getMaskFilter()) {
        GrPaint grPaint;
        if (!SkPaintToGrPaint(this->recordingContext(),
                              fSurfaceDrawContext->colorInfo(),
                              paint,
                              this->localToDevice(),
                              fSurfaceDrawContext->surfaceProps(),
                              &grPaint)) {
            return;
        }
        fSurfaceDrawContext->drawPath(this->clip(), std::move(grPaint),
                                      fSurfaceDrawContext->chooseAA(paint), this->localToDevice(),
                                      origSrcPath, GrStyle(paint));
        return;
    }

    // TODO: losing possible mutability of 'origSrcPath' here
    GrStyledShape shape(origSrcPath, paint);

    GrBlurUtils::DrawShapeWithMaskFilter(fContext.get(), fSurfaceDrawContext.get(), this->clip(),
                                         paint, this->localToDevice(), shape);
}

sk_sp<skif::Backend> Device::createImageFilteringBackend(const SkSurfaceProps& surfaceProps,
                                                         SkColorType colorType) const {
    return skif::MakeGaneshBackend(
            fContext, fSurfaceDrawContext->origin(), surfaceProps, colorType);
}

sk_sp<SkSpecialImage> Device::makeSpecial(const SkBitmap& bitmap) {
    ASSERT_SINGLE_OWNER

    // TODO: this makes a tight copy of 'bitmap' but it doesn't have to be (given SkSpecialImage's
    // semantics). Since this is cached we would have to bake the fit into the cache key though.
    auto view = std::get<0>(
            GrMakeCachedBitmapProxyView(fContext.get(), bitmap, /*label=*/"Device_MakeSpecial"));
    if (!view) {
        return nullptr;
    }

    const SkIRect rect = SkIRect::MakeSize(view.proxy()->dimensions());

    // GrMakeCachedBitmapProxyView creates a tight copy of 'bitmap' so we don't have to subset
    // the special image
    return SkSpecialImages::MakeDeferredFromGpu(fContext.get(),
                                                rect,
                                                bitmap.getGenerationID(),
                                                std::move(view),
                                                {SkColorTypeToGrColorType(bitmap.colorType()),
                                                 kPremul_SkAlphaType,
                                                 bitmap.refColorSpace()},
                                                this->surfaceProps());
}

sk_sp<SkSpecialImage> Device::makeSpecial(const SkImage* image) {
    ASSERT_SINGLE_OWNER

    SkPixmap pm;
    if (image->isTextureBacked()) {
        auto [view, ct] =
                skgpu::ganesh::AsView(this->recordingContext(), image, skgpu::Mipmapped::kNo);
        SkASSERT(view);

        return SkSpecialImages::MakeDeferredFromGpu(
                fContext.get(),
                SkIRect::MakeWH(image->width(), image->height()),
                image->uniqueID(),
                std::move(view),
                {ct, kPremul_SkAlphaType, image->refColorSpace()},
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
                                        skgpu::Mipmapped::kNo,  // Don't auto generate mips
                                        subset,
                                        SkBackingFit::kApprox,
                                        skgpu::Budgeted::kYes,
                                        /*label=*/"Device_SnapSpecial");  // Always budgeted
        if (!view) {
            return nullptr;
        }
        // Since this copied only the requested subset, the special image wrapping the proxy no
        // longer needs the original subset.
        finalSubset = SkIRect::MakeSize(view.dimensions());
    }

    return SkSpecialImages::MakeDeferredFromGpu(fContext.get(),
                                                finalSubset,
                                                kNeedNewImageUniqueID_SpecialImage,
                                                std::move(view),
                                                GrColorInfo(this->imageInfo().colorInfo()),
                                                this->surfaceProps());
}

sk_sp<SkSpecialImage> Device::snapSpecialScaled(const SkIRect& subset, const SkISize& dstDims) {
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

    auto scaledContext = sdc->rescale(sdc->imageInfo().makeDimensions(dstDims),
                                      sdc->origin(),
                                      subset,
                                      RescaleGamma::kSrc,
                                      RescaleMode::kLinear);
    if (!scaledContext) {
        return nullptr;
    }

    return SkSpecialImages::MakeDeferredFromGpu(fContext.get(),
                                                SkIRect::MakeSize(dstDims),
                                                kNeedNewImageUniqueID_SpecialImage,
                                                scaledContext->readSurfaceView(),
                                                GrColorInfo(this->imageInfo().colorInfo()),
                                                this->surfaceProps());
}

void Device::drawDevice(SkDevice* device,
                        const SkSamplingOptions& sampling,
                        const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    // clear of the source device must occur before CHECK_SHOULD_DRAW
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::ganesh::Device", "drawDevice", fContext.get());
    this->SkDevice::drawDevice(device, sampling, paint);
}

void Device::drawImageRect(const SkImage* image,
                           const SkRect* src,
                           const SkRect& dst,
                           const SkSamplingOptions& sampling,
                           const SkPaint& paint,
                           SkCanvas::SrcRectConstraint constraint) {
    ASSERT_SINGLE_OWNER

    GrAA aa = fSurfaceDrawContext->chooseAA(paint);
    SkCanvas::QuadAAFlags aaFlags = (aa == GrAA::kYes) ? SkCanvas::kAll_QuadAAFlags
                                                       : SkCanvas::kNone_QuadAAFlags;

    this->drawImageQuadDirect(image,
                              src ? *src
                                  : SkRect::MakeIWH(image->width(), image->height()),
                              dst,
                              /* dstClip= */ nullptr,
                              aaFlags,
                              /* preViewMatrix= */ nullptr,
                              sampling,
                              paint,
                              constraint);
}

bool Device::drawAsTiledImageRect(SkCanvas* canvas,
                                  const SkImage* image,
                                  const SkRect* src,
                                  const SkRect& dst,
                                  const SkSamplingOptions& sampling,
                                  const SkPaint& paint,
                                  SkCanvas::SrcRectConstraint constraint) {
    ASSERT_SINGLE_OWNER

    GrAA aa = fSurfaceDrawContext->chooseAA(paint);
    SkCanvas::QuadAAFlags aaFlags = (aa == GrAA::kYes) ? SkCanvas::kAll_QuadAAFlags
                                                       : SkCanvas::kNone_QuadAAFlags;

    return TiledTextureUtils::DrawAsTiledImageRect(canvas,
                                                   image,
                                                   src ? *src
                                                       : SkRect::MakeIWH(image->width(),
                                                                         image->height()),
                                                   dst, aaFlags, sampling, &paint, constraint);
}


void Device::drawViewLattice(GrSurfaceProxyView view,
                             const GrColorInfo& info,
                             std::unique_ptr<SkLatticeIter> iter,
                             const SkRect& dst,
                             SkFilterMode filter,
                             const SkPaint& origPaint) {
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::ganesh::Device", "drawViewLattice", fContext.get());
    SkASSERT(view);

    SkTCopyOnFirstWrite<SkPaint> paint(&origPaint);

    if (!info.isAlphaOnly() && (paint->getColor() & 0x00FFFFFF) != 0x00FFFFFF) {
        paint.writable()->setColor(SkColorSetARGB(origPaint.getAlpha(), 0xFF, 0xFF, 0xFF));
    }
    GrPaint grPaint;
    // Passing null as shaderFP indicates that the GP will provide the shader.
    if (!SkPaintToGrPaintReplaceShader(this->recordingContext(),
                                       fSurfaceDrawContext->colorInfo(),
                                       *paint,
                                       this->localToDevice(),
                                       /*shaderFP=*/nullptr,
                                       fSurfaceDrawContext->surfaceProps(),
                                       &grPaint)) {
        return;
    }

    if (info.isAlphaOnly()) {
        // If we were doing this with an FP graph we'd use a kDstIn blend between the texture and
        // the paint color.
        view.concatSwizzle(skgpu::Swizzle("aaaa"));
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

    auto [view, ct] = skgpu::ganesh::AsView(this->recordingContext(), image, skgpu::Mipmapped::kNo);
    if (view) {
        GrColorInfo colorInfo(ct, image->alphaType(), image->refColorSpace());
        this->drawViewLattice(std::move(view),
                              std::move(colorInfo),
                              std::move(iter),
                              dst,
                              filter,
                              paint);
    }
}

void Device::drawVertices(const SkVertices* vertices,
                          sk_sp<SkBlender> blender,
                          const SkPaint& paint,
                          bool skipColorXform) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::ganesh::Device", "drawVertices", fContext.get());
    SkASSERT(vertices);

#ifdef SK_LEGACY_IGNORE_DRAW_VERTICES_BLEND_WITH_NO_SHADER
    if (!paint.getShader()) {
        blender = SkBlender::Mode(SkBlendMode::kDst);
    }
#endif

    SkVerticesPriv info(vertices->priv());

    GrPaint grPaint;
    if (!init_vertices_paint(fContext.get(),
                             fSurfaceDrawContext->colorInfo(),
                             paint,
                             this->localToDevice(),
                             std::move(blender),
                             info.hasColors(),
                             fSurfaceDrawContext->surfaceProps(),
                             &grPaint)) {
        return;
    }
    fSurfaceDrawContext->drawVertices(this->clip(),
                                      std::move(grPaint),
                                      this->localToDevice(),
                                      sk_ref_sp(const_cast<SkVertices*>(vertices)),
                                      nullptr,
                                      skipColorXform);
}

void Device::drawMesh(const SkMesh& mesh, sk_sp<SkBlender> blender, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::ganesh::Device", "drawMesh", fContext.get());
    if (!mesh.isValid()) {
        return;
    }

    GrPaint grPaint;
    if (!init_vertices_paint(fContext.get(),
                             fSurfaceDrawContext->colorInfo(),
                             paint,
                             this->localToDevice(),
                             std::move(blender),
                             SkMeshSpecificationPriv::HasColors(*mesh.spec()),
                             fSurfaceDrawContext->surfaceProps(),
                             &grPaint)) {
        return;
    }

    TArray<std::unique_ptr<GrFragmentProcessor>> meshChildFPs;
    if (!init_mesh_child_effects(fContext.get(),
                                 fSurfaceDrawContext->colorInfo(),
                                 fSurfaceDrawContext->surfaceProps(),
                                 mesh,
                                 &meshChildFPs)) {
        return;
    }
    fSurfaceDrawContext->drawMesh(this->clip(), std::move(grPaint), this->localToDevice(), mesh,
                                  std::move(meshChildFPs));
}

///////////////////////////////////////////////////////////////////////////////

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
void Device::drawShadow(const SkPath& path, const SkDrawShadowRec& rec) {
#if defined(GR_TEST_UTILS)
    if (fContext->priv().options().fAllPathsVolatile && !path.isVolatile()) {
        this->drawShadow(SkPath(path).setIsVolatile(true), rec);
        return;
    }
#endif
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::ganesh::Device", "drawShadow", fContext.get());

    if (!fSurfaceDrawContext->drawFastShadow(this->clip(), this->localToDevice(), path, rec)) {
        // failed to find an accelerated case
        this->SkDevice::drawShadow(path, rec);
    }
}
#endif  // SK_ENABLE_OPTIMIZE_SIZE

///////////////////////////////////////////////////////////////////////////////

void Device::drawAtlas(const SkRSXform xform[],
                       const SkRect texRect[],
                       const SkColor colors[],
                       int count,
                       sk_sp<SkBlender> blender,
                       const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::ganesh::Device", "drawAtlas", fContext.get());

    GrPaint grPaint;
    if (colors) {
        if (!SkPaintToGrPaintWithBlend(this->recordingContext(),
                                       fSurfaceDrawContext->colorInfo(),
                                       paint,
                                       this->localToDevice(),
                                       blender.get(),
                                       fSurfaceDrawContext->surfaceProps(),
                                       &grPaint)) {
            return;
        }
    } else {
        if (!SkPaintToGrPaint(this->recordingContext(),
                              fSurfaceDrawContext->colorInfo(),
                              paint,
                              this->localToDevice(),
                              fSurfaceDrawContext->surfaceProps(),
                              &grPaint)) {
            return;
        }
    }

    fSurfaceDrawContext->drawAtlas(this->clip(), std::move(grPaint), this->localToDevice(), count,
                                   xform, texRect, colors);
}

///////////////////////////////////////////////////////////////////////////////

void Device::onDrawGlyphRunList(SkCanvas* canvas,
                                const sktext::GlyphRunList& glyphRunList,
                                const SkPaint& initialPaint,
                                const SkPaint& drawingPaint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("skgpu::ganesh::Device", "drawGlyphRunList", fContext.get());
    SkASSERT(!glyphRunList.hasRSXForm());

    if (glyphRunList.blob() == nullptr) {
        // If the glyphRunList does not have an associated text blob, then it was created by one of
        // the direct draw APIs (drawGlyphs, etc.). Use a Slug to draw the glyphs.
        auto slug = this->convertGlyphRunListToSlug(glyphRunList, initialPaint, drawingPaint);
        if (slug != nullptr) {
            this->drawSlug(canvas, slug.get(), drawingPaint);
        }
    } else {
        fSurfaceDrawContext->drawGlyphRunList(canvas,
                                              this->clip(),
                                              this->localToDevice(),
                                              glyphRunList,
                                              this->strikeDeviceInfo(),
                                              drawingPaint);
    }
}

///////////////////////////////////////////////////////////////////////////////

void Device::drawDrawable(SkCanvas* canvas, SkDrawable* drawable, const SkMatrix* matrix) {
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
    this->SkDevice::drawDrawable(canvas, drawable, matrix);
}


///////////////////////////////////////////////////////////////////////////////

GrSurfaceProxyView Device::readSurfaceView() {
    return this->surfaceFillContext()->readSurfaceView();
}

GrRenderTargetProxy* Device::targetProxy() {
    return this->readSurfaceView().asRenderTargetProxy();
}

bool Device::wait(int numSemaphores,
                  const GrBackendSemaphore* waitSemaphores,
                  bool deleteSemaphoresAfterWait) {
    ASSERT_SINGLE_OWNER

    return fSurfaceDrawContext->waitOnSemaphores(numSemaphores, waitSemaphores,
                                                 deleteSemaphoresAfterWait);
}

void Device::discard() {
    fSurfaceDrawContext->discard();
}

void Device::resolveMSAA() {
    fSurfaceDrawContext->resolveMSAA();
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

bool Device::replaceBackingProxy(SkSurface::ContentChangeMode mode) {
    ASSERT_SINGLE_OWNER

    const SkImageInfo& ii = this->imageInfo();
    GrRenderTargetProxy* oldRTP = this->targetProxy();
    GrSurfaceProxyView oldView = this->readSurfaceView();

    auto grColorType = SkColorTypeToGrColorType(ii.colorType());
    auto format = fContext->priv().caps()->getDefaultBackendFormat(grColorType, GrRenderable::kYes);
    if (!format.isValid()) {
        return false;
    }

    GrProxyProvider* proxyProvider = fContext->priv().proxyProvider();
    // This entry point is used by SkSurface_Ganesh::onCopyOnWrite so it must create a
    // kExact-backed render target proxy
    sk_sp<GrTextureProxy> proxy =
            proxyProvider->createProxy(format,
                                       ii.dimensions(),
                                       GrRenderable::kYes,
                                       oldRTP->numSamples(),
                                       oldView.mipmapped(),
                                       SkBackingFit::kExact,
                                       oldRTP->isBudgeted(),
                                       GrProtected::kNo,
                                       /*label=*/"BaseDevice_ReplaceBackingProxy");
    if (!proxy) {
        return false;
    }

    return this->replaceBackingProxy(mode, sk_ref_sp(proxy->asRenderTargetProxy()),
                                     grColorType, ii.refColorSpace(), oldView.origin(),
                                     this->surfaceProps());
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
                                             bool readAlpha,
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
                                         readAlpha,
                                         std::move(dstColorSpace),
                                         srcRect,
                                         dstSize,
                                         rescaleGamma,
                                         rescaleMode,
                                         callback,
                                         context);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkDevice> Device::createDevice(const CreateInfo& cinfo, const SkPaint*) {
    ASSERT_SINGLE_OWNER

    SkSurfaceProps props(this->surfaceProps().flags(), cinfo.fPixelGeometry);

    SkASSERT(cinfo.fInfo.colorType() != kRGBA_1010102_SkColorType);

    auto sdc = SurfaceDrawContext::MakeWithFallback(
            fContext.get(),
            SkColorTypeToGrColorType(cinfo.fInfo.colorType()),
            fSurfaceDrawContext->colorInfo().refColorSpace(),
            SkBackingFit::kApprox,
            cinfo.fInfo.dimensions(),
            props,
            fSurfaceDrawContext->numSamples(),
            skgpu::Mipmapped::kNo,
            fSurfaceDrawContext->asSurfaceProxy()->isProtected(),
            fSurfaceDrawContext->origin(),
            skgpu::Budgeted::kYes);
    if (!sdc) {
        return nullptr;
    }

    // Skia's convention is to only clear a device if it is non-opaque.
    InitContents init = cinfo.fInfo.isOpaque() ? InitContents::kUninit : InitContents::kClear;

    return Device::Make(std::move(sdc), cinfo.fInfo.alphaType(), init);
}

sk_sp<SkSurface> Device::makeSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    ASSERT_SINGLE_OWNER
    // TODO: Change the signature of newSurface to take a budgeted parameter.
    static const skgpu::Budgeted kBudgeted = skgpu::Budgeted::kNo;
    bool isProtected = this->targetProxy()->isProtected() == GrProtected::kYes;
    return SkSurfaces::RenderTarget(fContext.get(),
                                    kBudgeted,
                                    info,
                                    fSurfaceDrawContext->numSamples(),
                                    fSurfaceDrawContext->origin(),
                                    &props,
                                    /* shouldCreateWithMips= */ false,
                                    isProtected);
}

////////////////////////////////////////////////////////////////////////////////////

bool Device::android_utils_clipWithStencil() {
    SkRegion clipRegion;
    this->android_utils_clipAsRgn(&clipRegion);
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

SkStrikeDeviceInfo Device::strikeDeviceInfo() const {
    return {this->surfaceProps(), this->scalerContextFlags(), &fSDFTControl};
}

sk_sp<sktext::gpu::Slug>
Device::convertGlyphRunListToSlug(const sktext::GlyphRunList& glyphRunList,
                                  const SkPaint& initialPaint,
                                  const SkPaint& drawingPaint) {
    return sktext::gpu::SlugImpl::Make(this->localToDevice(),
                                       glyphRunList,
                                       initialPaint,
                                       drawingPaint,
                                       this->strikeDeviceInfo(),
                                       SkStrikeCache::GlobalStrikeCache());
}

void Device::drawSlug(SkCanvas* canvas, const sktext::gpu::Slug* slug,
                      const SkPaint& drawingPaint) {
    SkASSERT(canvas);
    SkASSERT(slug);
    const sktext::gpu::SlugImpl* slugImpl = static_cast<const sktext::gpu::SlugImpl*>(slug);
#if defined(SK_DEBUG)
    if (!fContext->priv().options().fSupportBilerpFromGlyphAtlas) {
        // We can draw a slug if the atlas has padding or if the creation matrix and the
        // drawing matrix are the same. If they are the same, then the Slug will use the direct
        // drawing code and not use bi-lerp.
        SkMatrix slugMatrix = slugImpl->initialPositionMatrix();
        SkMatrix positionMatrix = this->localToDevice();
        positionMatrix.preTranslate(slugImpl->origin().x(), slugImpl->origin().y());
        SkASSERT(slugMatrix == positionMatrix);
    }
#endif
    auto atlasDelegate = [&](const sktext::gpu::AtlasSubRun* subRun,
                             SkPoint drawOrigin,
                             const SkPaint& paint,
                             sk_sp<SkRefCnt> subRunStorage,
                             sktext::gpu::RendererData) {
        auto[drawingClip, op] = subRun->makeAtlasTextOp(
                this->clip(), this->localToDevice(), drawOrigin, paint,
                std::move(subRunStorage), fSurfaceDrawContext.get());
        if (op != nullptr) {
            fSurfaceDrawContext->addDrawOp(drawingClip, std::move(op));
        }
    };

    slugImpl->subRuns()->draw(canvas, slugImpl->origin(),
                              drawingPaint, slugImpl, atlasDelegate);
}

}  // namespace skgpu::ganesh
