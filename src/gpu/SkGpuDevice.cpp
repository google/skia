/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGpuDevice.h"

#include "GrBitmapTextureMaker.h"
#include "GrBlurUtils.h"
#include "GrContext.h"
#include "GrGpu.h"
#include "GrImageTextureMaker.h"
#include "GrRenderTargetContextPriv.h"
#include "GrStyle.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTextureAdjuster.h"
#include "GrTextureProxy.h"
#include "GrTracing.h"
#include "SkCanvasPriv.h"
#include "SkDraw.h"
#include "SkGlyphCache.h"
#include "SkGr.h"
#include "SkImageFilter.h"
#include "SkImageFilterCache.h"
#include "SkImageInfoPriv.h"
#include "SkImage_Base.h"
#include "SkLatticeIter.h"
#include "SkMaskFilter.h"
#include "SkPathEffect.h"
#include "SkPicture.h"
#include "SkPictureData.h"
#include "SkRRect.h"
#include "SkRasterClip.h"
#include "SkReadPixelsRec.h"
#include "SkRecord.h"
#include "SkSpecialImage.h"
#include "SkStroke.h"
#include "SkSurface.h"
#include "SkSurface_Gpu.h"
#include "SkTLazy.h"
#include "SkUtils.h"
#include "SkVertState.h"
#include "SkVertices.h"
#include "SkWritePixelsRec.h"
#include "effects/GrBicubicEffect.h"
#include "effects/GrSimpleTextureEffect.h"
#include "effects/GrTextureDomain.h"
#include "../private/SkShadowFlags.h"
#include "text/GrTextUtils.h"

#if SK_SUPPORT_GPU

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fContext->debugSingleOwner());)

#if 0
    extern bool (*gShouldDrawProc)();
    #define CHECK_SHOULD_DRAW()                                 \
        do {                                                    \
            if (gShouldDrawProc && !gShouldDrawProc()) return;  \
        } while (0)
#else
#define CHECK_SHOULD_DRAW()
#endif

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
                                     sk_sp<GrRenderTargetContext> renderTargetContext,
                                     int width, int height,
                                     InitContents init) {
    if (!renderTargetContext || renderTargetContext->wasAbandoned()) {
        return nullptr;
    }
    unsigned flags;
    if (!CheckAlphaTypeAndGetFlags(nullptr, init, &flags)) {
        return nullptr;
    }
    return sk_sp<SkGpuDevice>(new SkGpuDevice(context, std::move(renderTargetContext),
                                              width, height, flags));
}

sk_sp<SkGpuDevice> SkGpuDevice::Make(GrContext* context, SkBudgeted budgeted,
                                     const SkImageInfo& info, int sampleCount,
                                     GrSurfaceOrigin origin,
                                     const SkSurfaceProps* props, InitContents init) {
    unsigned flags;
    if (!CheckAlphaTypeAndGetFlags(&info, init, &flags)) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> renderTargetContext(MakeRenderTargetContext(context, budgeted,
                                                                             info, sampleCount,
                                                                             origin, props));
    if (!renderTargetContext) {
        return nullptr;
    }

    return sk_sp<SkGpuDevice>(new SkGpuDevice(context, std::move(renderTargetContext),
                                              info.width(), info.height(), flags));
}

static SkImageInfo make_info(GrRenderTargetContext* context, int w, int h, bool opaque) {
    SkColorType colorType;
    if (!GrPixelConfigToColorType(context->config(), &colorType)) {
        colorType = kUnknown_SkColorType;
    }
    return SkImageInfo::Make(w, h, colorType,
                             opaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType,
                             context->refColorSpace());
}

SkGpuDevice::SkGpuDevice(GrContext* context, sk_sp<GrRenderTargetContext> renderTargetContext,
                         int width, int height, unsigned flags)
    : INHERITED(make_info(renderTargetContext.get(), width, height,
                          SkToBool(flags & kIsOpaque_Flag)), renderTargetContext->surfaceProps())
    , fContext(SkRef(context))
    , fRenderTargetContext(std::move(renderTargetContext))
{
    fSize.set(width, height);
    fOpaque = SkToBool(flags & kIsOpaque_Flag);

    if (flags & kNeedClear_Flag) {
        this->clearAll();
    }
}

sk_sp<GrRenderTargetContext> SkGpuDevice::MakeRenderTargetContext(
                                                               GrContext* context,
                                                               SkBudgeted budgeted,
                                                               const SkImageInfo& origInfo,
                                                               int sampleCount,
                                                               GrSurfaceOrigin origin,
                                                               const SkSurfaceProps* surfaceProps) {
    if (kUnknown_SkColorType == origInfo.colorType() ||
        origInfo.width() < 0 || origInfo.height() < 0) {
        return nullptr;
    }

    if (!context) {
        return nullptr;
    }

    GrPixelConfig config = SkImageInfo2GrPixelConfig(origInfo, *context->caps());
    // This method is used to create SkGpuDevice's for SkSurface_Gpus. In this case
    // they need to be exact.
    return context->makeDeferredRenderTargetContext(
                                    SkBackingFit::kExact,
                                    origInfo.width(), origInfo.height(),
                                    config, origInfo.refColorSpace(), sampleCount,
                                    origin, surfaceProps, budgeted);
}

sk_sp<SkSpecialImage> SkGpuDevice::filterTexture(SkSpecialImage* srcImg,
                                                 int left, int top,
                                                 SkIPoint* offset,
                                                 const SkImageFilter* filter) {
    SkASSERT(srcImg->isTextureBacked());
    SkASSERT(filter);

    SkMatrix matrix = this->ctm();
    matrix.postTranslate(SkIntToScalar(-left), SkIntToScalar(-top));
    const SkIRect clipBounds = this->devClipBounds().makeOffset(-left, -top);
    sk_sp<SkImageFilterCache> cache(this->getImageFilterCache());
    SkImageFilter::OutputProperties outputProperties(fRenderTargetContext->getColorSpace());
    SkImageFilter::Context ctx(matrix, clipBounds, cache.get(), outputProperties);

    return filter->filterImage(srcImg, ctx, offset);
}

///////////////////////////////////////////////////////////////////////////////

bool SkGpuDevice::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                               int x, int y) {
    ASSERT_SINGLE_OWNER

    if (!SkImageInfoValidConversion(dstInfo, this->imageInfo())) {
        return false;
    }

    SkReadPixelsRec rec(dstInfo, dstPixels, dstRowBytes, x, y);
    if (!rec.trim(this->width(), this->height())) {
        return false;
    }

    return fRenderTargetContext->readPixels(rec.fInfo, rec.fPixels, rec.fRowBytes, rec.fX, rec.fY);
}

bool SkGpuDevice::onWritePixels(const SkImageInfo& srcInfo, const void* srcPixels,
                                size_t srcRowBytes, int x, int y) {
    ASSERT_SINGLE_OWNER

    if (!SkImageInfoValidConversion(this->imageInfo(), srcInfo)) {
        return false;
    }

    SkWritePixelsRec rec(srcInfo, srcPixels, srcRowBytes, x, y);
    if (!rec.trim(this->width(), this->height())) {
        return false;
    }

    return fRenderTargetContext->writePixels(rec.fInfo, rec.fPixels, rec.fRowBytes, rec.fX, rec.fY);
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
    fRenderTargetContext->clear(&rect, 0x0, true);
}

void SkGpuDevice::replaceRenderTargetContext(bool shouldRetainContent) {
    ASSERT_SINGLE_OWNER

    SkBudgeted budgeted = fRenderTargetContext->priv().isBudgeted();

    // This entry point is used by SkSurface_Gpu::onCopyOnWrite so it must create a
    // kExact-backed render target context.
    sk_sp<GrRenderTargetContext> newRTC(MakeRenderTargetContext(
                                                            this->context(),
                                                            budgeted,
                                                            this->imageInfo(),
                                                            fRenderTargetContext->numColorSamples(),
                                                            fRenderTargetContext->origin(),
                                                            &this->surfaceProps()));
    if (!newRTC) {
        return;
    }
    SkASSERT(newRTC->asSurfaceProxy()->priv().isExact());

    if (shouldRetainContent) {
        if (fRenderTargetContext->wasAbandoned()) {
            return;
        }
        newRTC->copy(fRenderTargetContext->asSurfaceProxy());
    }

    fRenderTargetContext = newRTC;
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawPaint(const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    CHECK_SHOULD_DRAW();
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawPaint", fContext.get());

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), fRenderTargetContext.get(), paint, this->ctm(),
                          &grPaint)) {
        return;
    }

    fRenderTargetContext->drawPaint(this->clip(), std::move(grPaint), this->ctm());
}

// must be in SkCanvas::PointMode order
static const GrPrimitiveType gPointMode2PrimitiveType[] = {
    GrPrimitiveType::kPoints,
    GrPrimitiveType::kLines,
    GrPrimitiveType::kLineStrip
};

void SkGpuDevice::drawPoints(SkCanvas::PointMode mode,
                             size_t count, const SkPoint pts[], const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawPoints", fContext.get());
    CHECK_SHOULD_DRAW();

    SkScalar width = paint.getStrokeWidth();
    if (width < 0) {
        return;
    }

    if (paint.getPathEffect() && 2 == count && SkCanvas::kLines_PointMode == mode) {
        GrStyle style(paint, SkPaint::kStroke_Style);
        GrPaint grPaint;
        if (!SkPaintToGrPaint(this->context(), fRenderTargetContext.get(), paint, this->ctm(),
                              &grPaint)) {
            return;
        }
        SkPath path;
        path.setIsVolatile(true);
        path.moveTo(pts[0]);
        path.lineTo(pts[1]);
        fRenderTargetContext->drawPath(this->clip(), std::move(grPaint),
                                       GrBoolToAA(paint.isAntiAlias()), this->ctm(), path, style);
        return;
    }

    SkScalar scales[2];
    bool isHairline = (0 == width) || (1 == width && this->ctm().getMinMaxScales(scales) &&
                                       SkScalarNearlyEqual(scales[0], 1.f) &&
                                       SkScalarNearlyEqual(scales[1], 1.f));
    // we only handle non-antialiased hairlines and paints without path effects or mask filters,
    // else we let the SkDraw call our drawPath()
    if (!isHairline || paint.getPathEffect() || paint.getMaskFilter() || paint.isAntiAlias()) {
        SkRasterClip rc(this->devClipBounds());
        SkDraw draw;
        draw.fDst = SkPixmap(SkImageInfo::MakeUnknown(this->width(), this->height()), nullptr, 0);
        draw.fMatrix = &this->ctm();
        draw.fRC = &rc;
        draw.drawPoints(mode, count, pts, paint, this);
        return;
    }

    GrPrimitiveType primitiveType = gPointMode2PrimitiveType[mode];

    const SkMatrix* viewMatrix = &this->ctm();
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
    if (!SkPaintToGrPaint(this->context(), fRenderTargetContext.get(), paint, *viewMatrix,
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
    CHECK_SHOULD_DRAW();


    // A couple reasons we might need to call drawPath.
    if (paint.getMaskFilter() || paint.getPathEffect()) {
        SkPath path;
        path.setIsVolatile(true);
        path.addRect(rect);
        GrBlurUtils::drawPathWithMaskFilter(fContext.get(), fRenderTargetContext.get(),
                                            this->clip(), path, paint, this->ctm(), nullptr,
                                            this->devClipBounds(), true);
        return;
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), fRenderTargetContext.get(), paint, this->ctm(),
                          &grPaint)) {
        return;
    }

    GrStyle style(paint);
    fRenderTargetContext->drawRect(this->clip(), std::move(grPaint),
                                   GrBoolToAA(paint.isAntiAlias()), this->ctm(), rect, &style);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawRRect", fContext.get());
    CHECK_SHOULD_DRAW();

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), fRenderTargetContext.get(), paint, this->ctm(),
                          &grPaint)) {
        return;
    }

    SkMaskFilter* mf = paint.getMaskFilter();
    if (mf && mf->asFragmentProcessor(nullptr)) {
        mf = nullptr; // already handled in SkPaintToGrPaint
    }

    GrStyle style(paint);
    if (mf) {
        // try to hit the fast path for drawing filtered round rects

        SkRRect devRRect;
        if (rrect.transform(this->ctm(), &devRRect)) {
            if (devRRect.allCornersCircular()) {
                SkRect maskRect;
                if (mf->canFilterMaskGPU(devRRect, this->devClipBounds(),
                                         this->ctm(), &maskRect)) {
                    SkIRect finalIRect;
                    maskRect.roundOut(&finalIRect);

                    // we used to test finalIRect for quickReject, but that seems unlikely
                    // given that the original shape was not rejected...

                    if (mf->directFilterRRectMaskGPU(this->context(), fRenderTargetContext.get(),
                                                     std::move(grPaint), this->clip(), this->ctm(),
                                                     style.strokeRec(), rrect, devRRect)) {
                        return;
                    }
                }

            }
        }
    }

    if (mf || style.pathEffect()) {
        // The only mask filter the native rrect drawing code could've handle was taken
        // care of above.
        // A path effect will presumably transform this rrect into something else.
        SkPath path;
        path.setIsVolatile(true);
        path.addRRect(rrect);
        GrBlurUtils::drawPathWithMaskFilter(fContext.get(), fRenderTargetContext.get(),
                                            this->clip(), path, paint, this->ctm(), nullptr,
                                            this->devClipBounds(), true);
        return;
    }

    SkASSERT(!style.pathEffect());

    fRenderTargetContext->drawRRect(this->clip(), std::move(grPaint),
                                    GrBoolToAA(paint.isAntiAlias()), this->ctm(), rrect, style);
}


void SkGpuDevice::drawDRRect(const SkRRect& outer,
                             const SkRRect& inner, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawDRRect", fContext.get());
    CHECK_SHOULD_DRAW();

    if (outer.isEmpty()) {
       return;
    }

    if (inner.isEmpty()) {
        return this->drawRRect(outer, paint);
    }

    SkStrokeRec stroke(paint);

    if (stroke.isFillStyle() && !paint.getMaskFilter() && !paint.getPathEffect()) {
        GrPaint grPaint;
        if (!SkPaintToGrPaint(this->context(), fRenderTargetContext.get(), paint, this->ctm(),
                              &grPaint)) {
            return;
        }

        fRenderTargetContext->drawDRRect(this->clip(), std::move(grPaint),
                                         GrBoolToAA(paint.isAntiAlias()), this->ctm(), outer,
                                         inner);
        return;
    }

    SkPath path;
    path.setIsVolatile(true);
    path.addRRect(outer);
    path.addRRect(inner);
    path.setFillType(SkPath::kEvenOdd_FillType);

    GrBlurUtils::drawPathWithMaskFilter(fContext.get(), fRenderTargetContext.get(), this->clip(),
                                        path, paint, this->ctm(), nullptr, this->devClipBounds(),
                                        true);
}


/////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawRegion(const SkRegion& region, const SkPaint& paint) {
    if (paint.getMaskFilter()) {
        SkPath path;
        region.getBoundaryPath(&path);
        return this->drawPath(path, paint, nullptr, false);
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), fRenderTargetContext.get(), paint, this->ctm(),
                          &grPaint)) {
        return;
    }

    fRenderTargetContext->drawRegion(this->clip(), std::move(grPaint),
                                     GrBoolToAA(paint.isAntiAlias()), this->ctm(), region,
                                     GrStyle(paint));
}

void SkGpuDevice::drawOval(const SkRect& oval, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawOval", fContext.get());
    CHECK_SHOULD_DRAW();

    // Presumably the path effect warps this to something other than an oval
    if (paint.getPathEffect()) {
        SkPath path;
        path.setIsVolatile(true);
        path.addOval(oval);
        this->drawPath(path, paint, nullptr, true);
        return;
    }

    if (paint.getMaskFilter()) {
        // The RRect path can handle special case blurring
        SkRRect rr = SkRRect::MakeOval(oval);
        return this->drawRRect(rr, paint);
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), fRenderTargetContext.get(), paint, this->ctm(),
                          &grPaint)) {
        return;
    }

    fRenderTargetContext->drawOval(this->clip(), std::move(grPaint),
                                   GrBoolToAA(paint.isAntiAlias()), this->ctm(), oval,
                                   GrStyle(paint));
}

void SkGpuDevice::drawArc(const SkRect& oval, SkScalar startAngle,
                          SkScalar sweepAngle, bool useCenter, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawArc", fContext.get());
    CHECK_SHOULD_DRAW();

    if (paint.getMaskFilter()) {
        this->INHERITED::drawArc(oval, startAngle, sweepAngle, useCenter, paint);
        return;
    }
    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), fRenderTargetContext.get(), paint, this->ctm(),
                          &grPaint)) {
        return;
    }

    fRenderTargetContext->drawArc(this->clip(), std::move(grPaint), GrBoolToAA(paint.isAntiAlias()),
                                  this->ctm(), oval, startAngle, sweepAngle, useCenter,
                                  GrStyle(paint));
}

#include "SkMaskFilter.h"

///////////////////////////////////////////////////////////////////////////////
void SkGpuDevice::drawStrokedLine(const SkPoint points[2],
                                  const SkPaint& origPaint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawStrokedLine", fContext.get());
    CHECK_SHOULD_DRAW();

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

    m.postConcat(this->ctm());

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), fRenderTargetContext.get(), newPaint, m, &grPaint)) {
        return;
    }

    fRenderTargetContext->fillRectWithLocalMatrix(
            this->clip(), std::move(grPaint), GrBoolToAA(newPaint.isAntiAlias()), m, rect, local);
}

void SkGpuDevice::drawPath(const SkPath& origSrcPath,
                           const SkPaint& paint, const SkMatrix* prePathMatrix,
                           bool pathIsMutable) {
    ASSERT_SINGLE_OWNER
    if (!origSrcPath.isInverseFillType() && !paint.getPathEffect() && !prePathMatrix) {
        SkPoint points[2];
        if (SkPaint::kStroke_Style == paint.getStyle() && paint.getStrokeWidth() > 0 &&
            !paint.getMaskFilter() && SkPaint::kRound_Cap != paint.getStrokeCap() &&
            this->ctm().preservesRightAngles() && origSrcPath.isLine(points)) {
            // Path-based stroking looks better for thin rects
            SkScalar strokeWidth = this->ctm().getMaxScale() * paint.getStrokeWidth();
            if (strokeWidth >= 1.0f) {
                // Round capping support is currently disabled b.c. it would require a RRect
                // GrDrawOp that takes a localMatrix.
                this->drawStrokedLine(points, paint);
                return;
            }
        }
        bool isClosed;
        SkRect rect;
        if (origSrcPath.isRect(&rect, &isClosed) && isClosed) {
            this->drawRect(rect, paint);
            return;
        }
        if (origSrcPath.isOval(&rect)) {
            this->drawOval(rect, paint);
            return;
        }
        SkRRect rrect;
        if (origSrcPath.isRRect(&rrect)) {
            this->drawRRect(rrect, paint);
            return;
        }
    }

    CHECK_SHOULD_DRAW();
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawPath", fContext.get());

    GrBlurUtils::drawPathWithMaskFilter(fContext.get(), fRenderTargetContext.get(), this->clip(),
                                        origSrcPath, paint, this->ctm(), prePathMatrix,
                                        this->devClipBounds(), pathIsMutable);
}

static const int kBmpSmallTileSize = 1 << 10;

static inline int get_tile_count(const SkIRect& srcRect, int tileSize)  {
    int tilesX = (srcRect.fRight / tileSize) - (srcRect.fLeft / tileSize) + 1;
    int tilesY = (srcRect.fBottom / tileSize) - (srcRect.fTop / tileSize) + 1;
    return tilesX * tilesY;
}

static int determine_tile_size(const SkIRect& src, int maxTileSize) {
    if (maxTileSize <= kBmpSmallTileSize) {
        return maxTileSize;
    }

    size_t maxTileTotalTileSize = get_tile_count(src, maxTileSize);
    size_t smallTotalTileSize = get_tile_count(src, kBmpSmallTileSize);

    maxTileTotalTileSize *= maxTileSize * maxTileSize;
    smallTotalTileSize *= kBmpSmallTileSize * kBmpSmallTileSize;

    if (maxTileTotalTileSize > 2 * smallTotalTileSize) {
        return kBmpSmallTileSize;
    } else {
        return maxTileSize;
    }
}

// Given a bitmap, an optional src rect, and a context with a clip and matrix determine what
// pixels from the bitmap are necessary.
static void determine_clipped_src_rect(int width, int height,
                                       const GrClip& clip,
                                       const SkMatrix& viewMatrix,
                                       const SkMatrix& srcToDstRect,
                                       const SkISize& imageSize,
                                       const SkRect* srcRectPtr,
                                       SkIRect* clippedSrcIRect) {
    clip.getConservativeBounds(width, height, clippedSrcIRect, nullptr);
    SkMatrix inv = SkMatrix::Concat(viewMatrix, srcToDstRect);
    if (!inv.invert(&inv)) {
        clippedSrcIRect->setEmpty();
        return;
    }
    SkRect clippedSrcRect = SkRect::Make(*clippedSrcIRect);
    inv.mapRect(&clippedSrcRect);
    if (srcRectPtr) {
        if (!clippedSrcRect.intersect(*srcRectPtr)) {
            clippedSrcIRect->setEmpty();
            return;
        }
    }
    clippedSrcRect.roundOut(clippedSrcIRect);
    SkIRect bmpBounds = SkIRect::MakeSize(imageSize);
    if (!clippedSrcIRect->intersect(bmpBounds)) {
        clippedSrcIRect->setEmpty();
    }
}

bool SkGpuDevice::shouldTileImageID(uint32_t imageID, const SkIRect& imageRect,
                                    const SkMatrix& viewMatrix,
                                    const SkMatrix& srcToDstRect,
                                    const GrSamplerParams& params,
                                    const SkRect* srcRectPtr,
                                    int maxTileSize,
                                    int* tileSize,
                                    SkIRect* clippedSubset) const {
    ASSERT_SINGLE_OWNER
    // if it's larger than the max tile size, then we have no choice but tiling.
    if (imageRect.width() > maxTileSize || imageRect.height() > maxTileSize) {
        determine_clipped_src_rect(fRenderTargetContext->width(), fRenderTargetContext->height(),
                                   this->clip(), viewMatrix, srcToDstRect, imageRect.size(),
                                   srcRectPtr, clippedSubset);
        *tileSize = determine_tile_size(*clippedSubset, maxTileSize);
        return true;
    }

    // If the image would only produce 4 tiles of the smaller size, don't bother tiling it.
    const size_t area = imageRect.width() * imageRect.height();
    if (area < 4 * kBmpSmallTileSize * kBmpSmallTileSize) {
        return false;
    }

    // At this point we know we could do the draw by uploading the entire bitmap
    // as a texture. However, if the texture would be large compared to the
    // cache size and we don't require most of it for this draw then tile to
    // reduce the amount of upload and cache spill.

    // assumption here is that sw bitmap size is a good proxy for its size as
    // a texture
    size_t bmpSize = area * sizeof(SkPMColor);  // assume 32bit pixels
    size_t cacheSize;
    fContext->getResourceCacheLimits(nullptr, &cacheSize);
    if (bmpSize < cacheSize / 2) {
        return false;
    }

    // Figure out how much of the src we will need based on the src rect and clipping. Reject if
    // tiling memory savings would be < 50%.
    determine_clipped_src_rect(fRenderTargetContext->width(), fRenderTargetContext->height(),
                               this->clip(), viewMatrix, srcToDstRect, imageRect.size(), srcRectPtr,
                               clippedSubset);
    *tileSize = kBmpSmallTileSize; // already know whole bitmap fits in one max sized tile.
    size_t usedTileBytes = get_tile_count(*clippedSubset, kBmpSmallTileSize) *
                           kBmpSmallTileSize * kBmpSmallTileSize *
                           sizeof(SkPMColor);  // assume 32bit pixels;

    return usedTileBytes * 2 < bmpSize;
}

bool SkGpuDevice::shouldTileImage(const SkImage* image, const SkRect* srcRectPtr,
                                  SkCanvas::SrcRectConstraint constraint, SkFilterQuality quality,
                                  const SkMatrix& viewMatrix,
                                  const SkMatrix& srcToDstRect) const {
    ASSERT_SINGLE_OWNER
    // if image is explictly texture backed then just use the texture
    if (image->isTextureBacked()) {
        return false;
    }

    GrSamplerParams params;
    bool doBicubic;
    GrSamplerParams::FilterMode textureFilterMode =
                    GrSkFilterQualityToGrFilterMode(quality, viewMatrix, srcToDstRect, &doBicubic);

    int tileFilterPad;
    if (doBicubic) {
        tileFilterPad = GrBicubicEffect::kFilterTexelPad;
    } else if (GrSamplerParams::kNone_FilterMode == textureFilterMode) {
        tileFilterPad = 0;
    } else {
        tileFilterPad = 1;
    }
    params.setFilterMode(textureFilterMode);

    int maxTileSize = fContext->caps()->maxTileSize() - 2 * tileFilterPad;

    // these are output, which we safely ignore, as we just want to know the predicate
    int outTileSize;
    SkIRect outClippedSrcRect;

    return this->shouldTileImageID(image->unique(), image->bounds(), viewMatrix, srcToDstRect,
                                   params, srcRectPtr, maxTileSize, &outTileSize,
                                   &outClippedSrcRect);
}

void SkGpuDevice::drawBitmap(const SkBitmap& bitmap,
                             SkScalar x,
                             SkScalar y,
                             const SkPaint& paint) {
    SkMatrix m = SkMatrix::MakeTrans(x, y);
    ASSERT_SINGLE_OWNER
    CHECK_SHOULD_DRAW();
    SkMatrix viewMatrix;
    viewMatrix.setConcat(this->ctm(), m);

    int maxTileSize = fContext->caps()->maxTileSize();

    // The tile code path doesn't currently support AA, so if the paint asked for aa and we could
    // draw untiled, then we bypass checking for tiling purely for optimization reasons.
    bool drawAA = GrFSAAType::kUnifiedMSAA != fRenderTargetContext->fsaaType() &&
                  paint.isAntiAlias() && bitmap.width() <= maxTileSize &&
                  bitmap.height() <= maxTileSize;

    bool skipTileCheck = drawAA || paint.getMaskFilter();

    if (!skipTileCheck) {
        SkRect srcRect = SkRect::MakeIWH(bitmap.width(), bitmap.height());
        int tileSize;
        SkIRect clippedSrcRect;

        GrSamplerParams params;
        bool doBicubic;
        GrSamplerParams::FilterMode textureFilterMode =
            GrSkFilterQualityToGrFilterMode(paint.getFilterQuality(), viewMatrix, SkMatrix::I(),
                                            &doBicubic);

        int tileFilterPad;

        if (doBicubic) {
            tileFilterPad = GrBicubicEffect::kFilterTexelPad;
        } else if (GrSamplerParams::kNone_FilterMode == textureFilterMode) {
            tileFilterPad = 0;
        } else {
            tileFilterPad = 1;
        }
        params.setFilterMode(textureFilterMode);

        int maxTileSizeForFilter = fContext->caps()->maxTileSize() - 2 * tileFilterPad;
        if (this->shouldTileImageID(bitmap.getGenerationID(), bitmap.getSubset(), viewMatrix,
                                    SkMatrix::I(), params, &srcRect, maxTileSizeForFilter,
                                    &tileSize, &clippedSrcRect)) {
            this->drawTiledBitmap(bitmap, viewMatrix, SkMatrix::I(), srcRect, clippedSrcRect,
                                  params, paint, SkCanvas::kStrict_SrcRectConstraint, tileSize,
                                  doBicubic);
            return;
        }
    }
    GrBitmapTextureMaker maker(fContext.get(), bitmap);
    this->drawTextureProducer(&maker, nullptr, nullptr, SkCanvas::kStrict_SrcRectConstraint,
                              viewMatrix, this->clip(), paint);
}

// This method outsets 'iRect' by 'outset' all around and then clamps its extents to
// 'clamp'. 'offset' is adjusted to remain positioned over the top-left corner
// of 'iRect' for all possible outsets/clamps.
static inline void clamped_outset_with_offset(SkIRect* iRect,
                                              int outset,
                                              SkPoint* offset,
                                              const SkIRect& clamp) {
    iRect->outset(outset, outset);

    int leftClampDelta = clamp.fLeft - iRect->fLeft;
    if (leftClampDelta > 0) {
        offset->fX -= outset - leftClampDelta;
        iRect->fLeft = clamp.fLeft;
    } else {
        offset->fX -= outset;
    }

    int topClampDelta = clamp.fTop - iRect->fTop;
    if (topClampDelta > 0) {
        offset->fY -= outset - topClampDelta;
        iRect->fTop = clamp.fTop;
    } else {
        offset->fY -= outset;
    }

    if (iRect->fRight > clamp.fRight) {
        iRect->fRight = clamp.fRight;
    }
    if (iRect->fBottom > clamp.fBottom) {
        iRect->fBottom = clamp.fBottom;
    }
}

// Break 'bitmap' into several tiles to draw it since it has already
// been determined to be too large to fit in VRAM
void SkGpuDevice::drawTiledBitmap(const SkBitmap& bitmap,
                                  const SkMatrix& viewMatrix,
                                  const SkMatrix& dstMatrix,
                                  const SkRect& srcRect,
                                  const SkIRect& clippedSrcIRect,
                                  const GrSamplerParams& params,
                                  const SkPaint& origPaint,
                                  SkCanvas::SrcRectConstraint constraint,
                                  int tileSize,
                                  bool bicubic) {
    ASSERT_SINGLE_OWNER

    // This is the funnel for all paths that draw tiled bitmaps/images. Log histogram entries.
    SK_HISTOGRAM_BOOLEAN("DrawTiled", true);
    LogDrawScaleFactor(viewMatrix, origPaint.getFilterQuality());

    const SkPaint* paint = &origPaint;
    SkPaint tempPaint;
    if (origPaint.isAntiAlias() && GrFSAAType::kUnifiedMSAA != fRenderTargetContext->fsaaType()) {
        // Drop antialiasing to avoid seams at tile boundaries.
        tempPaint = origPaint;
        tempPaint.setAntiAlias(false);
        paint = &tempPaint;
    }
    SkRect clippedSrcRect = SkRect::Make(clippedSrcIRect);

    int nx = bitmap.width() / tileSize;
    int ny = bitmap.height() / tileSize;
    for (int x = 0; x <= nx; x++) {
        for (int y = 0; y <= ny; y++) {
            SkRect tileR;
            tileR.set(SkIntToScalar(x * tileSize),
                      SkIntToScalar(y * tileSize),
                      SkIntToScalar((x + 1) * tileSize),
                      SkIntToScalar((y + 1) * tileSize));

            if (!SkRect::Intersects(tileR, clippedSrcRect)) {
                continue;
            }

            if (!tileR.intersect(srcRect)) {
                continue;
            }

            SkIRect iTileR;
            tileR.roundOut(&iTileR);
            SkVector offset = SkPoint::Make(SkIntToScalar(iTileR.fLeft),
                                            SkIntToScalar(iTileR.fTop));
            SkRect rectToDraw = tileR;
            dstMatrix.mapRect(&rectToDraw);
            if (GrSamplerParams::kNone_FilterMode != params.filterMode() || bicubic) {
                SkIRect iClampRect;

                if (SkCanvas::kFast_SrcRectConstraint == constraint) {
                    // In bleed mode we want to always expand the tile on all edges
                    // but stay within the bitmap bounds
                    iClampRect = SkIRect::MakeWH(bitmap.width(), bitmap.height());
                } else {
                    // In texture-domain/clamp mode we only want to expand the
                    // tile on edges interior to "srcRect" (i.e., we want to
                    // not bleed across the original clamped edges)
                    srcRect.roundOut(&iClampRect);
                }
                int outset = bicubic ? GrBicubicEffect::kFilterTexelPad : 1;
                clamped_outset_with_offset(&iTileR, outset, &offset, iClampRect);
            }

            SkBitmap tmpB;
            if (bitmap.extractSubset(&tmpB, iTileR)) {
                // now offset it to make it "local" to our tmp bitmap
                tileR.offset(-offset.fX, -offset.fY);
                // de-optimized this determination
                bool needsTextureDomain = true;
                this->drawBitmapTile(tmpB,
                                     viewMatrix,
                                     rectToDraw,
                                     tileR,
                                     params,
                                     *paint,
                                     constraint,
                                     bicubic,
                                     needsTextureDomain);
            }
        }
    }
}

void SkGpuDevice::drawBitmapTile(const SkBitmap& bitmap,
                                 const SkMatrix& viewMatrix,
                                 const SkRect& dstRect,
                                 const SkRect& srcRect,
                                 const GrSamplerParams& params,
                                 const SkPaint& paint,
                                 SkCanvas::SrcRectConstraint constraint,
                                 bool bicubic,
                                 bool needsTextureDomain) {
    // We should have already handled bitmaps larger than the max texture size.
    SkASSERT(bitmap.width() <= fContext->caps()->maxTextureSize() &&
             bitmap.height() <= fContext->caps()->maxTextureSize());
    // We should be respecting the max tile size by the time we get here.
    SkASSERT(bitmap.width() <= fContext->caps()->maxTileSize() &&
             bitmap.height() <= fContext->caps()->maxTileSize());

    SkASSERT(SkShader::kClamp_TileMode == params.getTileModeX() &&
             SkShader::kClamp_TileMode == params.getTileModeY());

    sk_sp<GrTextureProxy> proxy = GrRefCachedBitmapTextureProxy(fContext.get(), bitmap,
                                                                params, nullptr);
    if (!proxy) {
        return;
    }
    sk_sp<GrColorSpaceXform> colorSpaceXform =
        GrColorSpaceXform::Make(bitmap.colorSpace(), fRenderTargetContext->getColorSpace());

    // Compute a matrix that maps the rect we will draw to the src rect.
    const SkMatrix texMatrix = SkMatrix::MakeRectToRect(dstRect, srcRect,
                                                        SkMatrix::kFill_ScaleToFit);

    // Construct a GrPaint by setting the bitmap texture as the first effect and then configuring
    // the rest from the SkPaint.
    sk_sp<GrFragmentProcessor> fp;

    if (needsTextureDomain && (SkCanvas::kStrict_SrcRectConstraint == constraint)) {
        // Use a constrained texture domain to avoid color bleeding
        SkRect domain;
        if (srcRect.width() > SK_Scalar1) {
            domain.fLeft  = srcRect.fLeft + 0.5f;
            domain.fRight = srcRect.fRight - 0.5f;
        } else {
            domain.fLeft = domain.fRight = srcRect.centerX();
        }
        if (srcRect.height() > SK_Scalar1) {
            domain.fTop  = srcRect.fTop + 0.5f;
            domain.fBottom = srcRect.fBottom - 0.5f;
        } else {
            domain.fTop = domain.fBottom = srcRect.centerY();
        }
        if (bicubic) {
            fp = GrBicubicEffect::Make(std::move(proxy),
                                       std::move(colorSpaceXform), texMatrix, domain);
        } else {
            fp = GrTextureDomainEffect::Make(std::move(proxy),
                                             std::move(colorSpaceXform), texMatrix,
                                             domain, GrTextureDomain::kClamp_Mode,
                                             params.filterMode());
        }
    } else if (bicubic) {
        SkASSERT(GrSamplerParams::kNone_FilterMode == params.filterMode());
        SkShader::TileMode tileModes[2] = { params.getTileModeX(), params.getTileModeY() };
        fp = GrBicubicEffect::Make(std::move(proxy),
                                   std::move(colorSpaceXform), texMatrix, tileModes);
    } else {
        fp = GrSimpleTextureEffect::Make(std::move(proxy),
                                         std::move(colorSpaceXform), texMatrix, params);
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaintWithTexture(this->context(), fRenderTargetContext.get(), paint, viewMatrix,
                                     std::move(fp), kAlpha_8_SkColorType == bitmap.colorType(),
                                     &grPaint)) {
        return;
    }

    // Coverage-based AA would cause seams between tiles.
    GrAA aa = GrBoolToAA(paint.isAntiAlias() &&
                         GrFSAAType::kNone != fRenderTargetContext->fsaaType());
    fRenderTargetContext->drawRect(this->clip(), std::move(grPaint), aa, viewMatrix, dstRect);
}

void SkGpuDevice::drawSprite(const SkBitmap& bitmap,
                             int left, int top, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    CHECK_SHOULD_DRAW();
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawSprite", fContext.get());

    if (fContext->abandoned()) {
        return;
    }

    sk_sp<SkSpecialImage> srcImg = this->makeSpecial(bitmap);
    if (!srcImg) {
        return;
    }

    this->drawSpecial(srcImg.get(), left, top, paint, nullptr, SkMatrix::I());
}


void SkGpuDevice::drawSpecial(SkSpecialImage* special1, int left, int top, const SkPaint& paint,
                              SkImage* clipImage,const SkMatrix& clipMatrix) {
    ASSERT_SINGLE_OWNER
    CHECK_SHOULD_DRAW();
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawSpecial", fContext.get());

    // TODO: clipImage support.

    SkIPoint offset = { 0, 0 };

    sk_sp<SkSpecialImage> result;
    if (paint.getImageFilter()) {
        result = this->filterTexture(special1, left, top,
                                      &offset,
                                      paint.getImageFilter());
        if (!result) {
            return;
        }
    } else {
        result = sk_ref_sp(special1);
    }

    SkASSERT(result->isTextureBacked());
    sk_sp<GrTextureProxy> proxy = result->asTextureProxyRef(this->context());
    if (!proxy) {
        return;
    }

    const GrPixelConfig config = proxy->config();

    SkPaint tmpUnfiltered(paint);
    tmpUnfiltered.setImageFilter(nullptr);

    sk_sp<GrColorSpaceXform> colorSpaceXform =
        GrColorSpaceXform::Make(result->getColorSpace(), fRenderTargetContext->getColorSpace());

    sk_sp<GrFragmentProcessor> fp(GrSimpleTextureEffect::Make(std::move(proxy),
                                                              std::move(colorSpaceXform),
                                                              SkMatrix::I()));
    if (GrPixelConfigIsAlphaOnly(config)) {
        fp = GrFragmentProcessor::MakeInputPremulAndMulByOutput(std::move(fp));
    } else {
        fp = GrFragmentProcessor::MulOutputByInputAlpha(std::move(fp));
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaintReplaceShader(this->context(), fRenderTargetContext.get(), tmpUnfiltered,
                                       std::move(fp), &grPaint)) {
        return;
    }

    const SkIRect& subset = result->subset();

    fRenderTargetContext->fillRectToRect(
            this->clip(),
            std::move(grPaint),
            GrBoolToAA(paint.isAntiAlias()),
            SkMatrix::I(),
            SkRect::Make(SkIRect::MakeXYWH(left + offset.fX, top + offset.fY, subset.width(),
                                           subset.height())),
            SkRect::Make(subset));
}

void SkGpuDevice::drawBitmapRect(const SkBitmap& bitmap,
                                 const SkRect* src, const SkRect& origDst,
                                 const SkPaint& paint, SkCanvas::SrcRectConstraint constraint) {
    ASSERT_SINGLE_OWNER
    CHECK_SHOULD_DRAW();

    // The src rect is inferred to be the bmp bounds if not provided. Otherwise, the src rect must
    // be clipped to the bmp bounds. To determine tiling parameters we need the filter mode which
    // in turn requires knowing the src-to-dst mapping. If the src was clipped to the bmp bounds
    // then we use the src-to-dst mapping to compute a new clipped dst rect.
    const SkRect* dst = &origDst;
    const SkRect bmpBounds = SkRect::MakeIWH(bitmap.width(), bitmap.height());
    // Compute matrix from the two rectangles
    if (!src) {
        src = &bmpBounds;
    }

    SkMatrix srcToDstMatrix;
    if (!srcToDstMatrix.setRectToRect(*src, *dst, SkMatrix::kFill_ScaleToFit)) {
        return;
    }
    SkRect tmpSrc, tmpDst;
    if (src != &bmpBounds) {
        if (!bmpBounds.contains(*src)) {
            tmpSrc = *src;
            if (!tmpSrc.intersect(bmpBounds)) {
                return; // nothing to draw
            }
            src = &tmpSrc;
            srcToDstMatrix.mapRect(&tmpDst, *src);
            dst = &tmpDst;
        }
    }

    int maxTileSize = fContext->caps()->maxTileSize();

    // The tile code path doesn't currently support AA, so if the paint asked for aa and we could
    // draw untiled, then we bypass checking for tiling purely for optimization reasons.
    bool useCoverageAA = GrFSAAType::kUnifiedMSAA != fRenderTargetContext->fsaaType() &&
                         paint.isAntiAlias() && bitmap.width() <= maxTileSize &&
                         bitmap.height() <= maxTileSize;

    bool skipTileCheck = useCoverageAA || paint.getMaskFilter();

    if (!skipTileCheck) {
        int tileSize;
        SkIRect clippedSrcRect;

        GrSamplerParams params;
        bool doBicubic;
        GrSamplerParams::FilterMode textureFilterMode =
            GrSkFilterQualityToGrFilterMode(paint.getFilterQuality(), this->ctm(), srcToDstMatrix,
                                            &doBicubic);

        int tileFilterPad;

        if (doBicubic) {
            tileFilterPad = GrBicubicEffect::kFilterTexelPad;
        } else if (GrSamplerParams::kNone_FilterMode == textureFilterMode) {
            tileFilterPad = 0;
        } else {
            tileFilterPad = 1;
        }
        params.setFilterMode(textureFilterMode);

        int maxTileSizeForFilter = fContext->caps()->maxTileSize() - 2 * tileFilterPad;
        if (this->shouldTileImageID(bitmap.getGenerationID(), bitmap.getSubset(), this->ctm(),
                                    srcToDstMatrix, params, src, maxTileSizeForFilter, &tileSize,
                                    &clippedSrcRect)) {
            this->drawTiledBitmap(bitmap, this->ctm(), srcToDstMatrix, *src, clippedSrcRect,
                                  params, paint, constraint, tileSize, doBicubic);
            return;
        }
    }
    GrBitmapTextureMaker maker(fContext.get(), bitmap);
    this->drawTextureProducer(&maker, src, dst, constraint, this->ctm(), this->clip(), paint);
}

sk_sp<SkSpecialImage> SkGpuDevice::makeSpecial(const SkBitmap& bitmap) {
    // TODO: this makes a tight copy of 'bitmap' but it doesn't have to be (given SkSpecialImage's
    // semantics). Since this is cached we would have to bake the fit into the cache key though.
    sk_sp<GrTextureProxy> proxy = GrMakeCachedBitmapProxy(fContext->resourceProvider(), bitmap);
    if (!proxy) {
        return nullptr;
    }

    const SkIRect rect = SkIRect::MakeWH(proxy->width(), proxy->height());

    // GrMakeCachedBitmapProxy creates a tight copy of 'bitmap' so we don't have to subset
    // the special image
    return SkSpecialImage::MakeDeferredFromGpu(fContext.get(),
                                               rect,
                                               bitmap.getGenerationID(),
                                               std::move(proxy),
                                               bitmap.refColorSpace(),
                                               &this->surfaceProps());
}

sk_sp<SkSpecialImage> SkGpuDevice::makeSpecial(const SkImage* image) {
    SkPixmap pm;
    if (image->isTextureBacked()) {
        sk_sp<GrTextureProxy> proxy = as_IB(image)->asTextureProxyRef();

        return SkSpecialImage::MakeDeferredFromGpu(fContext.get(),
                                                   SkIRect::MakeWH(image->width(), image->height()),
                                                   image->uniqueID(),
                                                   std::move(proxy),
                                                   as_IB(image)->onImageInfo().refColorSpace(),
                                                   &this->surfaceProps());
    } else if (image->peekPixels(&pm)) {
        SkBitmap bm;

        bm.installPixels(pm);
        return this->makeSpecial(bm);
    } else {
        return nullptr;
    }
}

sk_sp<SkSpecialImage> SkGpuDevice::snapSpecial() {
    sk_sp<GrTextureProxy> proxy(this->accessRenderTargetContext()->asTextureProxyRef());
    if (!proxy) {
        // When the device doesn't have a texture, we create a temporary texture.
        // TODO: we should actually only copy the portion of the source needed to apply the image
        // filter
        proxy = GrSurfaceProxy::Copy(fContext.get(),
                                     this->accessRenderTargetContext()->asSurfaceProxy(),
                                     SkBudgeted::kYes);
        if (!proxy) {
            return nullptr;
        }
    }

    const SkImageInfo ii = this->imageInfo();
    const SkIRect srcRect = SkIRect::MakeWH(ii.width(), ii.height());

    return SkSpecialImage::MakeDeferredFromGpu(fContext.get(),
                                               srcRect,
                                               kNeedNewImageUniqueID_SpecialImage,
                                               std::move(proxy),
                                               ii.refColorSpace(),
                                               &this->surfaceProps());
}

void SkGpuDevice::drawDevice(SkBaseDevice* device,
                             int left, int top, const SkPaint& paint) {
    SkASSERT(!paint.getImageFilter());

    ASSERT_SINGLE_OWNER
    // clear of the source device must occur before CHECK_SHOULD_DRAW
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawDevice", fContext.get());

    // drawDevice is defined to be in device coords.
    CHECK_SHOULD_DRAW();

    SkGpuDevice* dev = static_cast<SkGpuDevice*>(device);
    sk_sp<SkSpecialImage> srcImg(dev->snapSpecial());
    if (!srcImg) {
        return;
    }

    this->drawSpecial(srcImg.get(), left, top, paint, nullptr, SkMatrix::I());
}

void SkGpuDevice::drawImage(const SkImage* image, SkScalar x, SkScalar y,
                            const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    SkMatrix viewMatrix = this->ctm();
    viewMatrix.preTranslate(x, y);
    uint32_t pinnedUniqueID;

    if (sk_sp<GrTextureProxy> proxy = as_IB(image)->refPinnedTextureProxy(&pinnedUniqueID)) {
        CHECK_SHOULD_DRAW();
        GrTextureAdjuster adjuster(this->context(), std::move(proxy),
                                   image->alphaType(), image->bounds(),
                                   pinnedUniqueID, as_IB(image)->onImageInfo().colorSpace());
        this->drawTextureProducer(&adjuster, nullptr, nullptr, SkCanvas::kFast_SrcRectConstraint,
                                  viewMatrix, this->clip(), paint);
        return;
    } else {
        SkBitmap bm;
        if (this->shouldTileImage(image, nullptr, SkCanvas::kFast_SrcRectConstraint,
                                  paint.getFilterQuality(), this->ctm(), SkMatrix::I())) {
            // only support tiling as bitmap at the moment, so force raster-version
            if (!as_IB(image)->getROPixels(&bm, fRenderTargetContext->getColorSpace())) {
                return;
            }
            this->drawBitmap(bm, x, y, paint);
        } else if (image->isLazyGenerated()) {
            CHECK_SHOULD_DRAW();
            GrImageTextureMaker maker(fContext.get(), image, SkImage::kAllow_CachingHint);
            this->drawTextureProducer(&maker, nullptr, nullptr, SkCanvas::kFast_SrcRectConstraint,
                                      viewMatrix, this->clip(), paint);
        } else if (as_IB(image)->getROPixels(&bm, fRenderTargetContext->getColorSpace())) {
            this->drawBitmap(bm, x, y, paint);
        }
    }
}

void SkGpuDevice::drawImageRect(const SkImage* image, const SkRect* src,
                                const SkRect& dst, const SkPaint& paint,
                                SkCanvas::SrcRectConstraint constraint) {
    ASSERT_SINGLE_OWNER
    uint32_t pinnedUniqueID;
    if (sk_sp<GrTextureProxy> proxy = as_IB(image)->refPinnedTextureProxy(&pinnedUniqueID)) {
        CHECK_SHOULD_DRAW();
        GrTextureAdjuster adjuster(this->context(), std::move(proxy),
                                   image->alphaType(), image->bounds(), pinnedUniqueID,
                                   as_IB(image)->onImageInfo().colorSpace());
        this->drawTextureProducer(&adjuster, src, &dst, constraint, this->ctm(), this->clip(),
                                  paint);
        return;
    }
    SkBitmap bm;
    SkMatrix srcToDstRect;
    srcToDstRect.setRectToRect((src ? *src : SkRect::MakeIWH(image->width(), image->height())),
                               dst, SkMatrix::kFill_ScaleToFit);
    if (this->shouldTileImage(image, src, constraint, paint.getFilterQuality(), this->ctm(),
                              srcToDstRect)) {
        // only support tiling as bitmap at the moment, so force raster-version
        if (!as_IB(image)->getROPixels(&bm, fRenderTargetContext->getColorSpace())) {
            return;
        }
        this->drawBitmapRect(bm, src, dst, paint, constraint);
    } else if (image->isLazyGenerated()) {
        CHECK_SHOULD_DRAW();
        GrImageTextureMaker maker(fContext.get(), image, SkImage::kAllow_CachingHint);
        this->drawTextureProducer(&maker, src, &dst, constraint, this->ctm(), this->clip(), paint);
    } else if (as_IB(image)->getROPixels(&bm, fRenderTargetContext->getColorSpace())) {
        this->drawBitmapRect(bm, src, dst, paint, constraint);
    }
}

void SkGpuDevice::drawProducerNine(GrTextureProducer* producer,
                                   const SkIRect& center, const SkRect& dst, const SkPaint& paint) {
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawProducerNine", fContext.get());

    CHECK_SHOULD_DRAW();

    bool useFallback = paint.getMaskFilter() || paint.isAntiAlias() ||
                       GrFSAAType::kUnifiedMSAA == fRenderTargetContext->fsaaType();
    bool doBicubic;
    GrSamplerParams::FilterMode textureFilterMode =
        GrSkFilterQualityToGrFilterMode(paint.getFilterQuality(), this->ctm(), SkMatrix::I(),
                                        &doBicubic);
    if (useFallback || doBicubic || GrSamplerParams::kNone_FilterMode != textureFilterMode) {
        SkLatticeIter iter(producer->width(), producer->height(), center, dst);

        SkRect srcR, dstR;
        while (iter.next(&srcR, &dstR)) {
            this->drawTextureProducer(producer, &srcR, &dstR, SkCanvas::kStrict_SrcRectConstraint,
                                      this->ctm(), this->clip(), paint);
        }
        return;
    }

    static const GrSamplerParams::FilterMode kMode = GrSamplerParams::kNone_FilterMode;
    sk_sp<GrFragmentProcessor> fp(
        producer->createFragmentProcessor(SkMatrix::I(),
                                          SkRect::MakeIWH(producer->width(), producer->height()),
                                          GrTextureProducer::kNo_FilterConstraint, true,
                                          &kMode, fRenderTargetContext->getColorSpace()));
    if (!fp) {
        return;
    }
    GrPaint grPaint;
    if (!SkPaintToGrPaintWithTexture(this->context(), fRenderTargetContext.get(), paint,
                                     this->ctm(), std::move(fp), producer->isAlphaOnly(),
                                     &grPaint)) {
        return;
    }

    std::unique_ptr<SkLatticeIter> iter(
            new SkLatticeIter(producer->width(), producer->height(), center, dst));
    fRenderTargetContext->drawImageLattice(this->clip(), std::move(grPaint), this->ctm(),
                                           producer->width(), producer->height(), std::move(iter),
                                           dst);
}

void SkGpuDevice::drawImageNine(const SkImage* image,
                                const SkIRect& center, const SkRect& dst, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    uint32_t pinnedUniqueID;
    if (sk_sp<GrTextureProxy> proxy = as_IB(image)->refPinnedTextureProxy(&pinnedUniqueID)) {
        CHECK_SHOULD_DRAW();
        GrTextureAdjuster adjuster(this->context(), std::move(proxy),
                                   image->alphaType(), image->bounds(),
                                   pinnedUniqueID, as_IB(image)->onImageInfo().colorSpace());
        this->drawProducerNine(&adjuster, center, dst, paint);
    } else {
        SkBitmap bm;
        if (image->isLazyGenerated()) {
            GrImageTextureMaker maker(fContext.get(), image, SkImage::kAllow_CachingHint);
            this->drawProducerNine(&maker, center, dst, paint);
        } else if (as_IB(image)->getROPixels(&bm, fRenderTargetContext->getColorSpace())) {
            this->drawBitmapNine(bm, center, dst, paint);
        }
    }
}

void SkGpuDevice::drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                 const SkRect& dst, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GrBitmapTextureMaker maker(fContext.get(), bitmap);
    this->drawProducerNine(&maker, center, dst, paint);
}

void SkGpuDevice::drawProducerLattice(GrTextureProducer* producer,
                                      const SkCanvas::Lattice& lattice, const SkRect& dst,
                                      const SkPaint& paint) {
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawProducerLattice", fContext.get());

    CHECK_SHOULD_DRAW();

    static const GrSamplerParams::FilterMode kMode = GrSamplerParams::kNone_FilterMode;
    sk_sp<GrFragmentProcessor> fp(
        producer->createFragmentProcessor(SkMatrix::I(),
                                          SkRect::MakeIWH(producer->width(), producer->height()),
                                          GrTextureProducer::kNo_FilterConstraint, true,
                                          &kMode, fRenderTargetContext->getColorSpace()));
    if (!fp) {
        return;
    }
    GrPaint grPaint;
    if (!SkPaintToGrPaintWithTexture(this->context(), fRenderTargetContext.get(), paint,
                                     this->ctm(), std::move(fp), producer->isAlphaOnly(),
                                     &grPaint)) {
        return;
    }

    std::unique_ptr<SkLatticeIter> iter(
            new SkLatticeIter(lattice, dst));
    fRenderTargetContext->drawImageLattice(this->clip(), std::move(grPaint), this->ctm(),
                                           producer->width(), producer->height(), std::move(iter),
                                           dst);
}

void SkGpuDevice::drawImageLattice(const SkImage* image,
                                   const SkCanvas::Lattice& lattice, const SkRect& dst,
                                   const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    uint32_t pinnedUniqueID;
    if (sk_sp<GrTextureProxy> proxy = as_IB(image)->refPinnedTextureProxy(&pinnedUniqueID)) {
        CHECK_SHOULD_DRAW();
        GrTextureAdjuster adjuster(this->context(), std::move(proxy),
                                   image->alphaType(), image->bounds(),
                                   pinnedUniqueID, as_IB(image)->onImageInfo().colorSpace());
        this->drawProducerLattice(&adjuster, lattice, dst, paint);
    } else {
        SkBitmap bm;
        if (image->isLazyGenerated()) {
            GrImageTextureMaker maker(fContext.get(), image, SkImage::kAllow_CachingHint);
            this->drawProducerLattice(&maker, lattice, dst, paint);
        } else if (as_IB(image)->getROPixels(&bm, fRenderTargetContext->getColorSpace())) {
            this->drawBitmapLattice(bm, lattice, dst, paint);
        }
    }
}

void SkGpuDevice::drawBitmapLattice(const SkBitmap& bitmap,
                                    const SkCanvas::Lattice& lattice, const SkRect& dst,
                                    const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GrBitmapTextureMaker maker(fContext.get(), bitmap);
    this->drawProducerLattice(&maker, lattice, dst, paint);
}

static bool init_vertices_paint(GrContext* context, GrRenderTargetContext* rtc,
                                const SkPaint& skPaint,
                                const SkMatrix& matrix, SkBlendMode bmode,
                                bool hasTexs, bool hasColors, GrPaint* grPaint) {
    if (hasTexs && skPaint.getShader()) {
        if (hasColors) {
            // When there are texs and colors the shader and colors are combined using bmode.
            return SkPaintToGrPaintWithXfermode(context, rtc, skPaint, matrix, bmode, grPaint);
        } else {
            // We have a shader, but no colors to blend it against.
            return SkPaintToGrPaint(context, rtc, skPaint, matrix, grPaint);
        }
    } else {
        if (hasColors) {
            // We have colors, but either have no shader or no texture coords (which implies that
            // we should ignore the shader).
            return SkPaintToGrPaintWithPrimitiveColor(context, rtc, skPaint, grPaint);
        } else {
            // No colors and no shaders. Just draw with the paint color.
            return SkPaintToGrPaintNoShader(context, rtc, skPaint, grPaint);
        }
    }
}

void SkGpuDevice::wireframeVertices(SkVertices::VertexMode vmode, int vertexCount,
                                    const SkPoint vertices[], SkBlendMode bmode,
                                    const uint16_t indices[], int indexCount,
                                    const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    CHECK_SHOULD_DRAW();
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "wireframeVertices", fContext.get());

    SkPaint copy(paint);
    copy.setStyle(SkPaint::kStroke_Style);
    copy.setStrokeWidth(0);

    GrPaint grPaint;
    // we ignore the shader since we have no texture coordinates.
    if (!SkPaintToGrPaintNoShader(this->context(), fRenderTargetContext.get(), copy, &grPaint)) {
        return;
    }

    int triangleCount = 0;
    int n = (nullptr == indices) ? vertexCount : indexCount;
    switch (vmode) {
        case SkVertices::kTriangles_VertexMode:
            triangleCount = n / 3;
            break;
        case SkVertices::kTriangleStrip_VertexMode:
        case SkVertices::kTriangleFan_VertexMode:
            triangleCount = n - 2;
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
                                       this->ctm(),
                                       builder.detach(),
                                       &primitiveType);
}

void SkGpuDevice::drawVertices(const SkVertices* vertices, SkBlendMode mode, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    CHECK_SHOULD_DRAW();
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawVertices", fContext.get());

    SkASSERT(vertices);
    GrPaint grPaint;
    bool hasColors = vertices->hasColors();
    bool hasTexs = vertices->hasTexCoords();
    if ((!hasTexs || !paint.getShader()) && !hasColors) {
        // The dreaded wireframe mode. Fallback to drawVertices and go so slooooooow.
        this->wireframeVertices(vertices->mode(), vertices->vertexCount(), vertices->positions(),
                                mode, vertices->indices(), vertices->indexCount(), paint);
        return;
    }
    if (!init_vertices_paint(fContext.get(), fRenderTargetContext.get(), paint, this->ctm(),
                             mode, hasTexs, hasColors, &grPaint)) {
        return;
    }
    fRenderTargetContext->drawVertices(this->clip(), std::move(grPaint), this->ctm(),
                                       sk_ref_sp(const_cast<SkVertices*>(vertices)));
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawShadow(const SkPath& path, const SkDrawShadowRec& rec) {

    ASSERT_SINGLE_OWNER
    CHECK_SHOULD_DRAW();
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawShadow", fContext.get());

    GrColor color = SkColorToPremulGrColor(rec.fColor);
    if (!fRenderTargetContext->drawFastShadow(this->clip(), color, this->ctm(), path, rec)) {
        // failed to find an accelerated case
        this->INHERITED::drawShadow(path, rec);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawAtlas(const SkImage* atlas, const SkRSXform xform[],
                            const SkRect texRect[], const SkColor colors[], int count,
                            SkBlendMode mode, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    if (paint.isAntiAlias()) {
        this->INHERITED::drawAtlas(atlas, xform, texRect, colors, count, mode, paint);
        return;
    }

    CHECK_SHOULD_DRAW();
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawText", fContext.get());

    SkPaint p(paint);
    p.setShader(atlas->makeShader());

    GrPaint grPaint;
    if (colors) {
        if (!SkPaintToGrPaintWithXfermode(this->context(), fRenderTargetContext.get(), p,
                                          this->ctm(), (SkBlendMode)mode, &grPaint)) {
            return;
        }
    } else {
        if (!SkPaintToGrPaint(this->context(), fRenderTargetContext.get(), p, this->ctm(),
                              &grPaint)) {
            return;
        }
    }

    SkDEBUGCODE(this->validate();)
    fRenderTargetContext->drawAtlas(
            this->clip(), std::move(grPaint), this->ctm(), count, xform, texRect, colors);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawText(const void* text,
                           size_t byteLength, SkScalar x, SkScalar y,
                           const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    CHECK_SHOULD_DRAW();
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawText", fContext.get());
    SkDEBUGCODE(this->validate();)

    fRenderTargetContext->drawText(this->clip(), paint, this->ctm(), (const char*)text, byteLength,
                                   x, y, this->devClipBounds());
}

void SkGpuDevice::drawPosText(const void* text, size_t byteLength,
                              const SkScalar pos[], int scalarsPerPos,
                              const SkPoint& offset, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawPosText", fContext.get());
    CHECK_SHOULD_DRAW();
    SkDEBUGCODE(this->validate();)

    fRenderTargetContext->drawPosText(this->clip(), paint, this->ctm(), (const char*)text,
                                      byteLength, pos, scalarsPerPos, offset,
                                      this->devClipBounds());
}

void SkGpuDevice::drawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                               const SkPaint& paint, SkDrawFilter* drawFilter) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawTextBlob", fContext.get());
    CHECK_SHOULD_DRAW();

    SkDEBUGCODE(this->validate();)

    fRenderTargetContext->drawTextBlob(this->clip(), paint, this->ctm(), blob, x, y, drawFilter,
                                       this->devClipBounds());
}

///////////////////////////////////////////////////////////////////////////////

bool SkGpuDevice::onShouldDisableLCD(const SkPaint& paint) const {
    return GrTextUtils::ShouldDisableLCD(paint);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::flush() {
    this->flushAndSignalSemaphores(0, nullptr);
}

bool SkGpuDevice::flushAndSignalSemaphores(int numSemaphores,
                                           GrBackendSemaphore* signalSemaphores) {
    ASSERT_SINGLE_OWNER

    return fRenderTargetContext->prepareForExternalIO(numSemaphores, signalSemaphores);
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

    sk_sp<GrRenderTargetContext> rtc(fContext->makeDeferredRenderTargetContext(
                                                   fit,
                                                   cinfo.fInfo.width(), cinfo.fInfo.height(),
                                                   fRenderTargetContext->config(),
                                                   fRenderTargetContext->refColorSpace(),
                                                   fRenderTargetContext->numStencilSamples(),
                                                   kBottomLeft_GrSurfaceOrigin,
                                                   &props));
    if (!rtc) {
        return nullptr;
    }

    // Skia's convention is to only clear a device if it is non-opaque.
    InitContents init = cinfo.fInfo.isOpaque() ? kUninit_InitContents : kClear_InitContents;

    return SkGpuDevice::Make(fContext.get(), std::move(rtc),
                             cinfo.fInfo.width(), cinfo.fInfo.height(), init).release();
}

sk_sp<SkSurface> SkGpuDevice::makeSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    ASSERT_SINGLE_OWNER
    // TODO: Change the signature of newSurface to take a budgeted parameter.
    static const SkBudgeted kBudgeted = SkBudgeted::kNo;
    return SkSurface::MakeRenderTarget(fContext.get(), kBudgeted, info,
                                       fRenderTargetContext->numStencilSamples(),
                                       fRenderTargetContext->origin(), &props);
}

SkImageFilterCache* SkGpuDevice::getImageFilterCache() {
    ASSERT_SINGLE_OWNER
    // We always return a transient cache, so it is freed after each
    // filter traversal.
    return SkImageFilterCache::Create(SkImageFilterCache::kDefaultTransientSize);
}

#endif
