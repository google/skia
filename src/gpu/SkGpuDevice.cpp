/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/SkGpuDevice.h"

#include "include/core/SkImageFilter.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPicture.h"
#include "include/core/SkSurface.h"
#include "include/core/SkVertices.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GrContext.h"
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
#include "src/core/SkSpecialImage.h"
#include "src/core/SkStroke.h"
#include "src/core/SkTLazy.h"
#include "src/core/SkVertState.h"
#include "src/core/SkVerticesPriv.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrBlurUtils.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrImageTextureMaker.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTextureAdjuster.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/text/GrTextTarget.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkReadPixelsRec.h"
#include "src/image/SkSurface_Gpu.h"
#include "src/utils/SkUTF.h"

#define ASSERT_SINGLE_OWNER \
SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fContext->priv().singleOwner());)


///////////////////////////////////////////////////////////////////////////////

/** Checks that the alpha type is legal and gets constructor flags. Returns false if device creation
    should fail. */
bool SkGpuDevice::CheckAlphaTypeAndGetFlags(
                        const SkImageInfo* info, SkGpuDevice::InitContents init, unsigned* flags) {
    *flags = 0;
    if (info) {
        switch (info->alphaType()) {
            case kPremul_SkAlphaType:
                break;
            case kOpaque_SkAlphaType:
                *flags |= SkGpuDevice::kIsOpaque_Flag;
                break;
            default: // If it is unpremul or unknown don't try to render
                return false;
        }
    }
    if (kClear_InitContents == init) {
        *flags |= kNeedClear_Flag;
    }
    return true;
}

sk_sp<SkGpuDevice> SkGpuDevice::Make(GrContext* context,
                                     std::unique_ptr<GrRenderTargetContext> renderTargetContext,
                                     InitContents init) {
    if (!renderTargetContext || context->priv().abandoned()) {
        return nullptr;
    }

    SkColorType ct = GrColorTypeToSkColorType(renderTargetContext->colorInfo().colorType());

    unsigned flags;
    if (!context->colorTypeSupportedAsSurface(ct) ||
        !CheckAlphaTypeAndGetFlags(nullptr, init, &flags)) {
        return nullptr;
    }
    return sk_sp<SkGpuDevice>(new SkGpuDevice(context, std::move(renderTargetContext), flags));
}

sk_sp<SkGpuDevice> SkGpuDevice::Make(GrContext* context, SkBudgeted budgeted,
                                     const SkImageInfo& info, int sampleCount,
                                     GrSurfaceOrigin origin, const SkSurfaceProps* props,
                                     GrMipMapped mipMapped, InitContents init) {
    unsigned flags;
    if (!context->colorTypeSupportedAsSurface(info.colorType()) ||
        !CheckAlphaTypeAndGetFlags(&info, init, &flags)) {
        return nullptr;
    }

    auto renderTargetContext =
            MakeRenderTargetContext(context, budgeted, info, sampleCount, origin, props, mipMapped);
    if (!renderTargetContext) {
        return nullptr;
    }

    return sk_sp<SkGpuDevice>(new SkGpuDevice(context, std::move(renderTargetContext), flags));
}

static SkImageInfo make_info(GrRenderTargetContext* context, bool opaque) {
    SkColorType colorType = GrColorTypeToSkColorType(context->colorInfo().colorType());
    return SkImageInfo::Make(context->width(), context->height(), colorType,
                             opaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType,
                             context->colorInfo().refColorSpace());
}

SkGpuDevice::SkGpuDevice(GrContext* context,
                         std::unique_ptr<GrRenderTargetContext> renderTargetContext,
                         unsigned flags)
        : INHERITED(make_info(renderTargetContext.get(), SkToBool(flags & kIsOpaque_Flag)),
                    renderTargetContext->surfaceProps())
        , fContext(SkRef(context))
        , fRenderTargetContext(std::move(renderTargetContext)) {
    if (flags & kNeedClear_Flag) {
        this->clearAll();
    }
}

std::unique_ptr<GrRenderTargetContext> SkGpuDevice::MakeRenderTargetContext(
        GrContext* context,
        SkBudgeted budgeted,
        const SkImageInfo& origInfo,
        int sampleCount,
        GrSurfaceOrigin origin,
        const SkSurfaceProps* surfaceProps,
        GrMipMapped mipMapped) {
    if (!context) {
        return nullptr;
    }

    // This method is used to create SkGpuDevice's for SkSurface_Gpus. In this case
    // they need to be exact.
    return GrRenderTargetContext::Make(
            context, SkColorTypeToGrColorType(origInfo.colorType()), origInfo.refColorSpace(),
            SkBackingFit::kExact, origInfo.dimensions(), sampleCount, mipMapped, GrProtected::kNo,
            origin, budgeted, surfaceProps);
}

sk_sp<SkSpecialImage> SkGpuDevice::filterTexture(SkSpecialImage* srcImg,
                                                 int left, int top,
                                                 SkIPoint* offset,
                                                 const SkImageFilter* filter) {
    SkASSERT(srcImg->isTextureBacked());
    SkASSERT(filter);

    SkMatrix matrix = this->localToDevice();
    matrix.postTranslate(SkIntToScalar(-left), SkIntToScalar(-top));
    const SkIRect clipBounds = this->devClipBounds().makeOffset(-left, -top);
    sk_sp<SkImageFilterCache> cache(this->getImageFilterCache());
    SkColorType colorType = GrColorTypeToSkColorType(fRenderTargetContext->colorInfo().colorType());
    if (colorType == kUnknown_SkColorType) {
        colorType = kRGBA_8888_SkColorType;
    }
    SkImageFilter_Base::Context ctx(matrix, clipBounds, cache.get(), colorType,
                                    fRenderTargetContext->colorInfo().colorSpace(), srcImg);

    return as_IFB(filter)->filterImage(ctx).imageAndOffset(offset);
}

///////////////////////////////////////////////////////////////////////////////

bool SkGpuDevice::onReadPixels(const SkPixmap& pm, int x, int y) {
    ASSERT_SINGLE_OWNER

    if (!SkImageInfoValidConversion(pm.info(), this->imageInfo())) {
        return false;
    }

    return fRenderTargetContext->readPixels(pm.info(), pm.writable_addr(), pm.rowBytes(), {x, y});
}

bool SkGpuDevice::onWritePixels(const SkPixmap& pm, int x, int y) {
    ASSERT_SINGLE_OWNER

    if (!SkImageInfoValidConversion(this->imageInfo(), pm.info())) {
        return false;
    }

    return fRenderTargetContext->writePixels(pm.info(), pm.addr(), pm.rowBytes(), {x, y});
}

bool SkGpuDevice::onAccessPixels(SkPixmap* pmap) {
    ASSERT_SINGLE_OWNER
    return false;
}

GrRenderTargetContext* SkGpuDevice::accessRenderTargetContext() {
    ASSERT_SINGLE_OWNER
    return fRenderTargetContext.get();
}

void SkGpuDevice::clearAll() {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "clearAll", fContext.get());

    SkIRect rect = SkIRect::MakeWH(this->width(), this->height());
    fRenderTargetContext->clear(&rect, SK_PMColor4fTRANSPARENT,
                                GrRenderTargetContext::CanClearFullscreen::kYes);
}

void SkGpuDevice::replaceRenderTargetContext(std::unique_ptr<GrRenderTargetContext> rtc,
                                             SkSurface::ContentChangeMode mode) {
    SkASSERT(rtc->width() == this->width());
    SkASSERT(rtc->height() == this->height());
    SkASSERT(rtc->numSamples() == fRenderTargetContext->numSamples());
    SkASSERT(rtc->asSurfaceProxy()->priv().isExact());
    if (mode == SkSurface::kRetain_ContentChangeMode) {
        if (this->context()->abandoned()) {
            return;
        }

        SkASSERT(fRenderTargetContext->asTextureProxy());
        SkAssertResult(rtc->blitTexture(fRenderTargetContext->readSurfaceView(),
                                        SkIRect::MakeWH(this->width(), this->height()),
                                        SkIPoint::Make(0,0)));
    }

    fRenderTargetContext = std::move(rtc);
}

void SkGpuDevice::replaceRenderTargetContext(SkSurface::ContentChangeMode mode) {
    ASSERT_SINGLE_OWNER

    SkBudgeted budgeted = fRenderTargetContext->priv().isBudgeted();

    // This entry point is used by SkSurface_Gpu::onCopyOnWrite so it must create a
    // kExact-backed render target context.
    auto newRTC = MakeRenderTargetContext(this->context(),
                                          budgeted,
                                          this->imageInfo(),
                                          fRenderTargetContext->numSamples(),
                                          fRenderTargetContext->origin(),
                                          &this->surfaceProps(),
                                          fRenderTargetContext->mipMapped());
    if (!newRTC) {
        return;
    }
    this->replaceRenderTargetContext(std::move(newRTC), mode);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawPaint(const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawPaint", fContext.get());

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), fRenderTargetContext->colorInfo(), paint,
                          this->localToDevice(), &grPaint)) {
        return;
    }

    fRenderTargetContext->drawPaint(this->clip(), std::move(grPaint), this->localToDevice());
}

static inline GrPrimitiveType point_mode_to_primitive_type(SkCanvas::PointMode mode) {
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

void SkGpuDevice::drawPoints(SkCanvas::PointMode mode,
                             size_t count, const SkPoint pts[], const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawPoints", fContext.get());
    SkScalar width = paint.getStrokeWidth();
    if (width < 0) {
        return;
    }

    if (paint.getPathEffect() && 2 == count && SkCanvas::kLines_PointMode == mode) {
        GrStyle style(paint, SkPaint::kStroke_Style);
        GrPaint grPaint;
        if (!SkPaintToGrPaint(this->context(), fRenderTargetContext->colorInfo(), paint,
                              this->localToDevice(), &grPaint)) {
            return;
        }
        SkPath path;
        path.setIsVolatile(true);
        path.moveTo(pts[0]);
        path.lineTo(pts[1]);
        fRenderTargetContext->drawPath(this->clip(), std::move(grPaint), GrAA(paint.isAntiAlias()),
                                       this->localToDevice(), path, style);
        return;
    }

    SkScalar scales[2];
    bool isHairline = (0 == width) ||
                       (1 == width && this->localToDevice().getMinMaxScales(scales) &&
                        SkScalarNearlyEqual(scales[0], 1.f) && SkScalarNearlyEqual(scales[1], 1.f));
    // we only handle non-antialiased hairlines and paints without path effects or mask filters,
    // else we let the SkDraw call our drawPath()
    if (!isHairline || paint.getPathEffect() || paint.getMaskFilter() || paint.isAntiAlias()) {
        SkRasterClip rc(this->devClipBounds());
        SkDraw draw;
        draw.fDst = SkPixmap(SkImageInfo::MakeUnknown(this->width(), this->height()), nullptr, 0);
        draw.fMatrix = &this->localToDevice();
        draw.fRC = &rc;
        draw.drawPoints(mode, count, pts, paint, this);
        return;
    }

    GrPrimitiveType primitiveType = point_mode_to_primitive_type(mode);

    const SkMatrix* viewMatrix = &this->localToDevice();
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    // This offsetting in device space matches the expectations of the Android framework for non-AA
    // points and lines.
    SkMatrix tempMatrix;
    if (GrIsPrimTypeLines(primitiveType) || GrPrimitiveType::kPoints == primitiveType) {
        tempMatrix = *viewMatrix;
        static const SkScalar kOffset = 0.063f; // Just greater than 1/16.
        tempMatrix.postTranslate(kOffset, kOffset);
        viewMatrix = &tempMatrix;
    }
#endif

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), fRenderTargetContext->colorInfo(), paint, *viewMatrix,
                          &grPaint)) {
        return;
    }

    static constexpr SkVertices::VertexMode kIgnoredMode = SkVertices::kTriangles_VertexMode;
    sk_sp<SkVertices> vertices = SkVertices::MakeCopy(kIgnoredMode, SkToS32(count), pts, nullptr,
                                                      nullptr);

    fRenderTargetContext->drawVertices(this->clip(), std::move(grPaint), *viewMatrix,
                                       std::move(vertices), &primitiveType);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawRect(const SkRect& rect, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawRect", fContext.get());

    GrStyle style(paint);

    // A couple reasons we might need to call drawPath.
    if (paint.getMaskFilter() || paint.getPathEffect()) {
        GrShape shape(rect, style);

        GrBlurUtils::drawShapeWithMaskFilter(fContext.get(), fRenderTargetContext.get(),
                                             this->clip(), paint, this->localToDevice(), shape);
        return;
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), fRenderTargetContext->colorInfo(), paint,
                          this->localToDevice(), &grPaint)) {
        return;
    }

    fRenderTargetContext->drawRect(this->clip(), std::move(grPaint), GrAA(paint.isAntiAlias()),
                                   this->localToDevice(), rect, &style);
}

void SkGpuDevice::drawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4],
                                 SkCanvas::QuadAAFlags aaFlags, const SkColor4f& color,
                                 SkBlendMode mode) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawEdgeAAQuad", fContext.get());

    SkPMColor4f dstColor = SkColor4fPrepForDst(color, fRenderTargetContext->colorInfo()).premul();

    GrPaint grPaint;
    grPaint.setColor4f(dstColor);
    if (mode != SkBlendMode::kSrcOver) {
        grPaint.setXPFactory(SkBlendMode_AsXPFactory(mode));
    }

    // This is exclusively meant for tiling operations, so keep AA enabled to handle MSAA seaming
    GrQuadAAFlags grAA = SkToGrQuadAAFlags(aaFlags);
    if (clip) {
        // Use fillQuadWithEdgeAA
        fRenderTargetContext->fillQuadWithEdgeAA(this->clip(), std::move(grPaint), GrAA::kYes, grAA,
                                                 this->localToDevice(), clip, nullptr);
    } else {
        // Use fillRectWithEdgeAA to preserve mathematical properties of dst being rectangular
        fRenderTargetContext->fillRectWithEdgeAA(this->clip(), std::move(grPaint), GrAA::kYes, grAA,
                                                 this->localToDevice(), rect);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawRRect", fContext.get());

    SkMaskFilterBase* mf = as_MFB(paint.getMaskFilter());
    if (mf) {
        if (mf->hasFragmentProcessor()) {
            mf = nullptr; // already handled in SkPaintToGrPaint
        }
    }

    GrStyle style(paint);

    if (mf || style.pathEffect()) {
        // A path effect will presumably transform this rrect into something else.
        GrShape shape(rrect, style);

        GrBlurUtils::drawShapeWithMaskFilter(fContext.get(), fRenderTargetContext.get(),
                                             this->clip(), paint, this->localToDevice(), shape);
        return;
    }

    SkASSERT(!style.pathEffect());

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), fRenderTargetContext->colorInfo(), paint,
                          this->localToDevice(), &grPaint)) {
        return;
    }

    fRenderTargetContext->drawRRect(this->clip(), std::move(grPaint), GrAA(paint.isAntiAlias()),
                                    this->localToDevice(), rrect, style);
}


void SkGpuDevice::drawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawDRRect", fContext.get());
    if (outer.isEmpty()) {
       return;
    }

    if (inner.isEmpty()) {
        return this->drawRRect(outer, paint);
    }

    SkStrokeRec stroke(paint);

    if (stroke.isFillStyle() && !paint.getMaskFilter() && !paint.getPathEffect()) {
        GrPaint grPaint;
        if (!SkPaintToGrPaint(this->context(), fRenderTargetContext->colorInfo(), paint,
                              this->localToDevice(), &grPaint)) {
            return;
        }

        fRenderTargetContext->drawDRRect(this->clip(), std::move(grPaint),
                                         GrAA(paint.isAntiAlias()), this->localToDevice(),
                                         outer, inner);
        return;
    }

    SkPath path;
    path.setIsVolatile(true);
    path.addRRect(outer);
    path.addRRect(inner);
    path.setFillType(SkPathFillType::kEvenOdd);

    // TODO: We are losing the possible mutability of the path here but this should probably be
    // fixed by upgrading GrShape to handle DRRects.
    GrShape shape(path, paint);

    GrBlurUtils::drawShapeWithMaskFilter(fContext.get(), fRenderTargetContext.get(), this->clip(),
                                         paint, this->localToDevice(), shape);
}


/////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawRegion(const SkRegion& region, const SkPaint& paint) {
    if (paint.getMaskFilter()) {
        SkPath path;
        region.getBoundaryPath(&path);
        path.setIsVolatile(true);
        return this->drawPath(path, paint, true);
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), fRenderTargetContext->colorInfo(), paint,
                          this->localToDevice(), &grPaint)) {
        return;
    }

    fRenderTargetContext->drawRegion(this->clip(), std::move(grPaint), GrAA(paint.isAntiAlias()),
                                     this->localToDevice(), region, GrStyle(paint));
}

void SkGpuDevice::drawOval(const SkRect& oval, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawOval", fContext.get());

    if (paint.getMaskFilter()) {
        // The RRect path can handle special case blurring
        SkRRect rr = SkRRect::MakeOval(oval);
        return this->drawRRect(rr, paint);
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), fRenderTargetContext->colorInfo(), paint,
                          this->localToDevice(), &grPaint)) {
        return;
    }

    fRenderTargetContext->drawOval(this->clip(), std::move(grPaint), GrAA(paint.isAntiAlias()),
                                   this->localToDevice(), oval, GrStyle(paint));
}

void SkGpuDevice::drawArc(const SkRect& oval, SkScalar startAngle,
                          SkScalar sweepAngle, bool useCenter, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawArc", fContext.get());
    if (paint.getMaskFilter()) {
        this->INHERITED::drawArc(oval, startAngle, sweepAngle, useCenter, paint);
        return;
    }
    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), fRenderTargetContext->colorInfo(), paint,
                          this->localToDevice(), &grPaint)) {
        return;
    }

    fRenderTargetContext->drawArc(this->clip(), std::move(grPaint), GrAA(paint.isAntiAlias()),
                                  this->localToDevice(), oval, startAngle, sweepAngle, useCenter,
                                  GrStyle(paint));
}

#include "include/core/SkMaskFilter.h"

///////////////////////////////////////////////////////////////////////////////
void SkGpuDevice::drawStrokedLine(const SkPoint points[2],
                                  const SkPaint& origPaint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawStrokedLine", fContext.get());
    // Adding support for round capping would require a
    // GrRenderTargetContext::fillRRectWithLocalMatrix entry point
    SkASSERT(SkPaint::kRound_Cap != origPaint.getStrokeCap());
    SkASSERT(SkPaint::kStroke_Style == origPaint.getStyle());
    SkASSERT(!origPaint.getPathEffect());
    SkASSERT(!origPaint.getMaskFilter());

    const SkScalar halfWidth = 0.5f * origPaint.getStrokeWidth();
    SkASSERT(halfWidth > 0);

    SkVector v = points[1] - points[0];

    SkScalar length = SkPoint::Normalize(&v);
    if (!length) {
        v.fX = 1.0f;
        v.fY = 0.0f;
    }

    SkPaint newPaint(origPaint);
    newPaint.setStyle(SkPaint::kFill_Style);

    SkScalar xtraLength = 0.0f;
    if (SkPaint::kButt_Cap != origPaint.getStrokeCap()) {
        xtraLength = halfWidth;
    }

    SkPoint mid = points[0] + points[1];
    mid.scale(0.5f);

    SkRect rect = SkRect::MakeLTRB(mid.fX-halfWidth, mid.fY - 0.5f*length - xtraLength,
                                   mid.fX+halfWidth, mid.fY + 0.5f*length + xtraLength);
    SkMatrix m;
    m.setSinCos(v.fX, -v.fY, mid.fX, mid.fY);

    SkMatrix local = m;

    m.postConcat(this->localToDevice());

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), fRenderTargetContext->colorInfo(), newPaint, m,
                          &grPaint)) {
        return;
    }

    fRenderTargetContext->fillRectWithLocalMatrix(
            this->clip(), std::move(grPaint), GrAA(newPaint.isAntiAlias()), m, rect, local);
}

void SkGpuDevice::drawPath(const SkPath& origSrcPath, const SkPaint& paint, bool pathIsMutable) {
    ASSERT_SINGLE_OWNER
    if (!origSrcPath.isInverseFillType() && !paint.getPathEffect()) {
        SkPoint points[2];
        if (SkPaint::kStroke_Style == paint.getStyle() && paint.getStrokeWidth() > 0 &&
            !paint.getMaskFilter() && SkPaint::kRound_Cap != paint.getStrokeCap() &&
            this->localToDevice().preservesRightAngles() && origSrcPath.isLine(points)) {
            // Path-based stroking looks better for thin rects
            SkScalar strokeWidth = this->localToDevice().getMaxScale() * paint.getStrokeWidth();
            if (strokeWidth >= 1.0f) {
                // Round capping support is currently disabled b.c. it would require a RRect
                // GrDrawOp that takes a localMatrix.
                this->drawStrokedLine(points, paint);
                return;
            }
        }
    }

    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawPath", fContext.get());
    if (!paint.getMaskFilter()) {
        GrPaint grPaint;
        if (!SkPaintToGrPaint(this->context(), fRenderTargetContext->colorInfo(), paint,
                              this->localToDevice(), &grPaint)) {
            return;
        }
        fRenderTargetContext->drawPath(this->clip(), std::move(grPaint), GrAA(paint.isAntiAlias()),
                                       this->localToDevice(), origSrcPath, GrStyle(paint));
        return;
    }

    // TODO: losing possible mutability of 'origSrcPath' here
    GrShape shape(origSrcPath, paint);

    GrBlurUtils::drawShapeWithMaskFilter(fContext.get(), fRenderTargetContext.get(), this->clip(),
                                         paint, this->localToDevice(), shape);
}

void SkGpuDevice::drawSpecial(SkSpecialImage* special, int left, int top, const SkPaint& paint,
                              SkImage* clipImage, const SkMatrix& clipMatrix) {
    SkASSERT(!paint.getMaskFilter());

    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawSpecial", fContext.get());

    sk_sp<SkSpecialImage> result;
    if (paint.getImageFilter()) {
        SkIPoint offset = { 0, 0 };

        result = this->filterTexture(special, left, top, &offset, paint.getImageFilter());
        if (!result) {
            return;
        }

        left += offset.fX;
        top += offset.fY;
    } else {
        result = sk_ref_sp(special);
    }

    SkASSERT(result->isTextureBacked());
    GrSurfaceProxyView view = result->view(this->context());
    if (!view) {
        return;
    }

    SkMatrix ctm = this->localToDevice();
    ctm.postTranslate(-SkIntToScalar(left), -SkIntToScalar(top));

    SkPaint tmpUnfiltered(paint);
    tmpUnfiltered.setImageFilter(nullptr);

    auto fp = GrTextureEffect::Make(std::move(view), special->alphaType());
    fp = GrColorSpaceXformEffect::Make(std::move(fp), result->getColorSpace(), result->alphaType(),
                                       fRenderTargetContext->colorInfo().colorSpace());
    if (GrColorTypeIsAlphaOnly(SkColorTypeToGrColorType(result->colorType()))) {
        fp = GrFragmentProcessor::MakeInputPremulAndMulByOutput(std::move(fp));
    } else {
        if (paint.getColor4f().isOpaque()) {
            fp = GrFragmentProcessor::OverrideInput(std::move(fp), SK_PMColor4fWHITE, false);
        } else {
            fp = GrFragmentProcessor::MulChildByInputAlpha(std::move(fp));
        }
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaintReplaceShader(this->context(), fRenderTargetContext->colorInfo(),
                                       tmpUnfiltered, std::move(fp), &grPaint)) {
        return;
    }

    const SkIRect& subset = result->subset();
    SkRect dstRect = SkRect::Make(SkIRect::MakeXYWH(left, top, subset.width(), subset.height()));
    SkRect srcRect = SkRect::Make(subset);
    if (clipImage) {
        // Add the image as a simple texture effect applied to coverage. Accessing content outside
        // of the clip image should behave as if it were a decal (i.e. zero coverage). However, to
        // limit pixels touched and hardware checks, we draw the clip image geometry to get the
        // decal effect.
        auto filter = paint.getFilterQuality() > kNone_SkFilterQuality
                              ? GrSamplerState::Filter::kBilerp
                              : GrSamplerState::Filter::kNearest;
        GrSurfaceProxyView clipView = as_IB(clipImage)->refView(this->context(), GrMipMapped::kNo);
        // Fold clip matrix into ctm
        ctm.preConcat(clipMatrix);
        SkMatrix inverseClipMatrix;

        std::unique_ptr<GrFragmentProcessor> cfp;
        if (clipView && ctm.invert(&inverseClipMatrix)) {
            GrColorType srcColorType = SkColorTypeToGrColorType(clipImage->colorType());

            cfp = GrTextureEffect::Make(std::move(clipView), clipImage->alphaType(),
                                        inverseClipMatrix, filter);
            if (srcColorType != GrColorType::kAlpha_8) {
                cfp = GrFragmentProcessor::SwizzleOutput(std::move(cfp), GrSwizzle::AAAA());
            }
        }

        if (cfp) {
            // If the grPaint already has coverage, this adds an additional stage that multiples
            // the image's alpha channel with the prior coverage.
            grPaint.addCoverageFragmentProcessor(std::move(cfp));

            // Undo the offset that was needed for shader coord transforms to get the transform for
            // the actual drawn geometry.
            ctm.postTranslate(SkIntToScalar(left), SkIntToScalar(top));
            inverseClipMatrix.preTranslate(-SkIntToScalar(left), -SkIntToScalar(top));
            SkRect clipGeometry = SkRect::MakeWH(clipImage->width(), clipImage->height());
            if (!clipGeometry.contains(inverseClipMatrix.mapRect(dstRect))) {
                // Draw the clip geometry since it is smaller, using dstRect as an extra scissor
                SkClipStack clip(this->cs());
                clip.clipDevRect(SkIRect::MakeXYWH(left, top, subset.width(), subset.height()),
                                 SkClipOp::kIntersect);
                SkMatrix local = SkMatrix::Concat(SkMatrix::MakeRectToRect(
                        dstRect, srcRect, SkMatrix::kFill_ScaleToFit), ctm);
                fRenderTargetContext->fillRectWithLocalMatrix(GrClipStackClip(&clip),
                        std::move(grPaint), GrAA(paint.isAntiAlias()), ctm, clipGeometry, local);
                return;
            }
            // Else fall through and draw the subset since that is contained in the clip geometry
        }
        // Else some issue configuring the coverage FP, so just draw without the clip mask image
    }
    // Draw directly in screen space, possibly with an extra coverage processor
    fRenderTargetContext->fillRectToRect(this->clip(), std::move(grPaint),
            GrAA(paint.isAntiAlias()), SkMatrix::I(), dstRect, srcRect);
}

sk_sp<SkSpecialImage> SkGpuDevice::makeSpecial(const SkBitmap& bitmap) {
    // TODO: this makes a tight copy of 'bitmap' but it doesn't have to be (given SkSpecialImage's
    // semantics). Since this is cached we would have to bake the fit into the cache key though.
    auto view = GrMakeCachedBitmapProxyView(fContext.get(), bitmap);
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
                                               &this->surfaceProps());
}

sk_sp<SkSpecialImage> SkGpuDevice::makeSpecial(const SkImage* image) {
    SkPixmap pm;
    if (image->isTextureBacked()) {
        const GrSurfaceProxyView* view = as_IB(image)->view(this->context());
        SkASSERT(view);

        return SkSpecialImage::MakeDeferredFromGpu(fContext.get(),
                                                   SkIRect::MakeWH(image->width(), image->height()),
                                                   image->uniqueID(),
                                                   *view,
                                                   SkColorTypeToGrColorType(image->colorType()),
                                                   image->refColorSpace(),
                                                   &this->surfaceProps());
    } else if (image->peekPixels(&pm)) {
        SkBitmap bm;

        bm.installPixels(pm);
        return this->makeSpecial(bm);
    } else {
        return nullptr;
    }
}

sk_sp<SkSpecialImage> SkGpuDevice::snapSpecial(const SkIRect& subset, bool forceCopy) {
    GrRenderTargetContext* rtc = this->accessRenderTargetContext();

    // If we are wrapping a vulkan secondary command buffer, then we can't snap off a special image
    // since it would require us to make a copy of the underlying VkImage which we don't have access
    // to. Additionaly we can't stop and start the render pass that is used with the secondary
    // command buffer.
    if (rtc->wrapsVkSecondaryCB()) {
        return nullptr;
    }

    SkASSERT(rtc->asSurfaceProxy());

    SkIRect finalSubset = subset;
    GrSurfaceProxyView view = rtc->readSurfaceView();
    if (forceCopy || !view.asTextureProxy()) {
        // When the device doesn't have a texture, or a copy is requested, we create a temporary
        // texture that matches the device contents
        view = GrSurfaceProxy::Copy(fContext.get(),
                                    rtc->asSurfaceProxy(),
                                    view.origin(),
                                    rtc->colorInfo().colorType(),
                                    GrMipMapped::kNo,      // Don't auto generate mips
                                    subset,
                                    SkBackingFit::kApprox,
                                    SkBudgeted::kYes);     // Always budgeted
        if (!view) {
            return nullptr;
        }

        // Since this copied only the requested subset, the special image wrapping the proxy no
        // longer needs the original subset.
        finalSubset = SkIRect::MakeSize(view.proxy()->dimensions());
    }

    GrColorType ct = SkColorTypeToGrColorType(this->imageInfo().colorType());

    return SkSpecialImage::MakeDeferredFromGpu(fContext.get(),
                                               finalSubset,
                                               kNeedNewImageUniqueID_SpecialImage,
                                               std::move(view),
                                               ct,
                                               this->imageInfo().refColorSpace(),
                                               &this->surfaceProps());
}

void SkGpuDevice::drawDevice(SkBaseDevice* device,
                             int left, int top, const SkPaint& paint) {
    SkASSERT(!paint.getImageFilter());
    SkASSERT(!paint.getMaskFilter());

    ASSERT_SINGLE_OWNER
    // clear of the source device must occur before CHECK_SHOULD_DRAW
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawDevice", fContext.get());

    // drawDevice is defined to be in device coords.
    SkGpuDevice* dev = static_cast<SkGpuDevice*>(device);
    sk_sp<SkSpecialImage> srcImg(dev->snapSpecial(SkIRect::MakeWH(dev->width(), dev->height())));
    if (!srcImg) {
        return;
    }

    this->drawSpecial(srcImg.get(), left, top, paint, nullptr, SkMatrix::I());
}

void SkGpuDevice::drawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                const SkPaint& paint, SkCanvas::SrcRectConstraint constraint) {
    ASSERT_SINGLE_OWNER
    GrQuadAAFlags aaFlags = paint.isAntiAlias() ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone;
    this->drawImageQuad(image, src, &dst, nullptr, GrAA(paint.isAntiAlias()), aaFlags, nullptr,
                        paint, constraint);
}

// When drawing nine-patches or n-patches, cap the filter quality at kBilerp.
static GrSamplerState::Filter compute_lattice_filter_mode(const SkPaint& paint) {
    if (paint.getFilterQuality() == kNone_SkFilterQuality) {
        return GrSamplerState::Filter::kNearest;
    }

    return GrSamplerState::Filter::kBilerp;
}

void SkGpuDevice::drawImageNine(const SkImage* image,
                                const SkIRect& center, const SkRect& dst, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    uint32_t pinnedUniqueID;
    auto iter = std::make_unique<SkLatticeIter>(image->width(), image->height(), center, dst);
    if (GrSurfaceProxyView view = as_IB(image)->refPinnedView(this->context(), &pinnedUniqueID)) {
        GrTextureAdjuster adjuster(this->context(), std::move(view),
                                   image->imageInfo().colorInfo(), pinnedUniqueID);
        this->drawProducerLattice(&adjuster, std::move(iter), dst, paint);
    } else {
        SkBitmap bm;
        if (image->isLazyGenerated()) {
            GrImageTextureMaker maker(fContext.get(), image, GrImageTexGenPolicy::kDraw);
            this->drawProducerLattice(&maker, std::move(iter), dst, paint);
        } else if (as_IB(image)->getROPixels(&bm)) {
            GrBitmapTextureMaker maker(fContext.get(), bm, GrImageTexGenPolicy::kDraw);
            this->drawProducerLattice(&maker, std::move(iter), dst, paint);
        }
    }
}

void SkGpuDevice::drawProducerLattice(GrTextureProducer* producer,
                                      std::unique_ptr<SkLatticeIter> iter, const SkRect& dst,
                                      const SkPaint& origPaint) {
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawProducerLattice", fContext.get());
    SkTCopyOnFirstWrite<SkPaint> paint(&origPaint);

    if (!producer->isAlphaOnly() && (paint->getColor() & 0x00FFFFFF) != 0x00FFFFFF) {
        paint.writable()->setColor(SkColorSetARGB(origPaint.getAlpha(), 0xFF, 0xFF, 0xFF));
    }
    GrPaint grPaint;
    if (!SkPaintToGrPaintWithPrimitiveColor(this->context(), fRenderTargetContext->colorInfo(),
                                            *paint, &grPaint)) {
        return;
    }

    auto dstColorSpace = fRenderTargetContext->colorInfo().colorSpace();
    const GrSamplerState::Filter filter = compute_lattice_filter_mode(*paint);
    auto view = producer->view(filter);
    if (!view) {
        return;
    }
    auto csxf = GrColorSpaceXform::Make(producer->colorSpace(), producer->alphaType(),
                                        dstColorSpace,          kPremul_SkAlphaType);

    fRenderTargetContext->drawImageLattice(this->clip(), std::move(grPaint), this->localToDevice(),
                                           std::move(view), producer->alphaType(), std::move(csxf),
                                           filter, std::move(iter), dst);
}

void SkGpuDevice::drawImageLattice(const SkImage* image,
                                   const SkCanvas::Lattice& lattice, const SkRect& dst,
                                   const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    uint32_t pinnedUniqueID;
    auto iter = std::make_unique<SkLatticeIter>(lattice, dst);
    if (GrSurfaceProxyView view = as_IB(image)->refPinnedView(this->context(), &pinnedUniqueID)) {
        GrTextureAdjuster adjuster(this->context(), std::move(view),
                                   image->imageInfo().colorInfo(), pinnedUniqueID);
        this->drawProducerLattice(&adjuster, std::move(iter), dst, paint);
    } else {
        SkBitmap bm;
        if (image->isLazyGenerated()) {
            GrImageTextureMaker maker(fContext.get(), image, GrImageTexGenPolicy::kDraw);
            this->drawProducerLattice(&maker, std::move(iter), dst, paint);
        } else if (as_IB(image)->getROPixels(&bm)) {
            GrBitmapTextureMaker maker(fContext.get(), bm, GrImageTexGenPolicy::kDraw);
            this->drawProducerLattice(&maker, std::move(iter), dst, paint);
        }
    }
}

static bool init_vertices_paint(GrContext* context, const GrColorInfo& colorInfo,
                                const SkPaint& skPaint, const SkMatrix& matrix, SkBlendMode bmode,
                                bool hasTexs, bool hasColors, GrPaint* grPaint) {
    if (hasTexs && skPaint.getShader()) {
        if (hasColors) {
            // When there are texs and colors the shader and colors are combined using bmode.
            return SkPaintToGrPaintWithXfermode(context, colorInfo, skPaint, matrix, bmode,
                                                grPaint);
        } else {
            // We have a shader, but no colors to blend it against.
            return SkPaintToGrPaint(context, colorInfo, skPaint, matrix, grPaint);
        }
    } else {
        if (hasColors) {
            // We have colors, but either have no shader or no texture coords (which implies that
            // we should ignore the shader).
            return SkPaintToGrPaintWithPrimitiveColor(context, colorInfo, skPaint, grPaint);
        } else {
            // No colors and no shaders. Just draw with the paint color.
            return SkPaintToGrPaintNoShader(context, colorInfo, skPaint, grPaint);
        }
    }
}

void SkGpuDevice::wireframeVertices(SkVertices::VertexMode vmode, int vertexCount,
                                    const SkPoint vertices[],
                                    SkBlendMode bmode,
                                    const uint16_t indices[], int indexCount,
                                    const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "wireframeVertices", fContext.get());

    SkPaint copy(paint);
    copy.setStyle(SkPaint::kStroke_Style);
    copy.setStrokeWidth(0);

    GrPaint grPaint;
    // we ignore the shader since we have no texture coordinates.
    if (!SkPaintToGrPaintNoShader(this->context(), fRenderTargetContext->colorInfo(), copy,
                                  &grPaint)) {
        return;
    }

    int triangleCount = 0;
    int n = (nullptr == indices) ? vertexCount : indexCount;
    switch (vmode) {
        case SkVertices::kTriangles_VertexMode:
            triangleCount = n / 3;
            break;
        case SkVertices::kTriangleStrip_VertexMode:
            triangleCount = n - 2;
            break;
        case SkVertices::kTriangleFan_VertexMode:
            SK_ABORT("Unexpected triangle fan.");
            break;
    }

    VertState       state(vertexCount, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(vmode);

    //number of indices for lines per triangle with kLines
    indexCount = triangleCount * 6;

    static constexpr SkVertices::VertexMode kIgnoredMode = SkVertices::kTriangles_VertexMode;
    SkVertices::Builder builder(kIgnoredMode, vertexCount, indexCount, 0);
    memcpy(builder.positions(), vertices, vertexCount * sizeof(SkPoint));

    uint16_t* lineIndices = builder.indices();
    int i = 0;
    while (vertProc(&state)) {
        lineIndices[i]     = state.f0;
        lineIndices[i + 1] = state.f1;
        lineIndices[i + 2] = state.f1;
        lineIndices[i + 3] = state.f2;
        lineIndices[i + 4] = state.f2;
        lineIndices[i + 5] = state.f0;
        i += 6;
    }

    GrPrimitiveType primitiveType = GrPrimitiveType::kLines;
    fRenderTargetContext->drawVertices(this->clip(),
                                       std::move(grPaint),
                                       this->localToDevice(),
                                       builder.detach(),
                                       &primitiveType);
}

void SkGpuDevice::drawVertices(const SkVertices* vertices, SkBlendMode mode, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawVertices", fContext.get());
    SkASSERT(vertices);

    SkVerticesPriv info(vertices->priv());

    const SkRuntimeEffect* effect =
            paint.getShader() ? as_SB(paint.getShader())->asRuntimeEffect() : nullptr;

    // Pretend that we have tex coords when using custom per-vertex data. The shader is going to
    // use those (rather than local coords), but our paint conversion remains the same.
    GrPaint grPaint;
    bool hasColors = info.hasColors();
    bool hasTexs = info.hasTexCoords() || info.hasCustomData();
    if ((!hasTexs || !paint.getShader()) && !hasColors) {
        // The dreaded wireframe mode. Fallback to drawVertices and go so slooooooow.
        this->wireframeVertices(info.mode(), info.vertexCount(), info.positions(), mode,
                                info.indices(), info.indexCount(), paint);
        return;
    }
    if (!init_vertices_paint(fContext.get(), fRenderTargetContext->colorInfo(), paint,
                             this->localToDevice(), mode, hasTexs, hasColors, &grPaint)) {
        return;
    }
    fRenderTargetContext->drawVertices(this->clip(), std::move(grPaint), this->localToDevice(),
                                       sk_ref_sp(const_cast<SkVertices*>(vertices)), nullptr,
                                       effect);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawShadow(const SkPath& path, const SkDrawShadowRec& rec) {

    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawShadow", fContext.get());

    if (!fRenderTargetContext->drawFastShadow(this->clip(), this->localToDevice(), path, rec)) {
        // failed to find an accelerated case
        this->INHERITED::drawShadow(path, rec);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawAtlas(const SkImage* atlas, const SkRSXform xform[],
                            const SkRect texRect[], const SkColor colors[], int count,
                            SkBlendMode mode, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawAtlas", fContext.get());

    SkPaint p(paint);
    p.setShader(atlas->makeShader());

    GrPaint grPaint;
    if (colors) {
        if (!SkPaintToGrPaintWithXfermode(this->context(), fRenderTargetContext->colorInfo(), p,
                                          this->localToDevice(), (SkBlendMode)mode, &grPaint)) {
            return;
        }
    } else {
        if (!SkPaintToGrPaint(this->context(), fRenderTargetContext->colorInfo(), p,
                              this->localToDevice(), &grPaint)) {
            return;
        }
    }

    fRenderTargetContext->drawAtlas(
            this->clip(), std::move(grPaint), this->localToDevice(), count, xform, texRect, colors);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawGlyphRunList(const SkGlyphRunList& glyphRunList) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawGlyphRunList", fContext.get());

    // Check for valid input
    if (!this->localToDevice().isFinite() || !glyphRunList.allFontsFinite()) {
        return;
    }

    fRenderTargetContext->drawGlyphRunList(this->clip(), this->localToDevice(), glyphRunList);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawDrawable(SkDrawable* drawable, const SkMatrix* matrix, SkCanvas* canvas) {
    GrBackendApi api = this->context()->backend();
    if (GrBackendApi::kVulkan == api) {
        const SkMatrix& ctm = canvas->getTotalMatrix();
        const SkMatrix& combinedMatrix = matrix ? SkMatrix::Concat(ctm, *matrix) : ctm;
        std::unique_ptr<SkDrawable::GpuDrawHandler> gpuDraw =
                drawable->snapGpuDrawHandler(api, combinedMatrix, canvas->getDeviceClipBounds(),
                                             this->imageInfo());
        if (gpuDraw) {
            fRenderTargetContext->drawDrawable(std::move(gpuDraw), drawable->getBounds());
            return;
        }
    }
    this->INHERITED::drawDrawable(drawable, matrix, canvas);
}


///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::flush() {
    this->flush(SkSurface::BackendSurfaceAccess::kNoAccess, GrFlushInfo());
}

GrSemaphoresSubmitted SkGpuDevice::flush(SkSurface::BackendSurfaceAccess access,
                                         const GrFlushInfo& info) {
    ASSERT_SINGLE_OWNER

    return fRenderTargetContext->flush(access, info);
}

bool SkGpuDevice::wait(int numSemaphores, const GrBackendSemaphore* waitSemaphores) {
    ASSERT_SINGLE_OWNER

    return fRenderTargetContext->waitOnSemaphores(numSemaphores, waitSemaphores);
}

///////////////////////////////////////////////////////////////////////////////

SkBaseDevice* SkGpuDevice::onCreateDevice(const CreateInfo& cinfo, const SkPaint*) {
    ASSERT_SINGLE_OWNER

    SkSurfaceProps props(this->surfaceProps().flags(), cinfo.fPixelGeometry);

    // layers are never drawn in repeat modes, so we can request an approx
    // match and ignore any padding.
    SkBackingFit fit = kNever_TileUsage == cinfo.fTileUsage ? SkBackingFit::kApprox
                                                            : SkBackingFit::kExact;

    SkASSERT(cinfo.fInfo.colorType() != kRGBA_1010102_SkColorType);

    auto rtc = GrRenderTargetContext::MakeWithFallback(
            fContext.get(), SkColorTypeToGrColorType(cinfo.fInfo.colorType()),
            fRenderTargetContext->colorInfo().refColorSpace(), fit, cinfo.fInfo.dimensions(),
            fRenderTargetContext->numSamples(), GrMipMapped::kNo,
            fRenderTargetContext->asSurfaceProxy()->isProtected(), kBottomLeft_GrSurfaceOrigin,
            SkBudgeted::kYes, &props);
    if (!rtc) {
        return nullptr;
    }

    // Skia's convention is to only clear a device if it is non-opaque.
    InitContents init = cinfo.fInfo.isOpaque() ? kUninit_InitContents : kClear_InitContents;

    return SkGpuDevice::Make(fContext.get(), std::move(rtc), init).release();
}

sk_sp<SkSurface> SkGpuDevice::makeSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    ASSERT_SINGLE_OWNER
    // TODO: Change the signature of newSurface to take a budgeted parameter.
    static const SkBudgeted kBudgeted = SkBudgeted::kNo;
    return SkSurface::MakeRenderTarget(fContext.get(), kBudgeted, info,
                                       fRenderTargetContext->numSamples(),
                                       fRenderTargetContext->origin(), &props);
}

SkImageFilterCache* SkGpuDevice::getImageFilterCache() {
    ASSERT_SINGLE_OWNER
    // We always return a transient cache, so it is freed after each
    // filter traversal.
    return SkImageFilterCache::Create(SkImageFilterCache::kDefaultTransientSize);
}

////////////////////////////////////////////////////////////////////////////////////

bool SkGpuDevice::android_utils_clipWithStencil() {
    SkRegion clipRegion;
    this->onAsRgnClip(&clipRegion);
    if (clipRegion.isEmpty()) {
        return false;
    }
    GrRenderTargetContext* rtc = this->accessRenderTargetContext();
    if (!rtc) {
        return false;
    }
    GrPaint grPaint;
    grPaint.setXPFactory(GrDisableColorXPFactory::Get());
    GrNoClip noClip;
    static constexpr GrUserStencilSettings kDrawToStencil(
        GrUserStencilSettings::StaticInit<
            0x1,
            GrUserStencilTest::kAlways,
            0x1,
            GrUserStencilOp::kReplace,
            GrUserStencilOp::kReplace,
            0x1>()
    );
    rtc->drawRegion(noClip, std::move(grPaint), GrAA::kNo, SkMatrix::I(), clipRegion,
                    GrStyle::SimpleFill(), &kDrawToStencil);
    return true;
}

