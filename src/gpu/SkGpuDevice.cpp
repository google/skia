/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGpuDevice.h"

#include "GrBlurUtils.h"
#include "GrContext.h"
#include "SkDraw.h"
#include "GrGpu.h"
#include "GrGpuResourcePriv.h"
#include "GrImageIDTextureAdjuster.h"
#include "GrLayerHoister.h"
#include "GrRecordReplaceDraw.h"
#include "GrStrokeInfo.h"
#include "GrTracing.h"
#include "SkCanvasPriv.h"
#include "SkErrorInternals.h"
#include "SkGlyphCache.h"
#include "SkGrTexturePixelRef.h"
#include "SkGr.h"
#include "SkGrPriv.h"
#include "SkImage_Base.h"
#include "SkImageCacherator.h"
#include "SkImageFilter.h"
#include "SkLayerInfo.h"
#include "SkMaskFilter.h"
#include "SkNinePatchIter.h"
#include "SkPathEffect.h"
#include "SkPicture.h"
#include "SkPictureData.h"
#include "SkRRect.h"
#include "SkRecord.h"
#include "SkStroke.h"
#include "SkSurface.h"
#include "SkSurface_Gpu.h"
#include "SkTLazy.h"
#include "SkUtils.h"
#include "SkVertState.h"
#include "SkXfermode.h"
#include "batches/GrRectBatchFactory.h"
#include "effects/GrBicubicEffect.h"
#include "effects/GrDashingEffect.h"
#include "effects/GrSimpleTextureEffect.h"
#include "effects/GrTextureDomain.h"
#include "text/GrTextUtils.h"

#if SK_SUPPORT_GPU

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fContext->debugSingleOwner());)

enum { kDefaultImageFilterCacheSize = 32 * 1024 * 1024 };

#if 0
    extern bool (*gShouldDrawProc)();
    #define CHECK_SHOULD_DRAW(draw)                             \
        do {                                                    \
            if (gShouldDrawProc && !gShouldDrawProc()) return;  \
            this->prepareDraw(draw);                            \
        } while (0)
#else
    #define CHECK_SHOULD_DRAW(draw) this->prepareDraw(draw)
#endif

///////////////////////////////////////////////////////////////////////////////

// Helper for turning a bitmap into a texture. If the bitmap is GrTexture backed this
// just accesses the backing GrTexture. Otherwise, it creates a cached texture
// representation and releases it in the destructor.
class AutoBitmapTexture : public SkNoncopyable {
public:
    AutoBitmapTexture() {}

    AutoBitmapTexture(GrContext* context,
                      const SkBitmap& bitmap,
                      const GrTextureParams& params,
                      GrTexture** texture) {
        SkASSERT(texture);
        *texture = this->set(context, bitmap, params);
    }

    GrTexture* set(GrContext* context,
                   const SkBitmap& bitmap,
                   const GrTextureParams& params) {
        // Either get the texture directly from the bitmap, or else use the cache and
        // remember to unref it.
        if (GrTexture* bmpTexture = bitmap.getTexture()) {
            fTexture.reset(nullptr);
            return bmpTexture;
        } else {
            fTexture.reset(GrRefCachedBitmapTexture(context, bitmap, params));
            return fTexture.get();
        }
    }

private:
    SkAutoTUnref<GrTexture> fTexture;
};

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

SkGpuDevice* SkGpuDevice::Create(GrRenderTarget* rt, const SkSurfaceProps* props,
                                 InitContents init) {
    return SkGpuDevice::Create(rt, rt->width(), rt->height(), props, init);
}

SkGpuDevice* SkGpuDevice::Create(GrRenderTarget* rt, int width, int height,
                                 const SkSurfaceProps* props, InitContents init) {
    if (!rt || rt->wasDestroyed()) {
        return nullptr;
    }
    unsigned flags;
    if (!CheckAlphaTypeAndGetFlags(nullptr, init, &flags)) {
        return nullptr;
    }
    return new SkGpuDevice(rt, width, height, props, flags);
}

SkGpuDevice* SkGpuDevice::Create(GrContext* context, SkBudgeted budgeted,
                                 const SkImageInfo& info, int sampleCount,
                                 const SkSurfaceProps* props, InitContents init,
                                 GrTextureStorageAllocator customAllocator) {
    unsigned flags;
    if (!CheckAlphaTypeAndGetFlags(&info, init, &flags)) {
        return nullptr;
    }

    SkAutoTUnref<GrRenderTarget> rt(CreateRenderTarget(
            context, budgeted, info, sampleCount, customAllocator));
    if (nullptr == rt) {
        return nullptr;
    }

    return new SkGpuDevice(rt, info.width(), info.height(), props, flags);
}

SkGpuDevice::SkGpuDevice(GrRenderTarget* rt, int width, int height,
                         const SkSurfaceProps* props, unsigned flags)
    : INHERITED(SkSurfacePropsCopyOrDefault(props))
    , fContext(SkRef(rt->getContext()))
    , fRenderTarget(SkRef(rt)) {
    fOpaque = SkToBool(flags & kIsOpaque_Flag);

    SkAlphaType at = fOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
    SkImageInfo info = rt->surfacePriv().info(at).makeWH(width, height);
    SkPixelRef* pr = new SkGrPixelRef(info, rt);
    fLegacyBitmap.setInfo(info);
    fLegacyBitmap.setPixelRef(pr)->unref();

    fDrawContext.reset(this->context()->drawContext(rt, &this->surfaceProps()));
    if (flags & kNeedClear_Flag) {
        this->clearAll();
    }
}

GrRenderTarget* SkGpuDevice::CreateRenderTarget(
        GrContext* context, SkBudgeted budgeted, const SkImageInfo& origInfo,
        int sampleCount, GrTextureStorageAllocator textureStorageAllocator) {
    if (kUnknown_SkColorType == origInfo.colorType() ||
        origInfo.width() < 0 || origInfo.height() < 0) {
        return nullptr;
    }

    if (!context) {
        return nullptr;
    }

    SkColorType ct = origInfo.colorType();
    SkAlphaType at = origInfo.alphaType();
    SkColorProfileType pt = origInfo.profileType();
    if (kRGB_565_SkColorType == ct) {
        at = kOpaque_SkAlphaType;  // force this setting
    } else if (ct != kBGRA_8888_SkColorType && ct != kRGBA_8888_SkColorType) {
        // Fall back from whatever ct was to default of kRGBA or kBGRA which is aliased as kN32
        ct = kN32_SkColorType;
    }
    if (kOpaque_SkAlphaType != at) {
        at = kPremul_SkAlphaType;  // force this setting
    }
    const SkImageInfo info = SkImageInfo::Make(origInfo.width(), origInfo.height(), ct, at, pt);

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = info.width();
    desc.fHeight = info.height();
    desc.fConfig = SkImageInfo2GrPixelConfig(info, *context->caps());
    desc.fSampleCnt = sampleCount;
    desc.fTextureStorageAllocator = textureStorageAllocator;
    desc.fIsMipMapped = false;
    GrTexture* texture = context->textureProvider()->createTexture(desc, budgeted, nullptr, 0);
    if (nullptr == texture) {
        return nullptr;
    }
    SkASSERT(nullptr != texture->asRenderTarget());
    return texture->asRenderTarget();
}

// This method ensures that we always have a texture-backed "bitmap" when we finally
// call through to the base impl so that the image filtering code will take the
// gpu-specific paths. This mirrors SkCanvas::internalDrawDevice (the other
// use of SkImageFilter::filterImage) in that the source and dest will have
// homogenous backing (e.g., raster or gpu).
void SkGpuDevice::drawSpriteWithFilter(const SkDraw& draw, const SkBitmap& bitmap,
                                       int x, int y, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawSpriteWithFilter", fContext);

    if (fContext->abandoned()) {
        return;
    }

    if (bitmap.getTexture()) {
        INHERITED::drawSpriteWithFilter(draw, bitmap, x, y, paint);
        return;
    }

    SkAutoLockPixels alp(bitmap, !bitmap.getTexture());
    if (!bitmap.getTexture() && !bitmap.readyToDraw()) {
        return;
    }

    GrTexture* texture;
    // draw sprite neither filters nor tiles.
    AutoBitmapTexture abt(fContext, bitmap, GrTextureParams::ClampNoFilter(), &texture);
    if (!texture) {
        return;
    }

    SkBitmap newBitmap;

    GrWrapTextureInBitmap(texture, texture->width(), texture->height(),
                          bitmap.isOpaque(), &newBitmap);

    INHERITED::drawSpriteWithFilter(draw, newBitmap, x, y, paint);
}

///////////////////////////////////////////////////////////////////////////////

bool SkGpuDevice::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                               int x, int y) {
    ASSERT_SINGLE_OWNER

    // TODO: teach fRenderTarget to take ImageInfo directly to specify the src pixels
    GrPixelConfig config = SkImageInfo2GrPixelConfig(dstInfo, *fContext->caps());
    if (kUnknown_GrPixelConfig == config) {
        return false;
    }

    uint32_t flags = 0;
    if (kUnpremul_SkAlphaType == dstInfo.alphaType()) {
        flags = GrContext::kUnpremul_PixelOpsFlag;
    }
    return fRenderTarget->readPixels(x, y, dstInfo.width(), dstInfo.height(), config, dstPixels,
                                     dstRowBytes, flags);
}

bool SkGpuDevice::onWritePixels(const SkImageInfo& info, const void* pixels, size_t rowBytes,
                                int x, int y) {
    ASSERT_SINGLE_OWNER
    // TODO: teach fRenderTarget to take ImageInfo directly to specify the src pixels
    GrPixelConfig config = SkImageInfo2GrPixelConfig(info, *fContext->caps());
    if (kUnknown_GrPixelConfig == config) {
        return false;
    }
    uint32_t flags = 0;
    if (kUnpremul_SkAlphaType == info.alphaType()) {
        flags = GrContext::kUnpremul_PixelOpsFlag;
    }
    fRenderTarget->writePixels(x, y, info.width(), info.height(), config, pixels, rowBytes, flags);

    // need to bump our genID for compatibility with clients that "know" we have a bitmap
    fLegacyBitmap.notifyPixelsChanged();

    return true;
}

const SkBitmap& SkGpuDevice::onAccessBitmap() {
    ASSERT_SINGLE_OWNER
    return fLegacyBitmap;
}

bool SkGpuDevice::onAccessPixels(SkPixmap* pmap) {
    ASSERT_SINGLE_OWNER
    // For compatibility with clients the know we're backed w/ a bitmap, and want to inspect its
    // genID. When we can hide/remove that fact, we can eliminate this call to notify.
    // ... ugh.
    fLegacyBitmap.notifyPixelsChanged();
    return false;
}

void SkGpuDevice::onAttachToCanvas(SkCanvas* canvas) {
    ASSERT_SINGLE_OWNER
    INHERITED::onAttachToCanvas(canvas);

    // Canvas promises that this ptr is valid until onDetachFromCanvas is called
    fClipStack.reset(SkRef(canvas->getClipStack()));
}

void SkGpuDevice::onDetachFromCanvas() {
    ASSERT_SINGLE_OWNER
    INHERITED::onDetachFromCanvas();
    fClip.reset();
    fClipStack.reset(nullptr);
}

// call this every draw call, to ensure that the context reflects our state,
// and not the state from some other canvas/device
void SkGpuDevice::prepareDraw(const SkDraw& draw) {
    ASSERT_SINGLE_OWNER
    SkASSERT(fClipStack.get());

    SkASSERT(draw.fClipStack && draw.fClipStack == fClipStack);

    fClip.setClipStack(fClipStack, &this->getOrigin());
}

GrRenderTarget* SkGpuDevice::accessRenderTarget() {
    ASSERT_SINGLE_OWNER
    return fRenderTarget;
}

void SkGpuDevice::clearAll() {
    ASSERT_SINGLE_OWNER
    GrColor color = 0;
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "clearAll", fContext);
    SkIRect rect = SkIRect::MakeWH(this->width(), this->height());
    fDrawContext->clear(&rect, color, true);
}

void SkGpuDevice::replaceRenderTarget(bool shouldRetainContent) {
    ASSERT_SINGLE_OWNER

    SkBudgeted budgeted = fRenderTarget->resourcePriv().isBudgeted();

    SkAutoTUnref<GrRenderTarget> newRT(CreateRenderTarget(
        this->context(), budgeted, this->imageInfo(), fRenderTarget->desc().fSampleCnt,
        fRenderTarget->desc().fTextureStorageAllocator));

    if (nullptr == newRT) {
        return;
    }

    if (shouldRetainContent) {
        if (fRenderTarget->wasDestroyed()) {
            return;
        }
        this->context()->copySurface(newRT, fRenderTarget);
    }

    SkASSERT(fRenderTarget != newRT);

    fRenderTarget.reset(newRT.release());

#ifdef SK_DEBUG
    SkImageInfo info = fRenderTarget->surfacePriv().info(fOpaque ? kOpaque_SkAlphaType :
                                                                   kPremul_SkAlphaType);
    SkASSERT(info == fLegacyBitmap.info());
#endif
    SkPixelRef* pr = new SkGrPixelRef(fLegacyBitmap.info(), fRenderTarget);
    fLegacyBitmap.setPixelRef(pr)->unref();

    fDrawContext.reset(this->context()->drawContext(fRenderTarget, &this->surfaceProps()));
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawPaint(const SkDraw& draw, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    CHECK_SHOULD_DRAW(draw);
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawPaint", fContext);

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), paint, *draw.fMatrix,
                          this->surfaceProps().allowSRGBInputs(), &grPaint)) {
        return;
    }

    fDrawContext->drawPaint(fClip, grPaint, *draw.fMatrix);
}

// must be in SkCanvas::PointMode order
static const GrPrimitiveType gPointMode2PrimtiveType[] = {
    kPoints_GrPrimitiveType,
    kLines_GrPrimitiveType,
    kLineStrip_GrPrimitiveType
};

// suppress antialiasing on axis-aligned integer-coordinate lines
static bool needs_antialiasing(SkCanvas::PointMode mode, size_t count, const SkPoint pts[]) {
    if (mode == SkCanvas::PointMode::kPoints_PointMode) {
        return false;
    }
    if (count == 2) {
        // We do not antialias as long as the primary axis of the line is integer-aligned, even if
        // the other coordinates are not. This does mean the two end pixels of the line will be
        // sharp even when they shouldn't be, but turning antialiasing on (as things stand
        // currently) means that the line will turn into a two-pixel-wide blur. While obviously a
        // more complete fix is possible down the road, for the time being we accept the error on
        // the two end pixels as being the lesser of two evils.
        if (pts[0].fX == pts[1].fX) {
            return ((int) pts[0].fX) != pts[0].fX;
        }
        if (pts[0].fY == pts[1].fY) {
            return ((int) pts[0].fY) != pts[0].fY;
        }
    }
    return true;
}

void SkGpuDevice::drawPoints(const SkDraw& draw, SkCanvas::PointMode mode,
                             size_t count, const SkPoint pts[], const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawPoints", fContext);
    CHECK_SHOULD_DRAW(draw);

    SkScalar width = paint.getStrokeWidth();
    if (width < 0) {
        return;
    }

    if (paint.getPathEffect() && 2 == count && SkCanvas::kLines_PointMode == mode) {
        GrStrokeInfo strokeInfo(paint, SkPaint::kStroke_Style);
        GrPaint grPaint;
        if (!SkPaintToGrPaint(this->context(), paint, *draw.fMatrix,
                              this->surfaceProps().allowSRGBInputs(), &grPaint)) {
            return;
        }
        SkPath path;
        path.setIsVolatile(true);
        path.moveTo(pts[0]);
        path.lineTo(pts[1]);
        fDrawContext->drawPath(fClip, grPaint, *draw.fMatrix, path, strokeInfo);
        return;
    }

    // we only handle non-antialiased hairlines and paints without path effects or mask filters,
    // else we let the SkDraw call our drawPath()
    if (width > 0 || paint.getPathEffect() || paint.getMaskFilter() ||
        (paint.isAntiAlias() && needs_antialiasing(mode, count, pts))) {
        draw.drawPoints(mode, count, pts, paint, true);
        return;
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), paint, *draw.fMatrix,
                          this->surfaceProps().allowSRGBInputs(), &grPaint)) {
        return;
    }

    fDrawContext->drawVertices(fClip,
                               grPaint,
                               *draw.fMatrix,
                               gPointMode2PrimtiveType[mode],
                               SkToS32(count),
                               (SkPoint*)pts,
                               nullptr,
                               nullptr,
                               nullptr,
                               0);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawRect(const SkDraw& draw, const SkRect& rect, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawRect", fContext);
    CHECK_SHOULD_DRAW(draw);

    bool doStroke = paint.getStyle() != SkPaint::kFill_Style;
    SkScalar width = paint.getStrokeWidth();

    /*
        We have special code for hairline strokes, miter-strokes, bevel-stroke
        and fills. Anything else we just call our path code.
     */
    bool usePath = doStroke && width > 0 &&
                   (paint.getStrokeJoin() == SkPaint::kRound_Join ||
                    (paint.getStrokeJoin() == SkPaint::kBevel_Join && rect.isEmpty()));

    // a few other reasons we might need to call drawPath...
    if (paint.getMaskFilter() || paint.getPathEffect() ||
        paint.getStyle() == SkPaint::kStrokeAndFill_Style) { // we can't both stroke and fill rects
        usePath = true;
    }

    if (usePath) {
        SkPath path;
        path.setIsVolatile(true);
        path.addRect(rect);
        GrBlurUtils::drawPathWithMaskFilter(fContext, fDrawContext,
                                            fClip, path, paint,
                                            *draw.fMatrix, nullptr,
                                            draw.fClip->getBounds(), true);
        return;
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), paint, *draw.fMatrix,
                          this->surfaceProps().allowSRGBInputs(), &grPaint)) {
        return;
    }

    GrStrokeInfo strokeInfo(paint);

    fDrawContext->drawRect(fClip, grPaint, *draw.fMatrix, rect, &strokeInfo);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawRRect(const SkDraw& draw, const SkRRect& rect,
                            const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawRRect", fContext);
    CHECK_SHOULD_DRAW(draw);

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), paint, *draw.fMatrix,
                          this->surfaceProps().allowSRGBInputs(), &grPaint)) {
        return;
    }

    GrStrokeInfo strokeInfo(paint);
    if (paint.getMaskFilter()) {
        // try to hit the fast path for drawing filtered round rects

        SkRRect devRRect;
        if (rect.transform(*draw.fMatrix, &devRRect)) {
            if (devRRect.allCornersCircular()) {
                SkRect maskRect;
                if (paint.getMaskFilter()->canFilterMaskGPU(devRRect,
                                                            draw.fClip->getBounds(),
                                                            *draw.fMatrix,
                                                            &maskRect)) {
                    SkIRect finalIRect;
                    maskRect.roundOut(&finalIRect);
                    if (draw.fClip->quickReject(finalIRect)) {
                        // clipped out
                        return;
                    }
                    if (paint.getMaskFilter()->directFilterRRectMaskGPU(fContext->textureProvider(),
                                                                        fDrawContext,
                                                                        &grPaint,
                                                                        fClip,
                                                                        *draw.fMatrix,
                                                                        strokeInfo,
                                                                        devRRect)) {
                        return;
                    }
                }

            }
        }
    }

    if (paint.getMaskFilter() || paint.getPathEffect()) {
        // The only mask filter the native rrect drawing code could've handle was taken
        // care of above.
        // A path effect will presumably transform this rrect into something else.
        SkPath path;
        path.setIsVolatile(true);
        path.addRRect(rect);
        GrBlurUtils::drawPathWithMaskFilter(fContext, fDrawContext,
                                            fClip, path, paint,
                                            *draw.fMatrix, nullptr,
                                            draw.fClip->getBounds(), true);
        return;
    }

    SkASSERT(!strokeInfo.isDashed());

    fDrawContext->drawRRect(fClip, grPaint, *draw.fMatrix, rect, strokeInfo);
}


void SkGpuDevice::drawDRRect(const SkDraw& draw, const SkRRect& outer,
                             const SkRRect& inner, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawDRRect", fContext);
    CHECK_SHOULD_DRAW(draw);

    if (outer.isEmpty()) {
       return;
    }

    if (inner.isEmpty()) {
        return this->drawRRect(draw, outer, paint);
    }

    SkStrokeRec stroke(paint);

    if (stroke.isFillStyle() && !paint.getMaskFilter() && !paint.getPathEffect()) {
        GrPaint grPaint;
        if (!SkPaintToGrPaint(this->context(), paint, *draw.fMatrix,
                              this->surfaceProps().allowSRGBInputs(), &grPaint)) {
            return;
        }

        fDrawContext->drawDRRect(fClip, grPaint, *draw.fMatrix, outer, inner);
        return;
    }

    SkPath path;
    path.setIsVolatile(true);
    path.addRRect(outer);
    path.addRRect(inner);
    path.setFillType(SkPath::kEvenOdd_FillType);

    GrBlurUtils::drawPathWithMaskFilter(fContext, fDrawContext,
                                        fClip, path, paint,
                                        *draw.fMatrix, nullptr,
                                        draw.fClip->getBounds(), true);
}


/////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawOval(const SkDraw& draw, const SkRect& oval, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawOval", fContext);
    CHECK_SHOULD_DRAW(draw);

    // Presumably the path effect warps this to something other than an oval
    if (paint.getPathEffect()) {
        SkPath path;
        path.setIsVolatile(true);
        path.addOval(oval);
        this->drawPath(draw, path, paint, nullptr, true);
        return;
    }

    if (paint.getMaskFilter()) {
        // The RRect path can handle special case blurring
        SkRRect rr = SkRRect::MakeOval(oval);
        return this->drawRRect(draw, rr, paint);
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), paint, *draw.fMatrix,
                          this->surfaceProps().allowSRGBInputs(), &grPaint)) {
        return;
    }

    GrStrokeInfo strokeInfo(paint);
    SkASSERT(!strokeInfo.isDashed());

    fDrawContext->drawOval(fClip, grPaint, *draw.fMatrix, oval, strokeInfo);
}

#include "SkMaskFilter.h"

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawPath(const SkDraw& draw, const SkPath& origSrcPath,
                           const SkPaint& paint, const SkMatrix* prePathMatrix,
                           bool pathIsMutable) {
    ASSERT_SINGLE_OWNER
    if (!origSrcPath.isInverseFillType() && !paint.getPathEffect() && !prePathMatrix) {
        bool isClosed;
        SkRect rect;
        if (origSrcPath.isRect(&rect, &isClosed) && isClosed) {
            this->drawRect(draw, rect, paint);
            return;
        }
        if (origSrcPath.isOval(&rect)) {
            this->drawOval(draw, rect, paint);
            return;
        }
        SkRRect rrect;
        if (origSrcPath.isRRect(&rrect)) {
            this->drawRRect(draw, rrect, paint);
            return;
        }
    }

    CHECK_SHOULD_DRAW(draw);
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawPath", fContext);

    GrBlurUtils::drawPathWithMaskFilter(fContext, fDrawContext,
                                        fClip, origSrcPath, paint,
                                        *draw.fMatrix, prePathMatrix,
                                        draw.fClip->getBounds(), pathIsMutable);
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
static void determine_clipped_src_rect(const GrRenderTarget* rt,
                                       const GrClip& clip,
                                       const SkMatrix& viewMatrix,
                                       const SkISize& imageSize,
                                       const SkRect* srcRectPtr,
                                       SkIRect* clippedSrcIRect) {
    clip.getConservativeBounds(rt->width(), rt->height(), clippedSrcIRect, nullptr);
    SkMatrix inv;
    if (!viewMatrix.invert(&inv)) {
        clippedSrcIRect->setEmpty();
        return;
    }
    SkRect clippedSrcRect = SkRect::Make(*clippedSrcIRect);
    inv.mapRect(&clippedSrcRect);
    if (srcRectPtr) {
        // we've setup src space 0,0 to map to the top left of the src rect.
        clippedSrcRect.offset(srcRectPtr->fLeft, srcRectPtr->fTop);
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
                                    const GrTextureParams& params,
                                    const SkRect* srcRectPtr,
                                    int maxTileSize,
                                    int* tileSize,
                                    SkIRect* clippedSubset) const {
    ASSERT_SINGLE_OWNER
    // if it's larger than the max tile size, then we have no choice but tiling.
    if (imageRect.width() > maxTileSize || imageRect.height() > maxTileSize) {
        determine_clipped_src_rect(fRenderTarget, fClip, viewMatrix, imageRect.size(),
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
    determine_clipped_src_rect(fRenderTarget, fClip, viewMatrix, imageRect.size(), srcRectPtr,
                               clippedSubset);
    *tileSize = kBmpSmallTileSize; // already know whole bitmap fits in one max sized tile.
    size_t usedTileBytes = get_tile_count(*clippedSubset, kBmpSmallTileSize) *
                           kBmpSmallTileSize * kBmpSmallTileSize;

    return usedTileBytes < 2 * bmpSize;
}

bool SkGpuDevice::shouldTileBitmap(const SkBitmap& bitmap,
                                   const SkMatrix& viewMatrix,
                                   const GrTextureParams& params,
                                   const SkRect* srcRectPtr,
                                   int maxTileSize,
                                   int* tileSize,
                                   SkIRect* clippedSrcRect) const {
    ASSERT_SINGLE_OWNER
    // if bitmap is explictly texture backed then just use the texture
    if (bitmap.getTexture()) {
        return false;
    }

    return this->shouldTileImageID(bitmap.getGenerationID(), bitmap.getSubset(), viewMatrix, params,
                                   srcRectPtr, maxTileSize, tileSize, clippedSrcRect);
}

bool SkGpuDevice::shouldTileImage(const SkImage* image, const SkRect* srcRectPtr,
                                  SkCanvas::SrcRectConstraint constraint, SkFilterQuality quality,
                                  const SkMatrix& viewMatrix) const {
    ASSERT_SINGLE_OWNER
    // if image is explictly texture backed then just use the texture
    if (as_IB(image)->peekTexture()) {
        return false;
    }

    GrTextureParams params;
    bool doBicubic;
    GrTextureParams::FilterMode textureFilterMode =
                    GrSkFilterQualityToGrFilterMode(quality, viewMatrix, SkMatrix::I(), &doBicubic);

    int tileFilterPad;
    if (doBicubic) {
        tileFilterPad = GrBicubicEffect::kFilterTexelPad;
    } else if (GrTextureParams::kNone_FilterMode == textureFilterMode) {
        tileFilterPad = 0;
    } else {
        tileFilterPad = 1;
    }
    params.setFilterMode(textureFilterMode);

    int maxTileSize = fContext->caps()->maxTileSize() - 2 * tileFilterPad;

    // these are output, which we safely ignore, as we just want to know the predicate
    int outTileSize;
    SkIRect outClippedSrcRect;

    return this->shouldTileImageID(image->unique(), image->bounds(), viewMatrix, params, srcRectPtr,
                                   maxTileSize, &outTileSize, &outClippedSrcRect);
}

void SkGpuDevice::drawBitmap(const SkDraw& origDraw,
                             const SkBitmap& bitmap,
                             const SkMatrix& m,
                             const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    CHECK_SHOULD_DRAW(origDraw);
    SkMatrix viewMatrix;
    viewMatrix.setConcat(*origDraw.fMatrix, m);
    if (bitmap.getTexture()) {
        GrBitmapTextureAdjuster adjuster(&bitmap);
        // We can use kFast here because we know texture-backed bitmaps don't support extractSubset.
        this->drawTextureProducer(&adjuster, nullptr, nullptr, SkCanvas::kFast_SrcRectConstraint,
                                  viewMatrix, fClip, paint);
        return;
    }
    int maxTileSize = fContext->caps()->maxTileSize();

    // The tile code path doesn't currently support AA, so if the paint asked for aa and we could
    // draw untiled, then we bypass checking for tiling purely for optimization reasons.
    bool drawAA = !fRenderTarget->isUnifiedMultisampled() &&
                  paint.isAntiAlias() &&
                  bitmap.width() <= maxTileSize &&
                  bitmap.height() <= maxTileSize;

    bool skipTileCheck = drawAA || paint.getMaskFilter();

    if (!skipTileCheck) {
        SkRect srcRect = SkRect::MakeIWH(bitmap.width(), bitmap.height());
        int tileSize;
        SkIRect clippedSrcRect;

        GrTextureParams params;
        bool doBicubic;
        GrTextureParams::FilterMode textureFilterMode =
            GrSkFilterQualityToGrFilterMode(paint.getFilterQuality(), viewMatrix, SkMatrix::I(),
                                            &doBicubic);

        int tileFilterPad;

        if (doBicubic) {
            tileFilterPad = GrBicubicEffect::kFilterTexelPad;
        } else if (GrTextureParams::kNone_FilterMode == textureFilterMode) {
            tileFilterPad = 0;
        } else {
            tileFilterPad = 1;
        }
        params.setFilterMode(textureFilterMode);

        int maxTileSizeForFilter = fContext->caps()->maxTileSize() - 2 * tileFilterPad;
        if (this->shouldTileBitmap(bitmap, viewMatrix, params, &srcRect,
                                   maxTileSizeForFilter, &tileSize, &clippedSrcRect)) {
            this->drawTiledBitmap(bitmap, viewMatrix, srcRect, clippedSrcRect, params, paint,
                                  SkCanvas::kStrict_SrcRectConstraint, tileSize, doBicubic);
            return;
        }
    }
    GrBitmapTextureMaker maker(fContext, bitmap);
    this->drawTextureProducer(&maker, nullptr, nullptr, SkCanvas::kStrict_SrcRectConstraint,
                              viewMatrix, fClip, paint);
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
                                  const SkRect& srcRect,
                                  const SkIRect& clippedSrcIRect,
                                  const GrTextureParams& params,
                                  const SkPaint& origPaint,
                                  SkCanvas::SrcRectConstraint constraint,
                                  int tileSize,
                                  bool bicubic) {
    ASSERT_SINGLE_OWNER

    // This is the funnel for all paths that draw tiled bitmaps/images. Log histogram entry.
    SK_HISTOGRAM_BOOLEAN("DrawTiled", true);

    // The following pixel lock is technically redundant, but it is desirable
    // to lock outside of the tile loop to prevent redecoding the whole image
    // at each tile in cases where 'bitmap' holds an SkDiscardablePixelRef that
    // is larger than the limit of the discardable memory pool.
    SkAutoLockPixels alp(bitmap);

    const SkPaint* paint = &origPaint;
    SkPaint tempPaint;
    if (origPaint.isAntiAlias() && !fRenderTarget->isUnifiedMultisampled()) {
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

            SkBitmap tmpB;
            SkIRect iTileR;
            tileR.roundOut(&iTileR);
            SkPoint offset = SkPoint::Make(SkIntToScalar(iTileR.fLeft),
                                           SkIntToScalar(iTileR.fTop));

            // Adjust the context matrix to draw at the right x,y in device space
            SkMatrix viewM = viewMatrix;
            SkMatrix tmpM;
            tmpM.setTranslate(offset.fX - srcRect.fLeft, offset.fY - srcRect.fTop);
            viewM.preConcat(tmpM);

            if (GrTextureParams::kNone_FilterMode != params.filterMode() || bicubic) {
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

            if (bitmap.extractSubset(&tmpB, iTileR)) {
                // now offset it to make it "local" to our tmp bitmap
                tileR.offset(-offset.fX, -offset.fY);
                GrTextureParams paramsTemp = params;
                // de-optimized this determination
                bool needsTextureDomain = true;
                this->internalDrawBitmap(tmpB,
                                         viewM,
                                         tileR,
                                         paramsTemp,
                                         *paint,
                                         constraint,
                                         bicubic,
                                         needsTextureDomain);
            }
        }
    }
}

/*
 *  This is called by drawBitmap(), which has to handle images that may be too
 *  large to be represented by a single texture.
 *
 *  internalDrawBitmap assumes that the specified bitmap will fit in a texture
 *  and that non-texture portion of the GrPaint has already been setup.
 */
void SkGpuDevice::internalDrawBitmap(const SkBitmap& bitmap,
                                     const SkMatrix& viewMatrix,
                                     const SkRect& srcRect,
                                     const GrTextureParams& params,
                                     const SkPaint& paint,
                                     SkCanvas::SrcRectConstraint constraint,
                                     bool bicubic,
                                     bool needsTextureDomain) {
    // We should have already handled bitmaps larger than the max texture size.
    SkASSERT(bitmap.width() <= fContext->caps()->maxTextureSize() &&
             bitmap.height() <= fContext->caps()->maxTextureSize());
    // Unless the bitmap is inherently texture-backed, we should be respecting the max tile size
    // by the time we get here.
    SkASSERT(bitmap.getTexture() ||
             (bitmap.width() <= fContext->caps()->maxTileSize() &&
              bitmap.height() <= fContext->caps()->maxTileSize()));

    GrTexture* texture;
    AutoBitmapTexture abt(fContext, bitmap, params, &texture);
    if (nullptr == texture) {
        return;
    }

    SkRect dstRect = {0, 0, srcRect.width(), srcRect.height() };
    SkRect paintRect;
    SkScalar wInv = SkScalarInvert(SkIntToScalar(texture->width()));
    SkScalar hInv = SkScalarInvert(SkIntToScalar(texture->height()));
    paintRect.setLTRB(SkScalarMul(srcRect.fLeft,   wInv),
                      SkScalarMul(srcRect.fTop,    hInv),
                      SkScalarMul(srcRect.fRight,  wInv),
                      SkScalarMul(srcRect.fBottom, hInv));

    SkMatrix texMatrix;
    texMatrix.reset();
    if (kAlpha_8_SkColorType == bitmap.colorType() && paint.getShader()) {
        // In cases where we are doing an A8 bitmap draw with a shader installed, we cannot use
        // local coords with the bitmap draw since it may mess up texture look ups for the shader.
        // Thus we need to pass in the transform matrix directly to the texture processor used for
        // the bitmap draw.
        texMatrix.setScale(wInv, hInv);
    }

    SkRect textureDomain = SkRect::MakeEmpty();

    // Construct a GrPaint by setting the bitmap texture as the first effect and then configuring
    // the rest from the SkPaint.
    SkAutoTUnref<const GrFragmentProcessor> fp;

    if (needsTextureDomain && (SkCanvas::kStrict_SrcRectConstraint == constraint)) {
        // Use a constrained texture domain to avoid color bleeding
        SkScalar left, top, right, bottom;
        if (srcRect.width() > SK_Scalar1) {
            SkScalar border = SK_ScalarHalf / texture->width();
            left = paintRect.left() + border;
            right = paintRect.right() - border;
        } else {
            left = right = SkScalarHalf(paintRect.left() + paintRect.right());
        }
        if (srcRect.height() > SK_Scalar1) {
            SkScalar border = SK_ScalarHalf / texture->height();
            top = paintRect.top() + border;
            bottom = paintRect.bottom() - border;
        } else {
            top = bottom = SkScalarHalf(paintRect.top() + paintRect.bottom());
        }
        textureDomain.setLTRB(left, top, right, bottom);
        if (bicubic) {
            fp.reset(GrBicubicEffect::Create(texture, texMatrix, textureDomain));
        } else {
            fp.reset(GrTextureDomainEffect::Create(texture,
                                                   texMatrix,
                                                   textureDomain,
                                                   GrTextureDomain::kClamp_Mode,
                                                   params.filterMode()));
        }
    } else if (bicubic) {
        SkASSERT(GrTextureParams::kNone_FilterMode == params.filterMode());
        SkShader::TileMode tileModes[2] = { params.getTileModeX(), params.getTileModeY() };
        fp.reset(GrBicubicEffect::Create(texture, texMatrix, tileModes));
    } else {
        fp.reset(GrSimpleTextureEffect::Create(texture, texMatrix, params));
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaintWithTexture(this->context(), paint, viewMatrix, fp,
                                     kAlpha_8_SkColorType == bitmap.colorType(),
                                     this->surfaceProps().allowSRGBInputs(), &grPaint)) {
        return;
    }

    if (kAlpha_8_SkColorType == bitmap.colorType() && paint.getShader()) {
        // We don't have local coords in this case and have previously set the transform
        // matrices directly on the texture processor.
        fDrawContext->drawRect(fClip, grPaint, viewMatrix, dstRect);
    } else {
        fDrawContext->fillRectToRect(fClip, grPaint, viewMatrix, dstRect, paintRect);
    }
}

bool SkGpuDevice::filterTexture(GrContext* context, GrTexture* texture,
                                int width, int height,
                                const SkImageFilter* filter,
                                const SkImageFilter::Context& ctx,
                                SkBitmap* result, SkIPoint* offset) {
    ASSERT_SINGLE_OWNER
    SkASSERT(filter);

    SkImageFilter::DeviceProxy proxy(this);

    if (filter->canFilterImageGPU()) {
        SkBitmap bm;
        GrWrapTextureInBitmap(texture, width, height, false, &bm);
        return filter->filterImageGPUDeprecated(&proxy, bm, ctx, result, offset);
    } else {
        return false;
    }
}

void SkGpuDevice::drawSprite(const SkDraw& draw, const SkBitmap& bitmap,
                             int left, int top, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    // drawSprite is defined to be in device coords.
    CHECK_SHOULD_DRAW(draw);

    SkAutoLockPixels alp(bitmap, !bitmap.getTexture());
    if (!bitmap.getTexture() && !bitmap.readyToDraw()) {
        return;
    }

    int offX = bitmap.pixelRefOrigin().fX;
    int offY = bitmap.pixelRefOrigin().fY;
    int w = bitmap.width();
    int h = bitmap.height();

    GrTexture* texture;
    // draw sprite neither filters nor tiles.
    AutoBitmapTexture abt(fContext, bitmap, GrTextureParams::ClampNoFilter(), &texture);
    if (!texture) {
        return;
    }

    bool alphaOnly = kAlpha_8_SkColorType == bitmap.colorType();

    SkImageFilter* filter = paint.getImageFilter();
    // This bitmap will own the filtered result as a texture.
    SkBitmap filteredBitmap;

    if (filter) {
        SkIPoint offset = SkIPoint::Make(0, 0);
        SkMatrix matrix(*draw.fMatrix);
        matrix.postTranslate(SkIntToScalar(-left), SkIntToScalar(-top));
        SkIRect clipBounds = draw.fClip->getBounds().makeOffset(-left, -top);
        SkAutoTUnref<SkImageFilter::Cache> cache(getImageFilterCache());
        // This cache is transient, and is freed (along with all its contained
        // textures) when it goes out of scope.
        SkImageFilter::Context ctx(matrix, clipBounds, cache);
        if (this->filterTexture(fContext, texture, w, h, filter, ctx, &filteredBitmap,
                                &offset)) {
            texture = (GrTexture*) filteredBitmap.getTexture();
            offX = filteredBitmap.pixelRefOrigin().fX;
            offY = filteredBitmap.pixelRefOrigin().fY;
            w = filteredBitmap.width();
            h = filteredBitmap.height();
            left += offset.x();
            top += offset.y();
        } else {
            return;
        }
        SkASSERT(!GrPixelConfigIsAlphaOnly(texture->config()));
        alphaOnly = false;
    }

    GrPaint grPaint;
    SkAutoTUnref<const GrFragmentProcessor> fp(
        GrSimpleTextureEffect::Create(texture, SkMatrix::I()));
    if (alphaOnly) {
        fp.reset(GrFragmentProcessor::MulOutputByInputUnpremulColor(fp));
    } else {
        fp.reset(GrFragmentProcessor::MulOutputByInputAlpha(fp));
    }
    if (!SkPaintToGrPaintReplaceShader(this->context(), paint, fp,
                                       this->surfaceProps().allowSRGBInputs(), &grPaint)) {
        return;
    }

    fDrawContext->fillRectToRect(fClip,
                                 grPaint,
                                 SkMatrix::I(),
                                 SkRect::MakeXYWH(SkIntToScalar(left),
                                                  SkIntToScalar(top),
                                                  SkIntToScalar(w),
                                                  SkIntToScalar(h)),
                                 SkRect::MakeXYWH(SkIntToScalar(offX) / texture->width(),
                                                  SkIntToScalar(offY) / texture->height(),
                                                  SkIntToScalar(w) / texture->width(),
                                                  SkIntToScalar(h) / texture->height()));
}

void SkGpuDevice::drawBitmapRect(const SkDraw& draw, const SkBitmap& bitmap,
                                 const SkRect* src, const SkRect& origDst,
                                 const SkPaint& paint, SkCanvas::SrcRectConstraint constraint) {
    ASSERT_SINGLE_OWNER
    CHECK_SHOULD_DRAW(draw);
    if (bitmap.getTexture()) {
        GrBitmapTextureAdjuster adjuster(&bitmap);
        this->drawTextureProducer(&adjuster, src, &origDst, constraint, *draw.fMatrix, fClip,
                                  paint);
        return;
    }
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
    bool drawAA = !fRenderTarget->isUnifiedMultisampled() &&
        paint.isAntiAlias() &&
        bitmap.width() <= maxTileSize &&
        bitmap.height() <= maxTileSize;

    bool skipTileCheck = drawAA || paint.getMaskFilter();

    if (!skipTileCheck) {
        int tileSize;
        SkIRect clippedSrcRect;

        GrTextureParams params;
        bool doBicubic;
        GrTextureParams::FilterMode textureFilterMode =
            GrSkFilterQualityToGrFilterMode(paint.getFilterQuality(), *draw.fMatrix, srcToDstMatrix,
                                            &doBicubic);

        int tileFilterPad;

        if (doBicubic) {
            tileFilterPad = GrBicubicEffect::kFilterTexelPad;
        } else if (GrTextureParams::kNone_FilterMode == textureFilterMode) {
            tileFilterPad = 0;
        } else {
            tileFilterPad = 1;
        }
        params.setFilterMode(textureFilterMode);

        int maxTileSizeForFilter = fContext->caps()->maxTileSize() - 2 * tileFilterPad;
        // Fold the dst rect into the view matrix. This is only OK because we don't get here if
        // we have a mask filter.
        SkMatrix viewMatrix = *draw.fMatrix;
        viewMatrix.preTranslate(dst->fLeft, dst->fTop);
        viewMatrix.preScale(dst->width()/src->width(), dst->height()/src->height());
        if (this->shouldTileBitmap(bitmap, viewMatrix, params, src,
                                   maxTileSizeForFilter, &tileSize, &clippedSrcRect)) {
            this->drawTiledBitmap(bitmap, viewMatrix, *src, clippedSrcRect, params, paint,
                                  constraint, tileSize, doBicubic);
            return;
        }
    }
    GrBitmapTextureMaker maker(fContext, bitmap);
    this->drawTextureProducer(&maker, src, dst, constraint, *draw.fMatrix, fClip, paint);
}

void SkGpuDevice::drawDevice(const SkDraw& draw, SkBaseDevice* device,
                             int x, int y, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    // clear of the source device must occur before CHECK_SHOULD_DRAW
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawDevice", fContext);
    SkGpuDevice* dev = static_cast<SkGpuDevice*>(device);

    // drawDevice is defined to be in device coords.
    CHECK_SHOULD_DRAW(draw);

    GrRenderTarget* devRT = dev->accessRenderTarget();
    GrTexture* devTex;
    if (nullptr == (devTex = devRT->asTexture())) {
        return;
    }

    const SkImageInfo ii = dev->imageInfo();
    int w = ii.width();
    int h = ii.height();

    SkImageFilter* filter = paint.getImageFilter();
    // This bitmap will own the filtered result as a texture.
    SkBitmap filteredBitmap;

    if (filter) {
        SkIPoint offset = SkIPoint::Make(0, 0);
        SkMatrix matrix(*draw.fMatrix);
        matrix.postTranslate(SkIntToScalar(-x), SkIntToScalar(-y));
        SkIRect clipBounds = draw.fClip->getBounds().makeOffset(-x, -y);
        // This cache is transient, and is freed (along with all its contained
        // textures) when it goes out of scope.
        SkAutoTUnref<SkImageFilter::Cache> cache(getImageFilterCache());
        SkImageFilter::Context ctx(matrix, clipBounds, cache);
        if (this->filterTexture(fContext, devTex, device->width(), device->height(),
                                filter, ctx, &filteredBitmap, &offset)) {
            devTex = filteredBitmap.getTexture();
            w = filteredBitmap.width();
            h = filteredBitmap.height();
            x += offset.fX;
            y += offset.fY;
        } else {
            return;
        }
    }

    GrPaint grPaint;
    SkAutoTUnref<const GrFragmentProcessor> fp(
        GrSimpleTextureEffect::Create(devTex, SkMatrix::I()));
    if (GrPixelConfigIsAlphaOnly(devTex->config())) {
        // Can this happen?
        fp.reset(GrFragmentProcessor::MulOutputByInputUnpremulColor(fp));
   } else {
        fp.reset(GrFragmentProcessor::MulOutputByInputAlpha(fp));
    }

    if (!SkPaintToGrPaintReplaceShader(this->context(), paint, fp,
                                       this->surfaceProps().allowSRGBInputs(), &grPaint)) {
        return;
    }

    SkRect dstRect = SkRect::MakeXYWH(SkIntToScalar(x),
                                      SkIntToScalar(y),
                                      SkIntToScalar(w),
                                      SkIntToScalar(h));

    // The device being drawn may not fill up its texture (e.g. saveLayer uses approximate
    // scratch texture).
    SkRect srcRect = SkRect::MakeWH(SK_Scalar1 * w / devTex->width(),
                                    SK_Scalar1 * h / devTex->height());

    fDrawContext->fillRectToRect(fClip, grPaint, SkMatrix::I(), dstRect, srcRect);
}

bool SkGpuDevice::canHandleImageFilter(const SkImageFilter* filter) {
    ASSERT_SINGLE_OWNER
    return filter->canFilterImageGPU();
}

bool SkGpuDevice::filterImage(const SkImageFilter* filter, const SkBitmap& src,
                              const SkImageFilter::Context& ctx,
                              SkBitmap* result, SkIPoint* offset) {
    ASSERT_SINGLE_OWNER
    // want explicitly our impl, so guard against a subclass of us overriding it
    if (!this->SkGpuDevice::canHandleImageFilter(filter)) {
        return false;
    }

    SkAutoLockPixels alp(src, !src.getTexture());
    if (!src.getTexture() && !src.readyToDraw()) {
        return false;
    }

    GrTexture* texture;
    // We assume here that the filter will not attempt to tile the src. Otherwise, this cache lookup
    // must be pushed upstack.
    AutoBitmapTexture abt(fContext, src, GrTextureParams::ClampNoFilter(), &texture);
    if (!texture) {
        return false;
    }

    return this->filterTexture(fContext, texture, src.width(), src.height(),
                               filter, ctx, result, offset);
}

void SkGpuDevice::drawImage(const SkDraw& draw, const SkImage* image, SkScalar x, SkScalar y,
                            const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    SkMatrix viewMatrix = *draw.fMatrix;
    viewMatrix.preTranslate(x, y);
    if (as_IB(image)->peekTexture()) {
        CHECK_SHOULD_DRAW(draw);
        GrImageTextureAdjuster adjuster(as_IB(image));
        this->drawTextureProducer(&adjuster, nullptr, nullptr, SkCanvas::kFast_SrcRectConstraint,
                                  viewMatrix, fClip, paint);
        return;
    } else {
        SkBitmap bm;
        if (this->shouldTileImage(image, nullptr, SkCanvas::kFast_SrcRectConstraint,
                                  paint.getFilterQuality(), *draw.fMatrix)) {
            // only support tiling as bitmap at the moment, so force raster-version
            if (!as_IB(image)->getROPixels(&bm)) {
                return;
            }
            this->drawBitmap(draw, bm, SkMatrix::MakeTrans(x, y), paint);
        } else if (SkImageCacherator* cacher = as_IB(image)->peekCacherator()) {
            CHECK_SHOULD_DRAW(draw);
            GrImageTextureMaker maker(fContext, cacher, image, SkImage::kAllow_CachingHint);
            this->drawTextureProducer(&maker, nullptr, nullptr, SkCanvas::kFast_SrcRectConstraint,
                                      viewMatrix, fClip, paint);
        } else if (as_IB(image)->getROPixels(&bm)) {
            this->drawBitmap(draw, bm, SkMatrix::MakeTrans(x, y), paint);
        }
    }
}

void SkGpuDevice::drawImageRect(const SkDraw& draw, const SkImage* image, const SkRect* src,
                                const SkRect& dst, const SkPaint& paint,
                                SkCanvas::SrcRectConstraint constraint) {
    ASSERT_SINGLE_OWNER
    if (as_IB(image)->peekTexture()) {
        CHECK_SHOULD_DRAW(draw);
        GrImageTextureAdjuster adjuster(as_IB(image));
        this->drawTextureProducer(&adjuster, src, &dst, constraint, *draw.fMatrix, fClip, paint);
        return;
    }
    SkBitmap bm;
    SkMatrix totalMatrix = *draw.fMatrix;
    totalMatrix.preScale(dst.width() / (src ? src->width() : image->width()),
                         dst.height() / (src ? src->height() : image->height()));
    if (this->shouldTileImage(image, src, constraint, paint.getFilterQuality(), totalMatrix)) {
        // only support tiling as bitmap at the moment, so force raster-version
        if (!as_IB(image)->getROPixels(&bm)) {
            return;
        }
        this->drawBitmapRect(draw, bm, src, dst, paint, constraint);
    } else if (SkImageCacherator* cacher = as_IB(image)->peekCacherator()) {
        CHECK_SHOULD_DRAW(draw);
        GrImageTextureMaker maker(fContext, cacher, image, SkImage::kAllow_CachingHint);
        this->drawTextureProducer(&maker, src, &dst, constraint, *draw.fMatrix, fClip, paint);
    } else if (as_IB(image)->getROPixels(&bm)) {
        this->drawBitmapRect(draw, bm, src, dst, paint, constraint);
    }
}

void SkGpuDevice::drawProducerNine(const SkDraw& draw, GrTextureProducer* producer,
                                   const SkIRect& center, const SkRect& dst, const SkPaint& paint) {
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawProducerNine", fContext);

    CHECK_SHOULD_DRAW(draw);

    bool useFallback = paint.getMaskFilter() || paint.isAntiAlias() ||
                       fRenderTarget->isUnifiedMultisampled();
    bool doBicubic;
    GrTextureParams::FilterMode textureFilterMode =
        GrSkFilterQualityToGrFilterMode(paint.getFilterQuality(), *draw.fMatrix, SkMatrix::I(),
                                        &doBicubic);
    if (useFallback || doBicubic || GrTextureParams::kNone_FilterMode != textureFilterMode) {
        SkNinePatchIter iter(producer->width(), producer->height(), center, dst);

        SkRect srcR, dstR;
        while (iter.next(&srcR, &dstR)) {
            this->drawTextureProducer(producer, &srcR, &dstR, SkCanvas::kStrict_SrcRectConstraint,
                                      *draw.fMatrix, fClip, paint);
        }
        return;
    }

    static const GrTextureParams::FilterMode kMode = GrTextureParams::kNone_FilterMode;
    SkAutoTUnref<const GrFragmentProcessor> fp(
        producer->createFragmentProcessor(SkMatrix::I(),
                                          SkRect::MakeIWH(producer->width(), producer->height()),
                                          GrTextureProducer::kNo_FilterConstraint, true,
                                          &kMode));
    GrPaint grPaint;
    if (!SkPaintToGrPaintWithTexture(this->context(), paint, *draw.fMatrix, fp,
                                     producer->isAlphaOnly(),
                                     this->surfaceProps().allowSRGBInputs(), &grPaint)) {
        return;
    }

    fDrawContext->drawImageNine(fClip, grPaint, *draw.fMatrix, producer->width(),
                                producer->height(), center, dst);
}

void SkGpuDevice::drawImageNine(const SkDraw& draw, const SkImage* image,
                                const SkIRect& center, const SkRect& dst, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    if (as_IB(image)->peekTexture()) {
        GrImageTextureAdjuster adjuster(as_IB(image));
        this->drawProducerNine(draw, &adjuster, center, dst, paint);
    } else {
        SkBitmap bm;
        if (SkImageCacherator* cacher = as_IB(image)->peekCacherator()) {
            GrImageTextureMaker maker(fContext, cacher, image, SkImage::kAllow_CachingHint);
            this->drawProducerNine(draw, &maker, center, dst, paint);
        } else if (as_IB(image)->getROPixels(&bm)) {
            this->drawBitmapNine(draw, bm, center, dst, paint);
        }
    }
}

void SkGpuDevice::drawBitmapNine(const SkDraw& draw, const SkBitmap& bitmap, const SkIRect& center,
                                 const SkRect& dst, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    if (bitmap.getTexture()) {
        GrBitmapTextureAdjuster adjuster(&bitmap);
        this->drawProducerNine(draw, &adjuster, center, dst, paint);
    } else {
        GrBitmapTextureMaker maker(fContext, bitmap);
        this->drawProducerNine(draw, &maker, center, dst, paint);
    }
}

///////////////////////////////////////////////////////////////////////////////

// must be in SkCanvas::VertexMode order
static const GrPrimitiveType gVertexMode2PrimitiveType[] = {
    kTriangles_GrPrimitiveType,
    kTriangleStrip_GrPrimitiveType,
    kTriangleFan_GrPrimitiveType,
};

void SkGpuDevice::drawVertices(const SkDraw& draw, SkCanvas::VertexMode vmode,
                              int vertexCount, const SkPoint vertices[],
                              const SkPoint texs[], const SkColor colors[],
                              SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    CHECK_SHOULD_DRAW(draw);
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawVertices", fContext);

    // If both textures and vertex-colors are nullptr, strokes hairlines with the paint's color.
    if ((nullptr == texs || nullptr == paint.getShader()) && nullptr == colors) {

        texs = nullptr;

        SkPaint copy(paint);
        copy.setStyle(SkPaint::kStroke_Style);
        copy.setStrokeWidth(0);

        GrPaint grPaint;
        // we ignore the shader if texs is null.
        if (!SkPaintToGrPaintNoShader(this->context(), copy,
                                      this->surfaceProps().allowSRGBInputs(), &grPaint)) {
            return;
        }

        int triangleCount = 0;
        int n = (nullptr == indices) ? vertexCount : indexCount;
        switch (vmode) {
            case SkCanvas::kTriangles_VertexMode:
                triangleCount = n / 3;
                break;
            case SkCanvas::kTriangleStrip_VertexMode:
            case SkCanvas::kTriangleFan_VertexMode:
                triangleCount = n - 2;
                break;
        }

        VertState       state(vertexCount, indices, indexCount);
        VertState::Proc vertProc = state.chooseProc(vmode);

        //number of indices for lines per triangle with kLines
        indexCount = triangleCount * 6;

        SkAutoTDeleteArray<uint16_t> lineIndices(new uint16_t[indexCount]);
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
        fDrawContext->drawVertices(fClip,
                                   grPaint,
                                   *draw.fMatrix,
                                   kLines_GrPrimitiveType,
                                   vertexCount,
                                   vertices,
                                   texs,
                                   colors,
                                   lineIndices.get(),
                                   indexCount);
        return;
    }

    GrPrimitiveType primType = gVertexMode2PrimitiveType[vmode];

    SkAutoSTMalloc<128, GrColor> convertedColors(0);
    if (colors) {
        // need to convert byte order and from non-PM to PM. TODO: Keep unpremul until after
        // interpolation.
        convertedColors.reset(vertexCount);
        for (int i = 0; i < vertexCount; ++i) {
            convertedColors[i] = SkColorToPremulGrColor(colors[i]);
        }
        colors = convertedColors.get();
    }
    GrPaint grPaint;
    if (texs && paint.getShader()) {
        if (colors) {
            // When there are texs and colors the shader and colors are combined using xmode. A null
            // xmode is defined to mean modulate.
            SkXfermode::Mode colorMode;
            if (xmode) {
                if (!xmode->asMode(&colorMode)) {
                    return;
                }
            } else {
                colorMode = SkXfermode::kModulate_Mode;
            }
            if (!SkPaintToGrPaintWithXfermode(this->context(), paint, *draw.fMatrix, colorMode,
                                              false, this->surfaceProps().allowSRGBInputs(),
                                              &grPaint)) {
                return;
            }
        } else {
            // We have a shader, but no colors to blend it against.
            if (!SkPaintToGrPaint(this->context(), paint, *draw.fMatrix,
                                  this->surfaceProps().allowSRGBInputs(), &grPaint)) {
                return;
            }
        }
    } else {
        if (colors) {
            // We have colors, but either have no shader or no texture coords (which implies that
            // we should ignore the shader).
            if (!SkPaintToGrPaintWithPrimitiveColor(this->context(), paint,
                                                    this->surfaceProps().allowSRGBInputs(),
                                                    &grPaint)) {
                return;
            }
        } else {
            // No colors and no shaders. Just draw with the paint color.
            if (!SkPaintToGrPaintNoShader(this->context(), paint,
                                          this->surfaceProps().allowSRGBInputs(), &grPaint)) {
                return;
            }
        }
    }

    fDrawContext->drawVertices(fClip,
                               grPaint,
                               *draw.fMatrix,
                               primType,
                               vertexCount,
                               vertices,
                               texs,
                               colors,
                               indices,
                               indexCount);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawAtlas(const SkDraw& draw, const SkImage* atlas, const SkRSXform xform[],
                            const SkRect texRect[], const SkColor colors[], int count,
                            SkXfermode::Mode mode, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    if (paint.isAntiAlias()) {
        this->INHERITED::drawAtlas(draw, atlas, xform, texRect, colors, count, mode, paint);
        return;
    }

    CHECK_SHOULD_DRAW(draw);
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawText", fContext);

    SkPaint p(paint);
    p.setShader(atlas->makeShader(SkShader::kClamp_TileMode, SkShader::kClamp_TileMode));

    GrPaint grPaint;
    if (colors) {
        if (!SkPaintToGrPaintWithXfermode(this->context(), p, *draw.fMatrix, mode, true,
                                          this->surfaceProps().allowSRGBInputs(), &grPaint)) {
            return;
        }
    } else {
        if (!SkPaintToGrPaint(this->context(), p, *draw.fMatrix,
                              this->surfaceProps().allowSRGBInputs(), &grPaint)) {
            return;
        }
    }

    SkDEBUGCODE(this->validate();)
    fDrawContext->drawAtlas(fClip, grPaint, *draw.fMatrix, count, xform, texRect, colors);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawText(const SkDraw& draw, const void* text,
                           size_t byteLength, SkScalar x, SkScalar y,
                           const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    CHECK_SHOULD_DRAW(draw);
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawText", fContext);

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), paint, *draw.fMatrix,
                          this->surfaceProps().allowSRGBInputs(), &grPaint)) {
        return;
    }

    SkDEBUGCODE(this->validate();)

    fDrawContext->drawText(fClip, grPaint, paint, *draw.fMatrix,
                           (const char *)text, byteLength, x, y, draw.fClip->getBounds());
}

void SkGpuDevice::drawPosText(const SkDraw& draw, const void* text, size_t byteLength,
                              const SkScalar pos[], int scalarsPerPos,
                              const SkPoint& offset, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawPosText", fContext);
    CHECK_SHOULD_DRAW(draw);

    GrPaint grPaint;
    if (!SkPaintToGrPaint(this->context(), paint, *draw.fMatrix,
                          this->surfaceProps().allowSRGBInputs(), &grPaint)) {
        return;
    }

    SkDEBUGCODE(this->validate();)

    fDrawContext->drawPosText(fClip, grPaint, paint, *draw.fMatrix,
                              (const char *)text, byteLength, pos, scalarsPerPos, offset,
                              draw.fClip->getBounds());
}

void SkGpuDevice::drawTextBlob(const SkDraw& draw, const SkTextBlob* blob, SkScalar x, SkScalar y,
                               const SkPaint& paint, SkDrawFilter* drawFilter) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice", "drawTextBlob", fContext);
    CHECK_SHOULD_DRAW(draw);

    SkDEBUGCODE(this->validate();)

    fDrawContext->drawTextBlob(fClip, paint, *draw.fMatrix,
                               blob, x, y, drawFilter, draw.fClip->getBounds());
}

///////////////////////////////////////////////////////////////////////////////

bool SkGpuDevice::onShouldDisableLCD(const SkPaint& paint) const {
    return GrTextUtils::ShouldDisableLCD(paint);
}

void SkGpuDevice::flush() {
    ASSERT_SINGLE_OWNER

    fRenderTarget->prepareForExternalIO();
}

///////////////////////////////////////////////////////////////////////////////

SkBaseDevice* SkGpuDevice::onCreateDevice(const CreateInfo& cinfo, const SkPaint*) {
    ASSERT_SINGLE_OWNER
    GrSurfaceDesc desc;
    desc.fConfig = fRenderTarget->config();
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = cinfo.fInfo.width();
    desc.fHeight = cinfo.fInfo.height();
    desc.fSampleCnt = fRenderTarget->desc().fSampleCnt;

    SkAutoTUnref<GrTexture> texture;
    // Skia's convention is to only clear a device if it is non-opaque.
    InitContents init = cinfo.fInfo.isOpaque() ? kUninit_InitContents : kClear_InitContents;

    // layers are never draw in repeat modes, so we can request an approx
    // match and ignore any padding.
    if (kNever_TileUsage == cinfo.fTileUsage) {
        texture.reset(fContext->textureProvider()->createApproxTexture(desc));
    } else {
        texture.reset(fContext->textureProvider()->createTexture(desc, SkBudgeted::kYes));
    }

    if (texture) {
        SkSurfaceProps props(this->surfaceProps().flags(), cinfo.fPixelGeometry);
        return SkGpuDevice::Create(
            texture->asRenderTarget(), cinfo.fInfo.width(), cinfo.fInfo.height(), &props, init);
    } else {
        SkErrorInternals::SetError( kInternalError_SkError,
                                    "---- failed to create gpu device texture [%d %d]\n",
                                    cinfo.fInfo.width(), cinfo.fInfo.height());
        return nullptr;
    }
}

sk_sp<SkSurface> SkGpuDevice::makeSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    ASSERT_SINGLE_OWNER
    // TODO: Change the signature of newSurface to take a budgeted parameter.
    static const SkBudgeted kBudgeted = SkBudgeted::kNo;
    return SkSurface::MakeRenderTarget(fContext, kBudgeted, info, fRenderTarget->desc().fSampleCnt,
                                       &props);
}

bool SkGpuDevice::EXPERIMENTAL_drawPicture(SkCanvas* mainCanvas, const SkPicture* mainPicture,
                                           const SkMatrix* matrix, const SkPaint* paint) {
    ASSERT_SINGLE_OWNER
#ifndef SK_IGNORE_GPU_LAYER_HOISTING
    // todo: should handle this natively
    if (paint) {
        return false;
    }

    const SkBigPicture::AccelData* data = nullptr;
    if (const SkBigPicture* bp = mainPicture->asSkBigPicture()) {
        data = bp->accelData();
    }
    if (!data) {
        return false;
    }

    const SkLayerInfo *gpuData = static_cast<const SkLayerInfo*>(data);
    if (0 == gpuData->numBlocks()) {
        return false;
    }

    SkTDArray<GrHoistedLayer> atlasedNeedRendering, atlasedRecycled;

    SkIRect iBounds;
    if (!mainCanvas->getClipDeviceBounds(&iBounds)) {
        return false;
    }

    SkRect clipBounds = SkRect::Make(iBounds);

    SkMatrix initialMatrix = mainCanvas->getTotalMatrix();

    GrLayerHoister::Begin(fContext);

    GrLayerHoister::FindLayersToAtlas(fContext, mainPicture,
                                      initialMatrix,
                                      clipBounds,
                                      &atlasedNeedRendering, &atlasedRecycled,
                                      fRenderTarget->numColorSamples());

    GrLayerHoister::DrawLayersToAtlas(fContext, atlasedNeedRendering);

    SkTDArray<GrHoistedLayer> needRendering, recycled;

    SkAutoCanvasMatrixPaint acmp(mainCanvas, matrix, paint, mainPicture->cullRect());

    GrLayerHoister::FindLayersToHoist(fContext, mainPicture,
                                      initialMatrix,
                                      clipBounds,
                                      &needRendering, &recycled,
                                      fRenderTarget->numColorSamples());

    GrLayerHoister::DrawLayers(fContext, needRendering);

    // Render the entire picture using new layers
    GrRecordReplaceDraw(mainPicture, mainCanvas, fContext->getLayerCache(),
                        initialMatrix, nullptr);

    GrLayerHoister::UnlockLayers(fContext, needRendering);
    GrLayerHoister::UnlockLayers(fContext, recycled);
    GrLayerHoister::UnlockLayers(fContext, atlasedNeedRendering);
    GrLayerHoister::UnlockLayers(fContext, atlasedRecycled);
    GrLayerHoister::End(fContext);

    return true;
#else
    return false;
#endif
}

SkImageFilter::Cache* SkGpuDevice::NewImageFilterCache() {
    return SkImageFilter::Cache::Create(kDefaultImageFilterCacheSize);
}

SkImageFilter::Cache* SkGpuDevice::getImageFilterCache() {
    ASSERT_SINGLE_OWNER
    // We always return a transient cache, so it is freed after each
    // filter traversal.
    return SkGpuDevice::NewImageFilterCache();
}

#endif
