/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGpuDevice.h"

#include "GrBlurUtils.h"
#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrFontScaler.h"
#include "GrGpu.h"
#include "GrGpuResourcePriv.h"
#include "GrLayerHoister.h"
#include "GrRecordReplaceDraw.h"
#include "GrStrokeInfo.h"
#include "GrTextContext.h"
#include "GrTracing.h"
#include "SkCanvasPriv.h"
#include "SkDrawProcs.h"
#include "SkErrorInternals.h"
#include "SkGlyphCache.h"
#include "SkGrTexturePixelRef.h"
#include "SkImage_Base.h"
#include "SkImageFilter.h"
#include "SkLayerInfo.h"
#include "SkMaskFilter.h"
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
#include "effects/GrBicubicEffect.h"
#include "effects/GrDashingEffect.h"
#include "effects/GrSimpleTextureEffect.h"
#include "effects/GrTextureDomain.h"

#if SK_SUPPORT_GPU

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

// This constant represents the screen alignment criterion in texels for
// requiring texture domain clamping to prevent color bleeding when drawing
// a sub region of a larger source image.
#define COLOR_BLEED_TOLERANCE 0.001f

#define DO_DEFERRED_CLEAR()             \
    do {                                \
        if (fNeedClear) {               \
            this->clearAll();           \
        }                               \
    } while (false)                     \

///////////////////////////////////////////////////////////////////////////////

#define CHECK_FOR_ANNOTATION(paint) \
    do { if (paint.getAnnotation()) { return; } } while (0)

///////////////////////////////////////////////////////////////////////////////

// Helper for turning a bitmap into a texture. If the bitmap is GrTexture backed this
// just accesses the backing GrTexture. Otherwise, it creates a cached texture
// representation and releases it in the destructor.
class AutoBitmapTexture : public SkNoncopyable {
public:
    AutoBitmapTexture() {}

    AutoBitmapTexture(GrContext* context,
                      const SkBitmap& bitmap,
                      const GrTextureParams* params,
                      GrTexture** texture) {
        SkASSERT(texture);
        *texture = this->set(context, bitmap, params);
    }

    GrTexture* set(GrContext* context,
                   const SkBitmap& bitmap,
                   const GrTextureParams* params) {
        // Either get the texture directly from the bitmap, or else use the cache and
        // remember to unref it.
        if (GrTexture* bmpTexture = bitmap.getTexture()) {
            fTexture.reset(NULL);
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

struct GrSkDrawProcs : public SkDrawProcs {
public:
    GrContext* fContext;
    GrTextContext* fTextContext;
    GrFontScaler* fFontScaler;  // cached in the skia glyphcache
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
        return NULL;
    }
    unsigned flags;
    if (!CheckAlphaTypeAndGetFlags(NULL, init, &flags)) {
        return NULL;
    }
    return SkNEW_ARGS(SkGpuDevice, (rt, width, height, props, flags));
}

SkGpuDevice* SkGpuDevice::Create(GrContext* context, SkSurface::Budgeted budgeted,
                                 const SkImageInfo& info, int sampleCount,
                                 const SkSurfaceProps* props, InitContents init) {
    unsigned flags;
    if (!CheckAlphaTypeAndGetFlags(&info, init, &flags)) {
        return NULL;
    }

    SkAutoTUnref<GrRenderTarget> rt(CreateRenderTarget(context, budgeted, info,  sampleCount));
    if (NULL == rt) {
        return NULL;
    }

    return SkNEW_ARGS(SkGpuDevice, (rt, info.width(), info.height(), props, flags));
}

SkGpuDevice::SkGpuDevice(GrRenderTarget* rt, int width, int height,
                         const SkSurfaceProps* props, unsigned flags)
    : INHERITED(SkSurfacePropsCopyOrDefault(props))
{
    fDrawProcs = NULL;

    fContext = SkRef(rt->getContext());
    fNeedClear = SkToBool(flags & kNeedClear_Flag);
    fOpaque = SkToBool(flags & kIsOpaque_Flag);

    fRenderTarget = SkRef(rt);

    SkAlphaType at = fOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
    SkImageInfo info = rt->surfacePriv().info(at).makeWH(width, height);
    SkPixelRef* pr = SkNEW_ARGS(SkGrPixelRef, (info, rt));
    fLegacyBitmap.setInfo(info);
    fLegacyBitmap.setPixelRef(pr)->unref();

    fDrawContext.reset(SkRef(fContext->drawContext(&this->surfaceProps())));
}

GrRenderTarget* SkGpuDevice::CreateRenderTarget(GrContext* context, SkSurface::Budgeted budgeted,
                                                const SkImageInfo& origInfo, int sampleCount) {
    if (kUnknown_SkColorType == origInfo.colorType() ||
        origInfo.width() < 0 || origInfo.height() < 0) {
        return NULL;
    }

    if (!context) {
        return NULL;
    }

    SkColorType ct = origInfo.colorType();
    SkAlphaType at = origInfo.alphaType();
    if (kRGB_565_SkColorType == ct) {
        at = kOpaque_SkAlphaType;  // force this setting
    } else if (ct != kBGRA_8888_SkColorType && ct != kRGBA_8888_SkColorType) {
        // Fall back from whatever ct was to default of kRGBA or kBGRA which is aliased as kN32
        ct = kN32_SkColorType;
    }
    if (kOpaque_SkAlphaType != at) {
        at = kPremul_SkAlphaType;  // force this setting
    }
    const SkImageInfo info = SkImageInfo::Make(origInfo.width(), origInfo.height(), ct, at);

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = info.width();
    desc.fHeight = info.height();
    desc.fConfig = SkImageInfo2GrPixelConfig(info);
    desc.fSampleCnt = sampleCount;
    GrTexture* texture = context->textureProvider()->createTexture(
        desc, SkToBool(budgeted), NULL, 0);
    if (NULL == texture) {
        return NULL;
    }
    SkASSERT(NULL != texture->asRenderTarget());
    return texture->asRenderTarget();
}

SkGpuDevice::~SkGpuDevice() {
    if (fDrawProcs) {
        delete fDrawProcs;
    }

    fRenderTarget->unref();
    fContext->unref();
}

///////////////////////////////////////////////////////////////////////////////

bool SkGpuDevice::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                               int x, int y) {
    DO_DEFERRED_CLEAR();

    // TODO: teach fRenderTarget to take ImageInfo directly to specify the src pixels
    GrPixelConfig config = SkImageInfo2GrPixelConfig(dstInfo);
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
    // TODO: teach fRenderTarget to take ImageInfo directly to specify the src pixels
    GrPixelConfig config = SkImageInfo2GrPixelConfig(info);
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
    DO_DEFERRED_CLEAR();
    return fLegacyBitmap;
}

bool SkGpuDevice::onAccessPixels(SkPixmap* pmap) {
    DO_DEFERRED_CLEAR();
    // For compatibility with clients the know we're backed w/ a bitmap, and want to inspect its
    // genID. When we can hide/remove that fact, we can eliminate this call to notify.
    // ... ugh.
    fLegacyBitmap.notifyPixelsChanged();
    return false;
}

void SkGpuDevice::onAttachToCanvas(SkCanvas* canvas) {
    INHERITED::onAttachToCanvas(canvas);

    // Canvas promises that this ptr is valid until onDetachFromCanvas is called
    fClipStack.reset(SkRef(canvas->getClipStack()));
}

void SkGpuDevice::onDetachFromCanvas() {
    INHERITED::onDetachFromCanvas();
    fClip.reset();
    fClipStack.reset(NULL);
}

// call this every draw call, to ensure that the context reflects our state,
// and not the state from some other canvas/device
void SkGpuDevice::prepareDraw(const SkDraw& draw) {
    SkASSERT(fClipStack.get());

    SkASSERT(draw.fClipStack && draw.fClipStack == fClipStack);

    fClip.setClipStack(fClipStack, &this->getOrigin());

    DO_DEFERRED_CLEAR();
}

GrRenderTarget* SkGpuDevice::accessRenderTarget() {
    DO_DEFERRED_CLEAR();
    return fRenderTarget;
}

void SkGpuDevice::clearAll() {
    GrColor color = 0;
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::clearAll", fContext);
    SkIRect rect = SkIRect::MakeWH(this->width(), this->height());
    fDrawContext->clear(fRenderTarget, &rect, color, true);
    fNeedClear = false;
}

void SkGpuDevice::replaceRenderTarget(bool shouldRetainContent) {
    // Caller must have accessed the render target, because it knows the rt must be replaced.
    SkASSERT(!fNeedClear);

    SkSurface::Budgeted budgeted =
            fRenderTarget->resourcePriv().isBudgeted() ? SkSurface::kYes_Budgeted
                                                       : SkSurface::kNo_Budgeted;

    SkAutoTUnref<GrRenderTarget> newRT(CreateRenderTarget(
        fRenderTarget->getContext(), budgeted, this->imageInfo(), fRenderTarget->desc().fSampleCnt));

    if (NULL == newRT) {
        return;
    }

    if (shouldRetainContent) {
        if (fRenderTarget->wasDestroyed()) {
            return;
        }
        this->context()->copySurface(newRT, fRenderTarget);
    }

    SkASSERT(fRenderTarget != newRT);

    fRenderTarget->unref();
    fRenderTarget = newRT.detach();

#ifdef SK_DEBUG
    SkImageInfo info = fRenderTarget->surfacePriv().info(fOpaque ? kOpaque_SkAlphaType :
                                                                   kPremul_SkAlphaType);
    SkASSERT(info == fLegacyBitmap.info());
#endif
    SkPixelRef* pr = SkNEW_ARGS(SkGrPixelRef, (fLegacyBitmap.info(), fRenderTarget));
    fLegacyBitmap.setPixelRef(pr)->unref();

    fDrawContext.reset(SkRef(fRenderTarget->getContext()->drawContext(&this->surfaceProps())));
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawPaint(const SkDraw& draw, const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw);
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::drawPaint", fContext);

    GrPaint grPaint;
    if (!SkPaint2GrPaint(this->context(), fRenderTarget, paint, *draw.fMatrix, true, &grPaint)) {
        return;
    }

    fDrawContext->drawPaint(fRenderTarget, fClip, grPaint, *draw.fMatrix);
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
    CHECK_FOR_ANNOTATION(paint);
    CHECK_SHOULD_DRAW(draw);

    SkScalar width = paint.getStrokeWidth();
    if (width < 0) {
        return;
    }

    if (paint.getPathEffect() && 2 == count && SkCanvas::kLines_PointMode == mode) {
        GrStrokeInfo strokeInfo(paint, SkPaint::kStroke_Style);
        GrPaint grPaint;
        if (!SkPaint2GrPaint(this->context(), fRenderTarget, paint, *draw.fMatrix, true,
                             &grPaint)) {
            return;
        }
        SkPath path;
        path.setIsVolatile(true);
        path.moveTo(pts[0]);
        path.lineTo(pts[1]);
        fDrawContext->drawPath(fRenderTarget, fClip, grPaint, *draw.fMatrix, path, strokeInfo);
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
    if (!SkPaint2GrPaint(this->context(), fRenderTarget, paint, *draw.fMatrix, true, &grPaint)) {
        return;
    }

    fDrawContext->drawVertices(fRenderTarget,
                               fClip,
                               grPaint,
                               *draw.fMatrix,
                               gPointMode2PrimtiveType[mode],
                               SkToS32(count),
                               (SkPoint*)pts,
                               NULL,
                               NULL,
                               NULL,
                               0);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawRect(const SkDraw& draw, const SkRect& rect,
                           const SkPaint& paint) {
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::drawRect", fContext);

    CHECK_FOR_ANNOTATION(paint);
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
    if (paint.getMaskFilter() ||
        paint.getStyle() == SkPaint::kStrokeAndFill_Style) { // we can't both stroke and fill rects
        usePath = true;
    }

    if (!usePath && paint.isAntiAlias() && !draw.fMatrix->rectStaysRect()) {
        usePath = true;
    }

    GrStrokeInfo strokeInfo(paint);

    const SkPathEffect* pe = paint.getPathEffect();
    if (!usePath && pe && !strokeInfo.isDashed()) {
        usePath = true;
    }

    if (usePath) {
        SkPath path;
        path.setIsVolatile(true);
        path.addRect(rect);
        this->drawPath(draw, path, paint, NULL, true);
        return;
    }

    GrPaint grPaint;
    if (!SkPaint2GrPaint(this->context(), fRenderTarget, paint, *draw.fMatrix, true, &grPaint)) {
        return;
    }

    fDrawContext->drawRect(fRenderTarget, fClip, grPaint, *draw.fMatrix, rect, &strokeInfo);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawRRect(const SkDraw& draw, const SkRRect& rect,
                            const SkPaint& paint) {
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::drawRRect", fContext);
    CHECK_FOR_ANNOTATION(paint);
    CHECK_SHOULD_DRAW(draw);

    GrPaint grPaint;
    if (!SkPaint2GrPaint(this->context(), fRenderTarget, paint, *draw.fMatrix, true, &grPaint)) {
        return;
    }

    GrStrokeInfo strokeInfo(paint);
    if (paint.getMaskFilter()) {
        // try to hit the fast path for drawing filtered round rects

        SkRRect devRRect;
        if (rect.transform(*draw.fMatrix, &devRRect)) {
            if (devRRect.allCornersCircular()) {
                SkRect maskRect;
                if (paint.getMaskFilter()->canFilterMaskGPU(devRRect.rect(),
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
                                                                        fRenderTarget,
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

    bool usePath = false;

    if (paint.getMaskFilter()) {
        usePath = true;
    } else {
        const SkPathEffect* pe = paint.getPathEffect();
        if (pe && !strokeInfo.isDashed()) {
            usePath = true;
        }
    }


    if (usePath) {
        SkPath path;
        path.setIsVolatile(true);
        path.addRRect(rect);
        this->drawPath(draw, path, paint, NULL, true);
        return;
    }

    fDrawContext->drawRRect(fRenderTarget, fClip, grPaint, *draw.fMatrix, rect, strokeInfo);
}

void SkGpuDevice::drawDRRect(const SkDraw& draw, const SkRRect& outer,
                             const SkRRect& inner, const SkPaint& paint) {
    SkStrokeRec stroke(paint);
    if (stroke.isFillStyle()) {

        CHECK_FOR_ANNOTATION(paint);
        CHECK_SHOULD_DRAW(draw);

        GrPaint grPaint;
        if (!SkPaint2GrPaint(this->context(), fRenderTarget, paint, *draw.fMatrix, true,
                             &grPaint)) {
            return;
        }

        if (NULL == paint.getMaskFilter() && NULL == paint.getPathEffect()) {
            fDrawContext->drawDRRect(fRenderTarget, fClip, grPaint, *draw.fMatrix, outer, inner);
            return;
        }
    }

    SkPath path;
    path.setIsVolatile(true);
    path.addRRect(outer);
    path.addRRect(inner);
    path.setFillType(SkPath::kEvenOdd_FillType);

    this->drawPath(draw, path, paint, NULL, true);
}


/////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawOval(const SkDraw& draw, const SkRect& oval,
                           const SkPaint& paint) {
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::drawOval", fContext);
    CHECK_FOR_ANNOTATION(paint);
    CHECK_SHOULD_DRAW(draw);

    GrStrokeInfo strokeInfo(paint);

    bool usePath = false;
    // some basic reasons we might need to call drawPath...
    if (paint.getMaskFilter()) {
        usePath = true;
    } else {
        const SkPathEffect* pe = paint.getPathEffect();
        if (pe && !strokeInfo.isDashed()) {
            usePath = true;
        }
    }

    if (usePath) {
        SkPath path;
        path.setIsVolatile(true);
        path.addOval(oval);
        this->drawPath(draw, path, paint, NULL, true);
        return;
    }

    GrPaint grPaint;
    if (!SkPaint2GrPaint(this->context(), fRenderTarget, paint, *draw.fMatrix, true, &grPaint)) {
        return;
    }

    fDrawContext->drawOval(fRenderTarget, fClip, grPaint, *draw.fMatrix, oval, strokeInfo);
}

#include "SkMaskFilter.h"

///////////////////////////////////////////////////////////////////////////////

static SkBitmap wrap_texture(GrTexture* texture, int width, int height) {
    SkBitmap result;
    result.setInfo(SkImageInfo::MakeN32Premul(width, height));
    result.setPixelRef(SkNEW_ARGS(SkGrPixelRef, (result.info(), texture)))->unref();
    return result;
}

void SkGpuDevice::drawPath(const SkDraw& draw, const SkPath& origSrcPath,
                           const SkPaint& paint, const SkMatrix* prePathMatrix,
                           bool pathIsMutable) {
    CHECK_FOR_ANNOTATION(paint);
    CHECK_SHOULD_DRAW(draw);
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::drawPath", fContext);

    GrBlurUtils::drawPathWithMaskFilter(fContext, fDrawContext, fRenderTarget,
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

static int determine_tile_size(const SkBitmap& bitmap, const SkIRect& src, int maxTileSize) {
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
                                       const SkBitmap& bitmap,
                                       const SkRect* srcRectPtr,
                                       SkIRect* clippedSrcIRect) {
    clip.getConservativeBounds(rt, clippedSrcIRect, NULL);
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
    SkIRect bmpBounds = SkIRect::MakeWH(bitmap.width(), bitmap.height());
    if (!clippedSrcIRect->intersect(bmpBounds)) {
        clippedSrcIRect->setEmpty();
    }
}

bool SkGpuDevice::shouldTileBitmap(const SkBitmap& bitmap,
                                   const SkMatrix& viewMatrix,
                                   const GrTextureParams& params,
                                   const SkRect* srcRectPtr,
                                   int maxTileSize,
                                   int* tileSize,
                                   SkIRect* clippedSrcRect) const {
    // if bitmap is explictly texture backed then just use the texture
    if (bitmap.getTexture()) {
        return false;
    }

    // if it's larger than the max tile size, then we have no choice but tiling.
    if (bitmap.width() > maxTileSize || bitmap.height() > maxTileSize) {
        determine_clipped_src_rect(fRenderTarget, fClip, viewMatrix, bitmap,
                                   srcRectPtr, clippedSrcRect);
        *tileSize = determine_tile_size(bitmap, *clippedSrcRect, maxTileSize);
        return true;
    }

    if (bitmap.width() * bitmap.height() < 4 * kBmpSmallTileSize * kBmpSmallTileSize) {
        return false;
    }

    // if the entire texture is already in our cache then no reason to tile it
    if (GrIsBitmapInCache(fContext, bitmap, &params)) {
        return false;
    }

    // At this point we know we could do the draw by uploading the entire bitmap
    // as a texture. However, if the texture would be large compared to the
    // cache size and we don't require most of it for this draw then tile to
    // reduce the amount of upload and cache spill.

    // assumption here is that sw bitmap size is a good proxy for its size as
    // a texture
    size_t bmpSize = bitmap.getSize();
    size_t cacheSize;
    fContext->getResourceCacheLimits(NULL, &cacheSize);
    if (bmpSize < cacheSize / 2) {
        return false;
    }

    // Figure out how much of the src we will need based on the src rect and clipping.
    determine_clipped_src_rect(fRenderTarget, fClip, viewMatrix, bitmap, srcRectPtr,
                               clippedSrcRect);
    *tileSize = kBmpSmallTileSize; // already know whole bitmap fits in one max sized tile.
    size_t usedTileBytes = get_tile_count(*clippedSrcRect, kBmpSmallTileSize) *
                           kBmpSmallTileSize * kBmpSmallTileSize;

    return usedTileBytes < 2 * bmpSize;
}

void SkGpuDevice::drawBitmap(const SkDraw& origDraw,
                             const SkBitmap& bitmap,
                             const SkMatrix& m,
                             const SkPaint& paint) {
    SkMatrix concat;
    SkTCopyOnFirstWrite<SkDraw> draw(origDraw);
    if (!m.isIdentity()) {
        concat.setConcat(*draw->fMatrix, m);
        draw.writable()->fMatrix = &concat;
    }
    this->drawBitmapCommon(*draw, bitmap, NULL, NULL, paint, SkCanvas::kStrict_SrcRectConstraint);
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

static bool has_aligned_samples(const SkRect& srcRect,
                                const SkRect& transformedRect) {
    // detect pixel disalignment
    if (SkScalarAbs(SkScalarRoundToScalar(transformedRect.left()) -
            transformedRect.left()) < COLOR_BLEED_TOLERANCE &&
        SkScalarAbs(SkScalarRoundToScalar(transformedRect.top()) -
            transformedRect.top()) < COLOR_BLEED_TOLERANCE &&
        SkScalarAbs(transformedRect.width() - srcRect.width()) <
            COLOR_BLEED_TOLERANCE &&
        SkScalarAbs(transformedRect.height() - srcRect.height()) <
            COLOR_BLEED_TOLERANCE) {
        return true;
    }
    return false;
}

static bool may_color_bleed(const SkRect& srcRect,
                            const SkRect& transformedRect,
                            const SkMatrix& m,
                            bool isMSAA) {
    // Only gets called if has_aligned_samples returned false.
    // So we can assume that sampling is axis aligned but not texel aligned.
    SkASSERT(!has_aligned_samples(srcRect, transformedRect));
    SkRect innerSrcRect(srcRect), innerTransformedRect,
        outerTransformedRect(transformedRect);
    if (isMSAA) {
        innerSrcRect.inset(SK_Scalar1, SK_Scalar1);
    } else {
        innerSrcRect.inset(SK_ScalarHalf, SK_ScalarHalf);
    }
    m.mapRect(&innerTransformedRect, innerSrcRect);

    // The gap between outerTransformedRect and innerTransformedRect
    // represents the projection of the source border area, which is
    // problematic for color bleeding.  We must check whether any
    // destination pixels sample the border area.
    outerTransformedRect.inset(COLOR_BLEED_TOLERANCE, COLOR_BLEED_TOLERANCE);
    innerTransformedRect.outset(COLOR_BLEED_TOLERANCE, COLOR_BLEED_TOLERANCE);
    SkIRect outer, inner;
    outerTransformedRect.round(&outer);
    innerTransformedRect.round(&inner);
    // If the inner and outer rects round to the same result, it means the
    // border does not overlap any pixel centers. Yay!
    return inner != outer;
}

static bool needs_texture_domain(const SkBitmap& bitmap,
                                 const SkRect& srcRect,
                                 GrTextureParams &params,
                                 const SkMatrix& contextMatrix,
                                 bool bicubic,
                                 bool isMSAA) {
    bool needsTextureDomain = false;
    GrTexture* tex = bitmap.getTexture();
    int width = tex ? tex->width() : bitmap.width();
    int height = tex ? tex->height() : bitmap.height();

    if (bicubic || params.filterMode() != GrTextureParams::kNone_FilterMode) {
        // Need texture domain if drawing a sub rect
        needsTextureDomain = srcRect.width() < width ||
                             srcRect.height() < height;
        if (!bicubic && needsTextureDomain && contextMatrix.rectStaysRect()) {
            // sampling is axis-aligned
            SkRect transformedRect;
            contextMatrix.mapRect(&transformedRect, srcRect);

            if (has_aligned_samples(srcRect, transformedRect)) {
                params.setFilterMode(GrTextureParams::kNone_FilterMode);
                needsTextureDomain = false;
            } else {
                needsTextureDomain = may_color_bleed(srcRect, transformedRect,
                                                     contextMatrix, isMSAA);
            }
        }
    }
    return needsTextureDomain;
}

void SkGpuDevice::drawBitmapCommon(const SkDraw& draw,
                                   const SkBitmap& bitmap,
                                   const SkRect* srcRectPtr,
                                   const SkSize* dstSizePtr,
                                   const SkPaint& paint,
                                   SkCanvas::SrcRectConstraint constraint) {
    CHECK_SHOULD_DRAW(draw);

    SkRect srcRect;
    SkSize dstSize;
    // If there is no src rect, or the src rect contains the entire bitmap then we're effectively
    // in the (easier) bleed case, so update flags.
    if (NULL == srcRectPtr) {
        SkScalar w = SkIntToScalar(bitmap.width());
        SkScalar h = SkIntToScalar(bitmap.height());
        dstSize.fWidth = w;
        dstSize.fHeight = h;
        srcRect.set(0, 0, w, h);
    } else {
        SkASSERT(dstSizePtr);
        srcRect = *srcRectPtr;
        dstSize = *dstSizePtr;
    }
    GrTexture* tex = bitmap.getTexture();
    int width = tex ? tex->width() : bitmap.width();
    int height = tex ? tex->height() : bitmap.height();
    if (srcRect.fLeft <= 0 && srcRect.fTop <= 0 &&
        srcRect.fRight >= width && srcRect.fBottom >= height) {
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }

    // If the render target is not msaa and draw is antialiased, we call
    // drawRect instead of drawing on the render target directly.
    // FIXME: the tiled bitmap code path doesn't currently support
    // anti-aliased edges, we work around that for now by drawing directly
    // if the image size exceeds maximum texture size.
    int maxTextureSize = fContext->caps()->maxTextureSize();
    bool directDraw = fRenderTarget->isUnifiedMultisampled() ||
                      !paint.isAntiAlias() ||
                      bitmap.width() > maxTextureSize ||
                      bitmap.height() > maxTextureSize;

    // we check whether dst rect are pixel aligned
    if (!directDraw) {
        bool staysRect = draw.fMatrix->rectStaysRect();

        if (staysRect) {
            SkRect rect;
            SkRect dstRect = SkRect::MakeXYWH(0, 0, dstSize.fWidth, dstSize.fHeight);
            draw.fMatrix->mapRect(&rect, dstRect);
            const SkScalar *scalars = rect.asScalars();
            bool isDstPixelAligned = true;
            for (int i = 0; i < 4; i++) {
                if (!SkScalarIsInt(scalars[i])) {
                    isDstPixelAligned = false;
                    break;
                }
            }

            if (isDstPixelAligned)
                directDraw = true;
        }
    }

    if (paint.getMaskFilter() || !directDraw) {
        // Convert the bitmap to a shader so that the rect can be drawn
        // through drawRect, which supports mask filters.
        SkBitmap        tmp;    // subset of bitmap, if necessary
        const SkBitmap* bitmapPtr = &bitmap;
        SkMatrix localM;
        if (srcRectPtr) {
            localM.setTranslate(-srcRectPtr->fLeft, -srcRectPtr->fTop);
            localM.postScale(dstSize.fWidth / srcRectPtr->width(),
                             dstSize.fHeight / srcRectPtr->height());
            // In bleed mode we position and trim the bitmap based on the src rect which is
            // already accounted for in 'm' and 'srcRect'. In clamp mode we need to chop out
            // the desired portion of the bitmap and then update 'm' and 'srcRect' to
            // compensate.
            if (SkCanvas::kStrict_SrcRectConstraint == constraint) {
                SkIRect iSrc;
                srcRect.roundOut(&iSrc);

                SkPoint offset = SkPoint::Make(SkIntToScalar(iSrc.fLeft),
                                               SkIntToScalar(iSrc.fTop));

                if (!bitmap.extractSubset(&tmp, iSrc)) {
                    return;     // extraction failed
                }
                bitmapPtr = &tmp;
                srcRect.offset(-offset.fX, -offset.fY);

                // The source rect has changed so update the matrix
                localM.preTranslate(offset.fX, offset.fY);
            }
        } else {
            localM.reset();
        }

        SkPaint paintWithShader(paint);
        paintWithShader.setShader(SkShader::CreateBitmapShader(*bitmapPtr,
            SkShader::kClamp_TileMode, SkShader::kClamp_TileMode, &localM))->unref();
        SkRect dstRect = {0, 0, dstSize.fWidth, dstSize.fHeight};
        this->drawRect(draw, dstRect, paintWithShader);

        return;
    }

    // If there is no mask filter than it is OK to handle the src rect -> dst rect scaling using
    // the view matrix rather than a local matrix.
    SkMatrix m;
    m.setScale(dstSize.fWidth / srcRect.width(),
               dstSize.fHeight / srcRect.height());
    SkMatrix viewM = *draw.fMatrix;
    viewM.preConcat(m);

    GrTextureParams params;
    SkFilterQuality paintFilterQuality = paint.getFilterQuality();
    GrTextureParams::FilterMode textureFilterMode;

    bool doBicubic = false;

    switch(paintFilterQuality) {
        case kNone_SkFilterQuality:
            textureFilterMode = GrTextureParams::kNone_FilterMode;
            break;
        case kLow_SkFilterQuality:
            textureFilterMode = GrTextureParams::kBilerp_FilterMode;
            break;
        case kMedium_SkFilterQuality:
            if (viewM.getMinScale() < SK_Scalar1) {
                textureFilterMode = GrTextureParams::kMipMap_FilterMode;
            } else {
                // Don't trigger MIP level generation unnecessarily.
                textureFilterMode = GrTextureParams::kBilerp_FilterMode;
            }
            break;
        case kHigh_SkFilterQuality:
            // Minification can look bad with the bicubic effect.
            doBicubic =
                GrBicubicEffect::ShouldUseBicubic(viewM, &textureFilterMode);
            break;
        default:
            SkErrorInternals::SetError( kInvalidPaint_SkError,
                                        "Sorry, I don't understand the filtering "
                                        "mode you asked for.  Falling back to "
                                        "MIPMaps.");
            textureFilterMode = GrTextureParams::kMipMap_FilterMode;
            break;
    }

    int tileFilterPad;
    if (doBicubic) {
        tileFilterPad = GrBicubicEffect::kFilterTexelPad;
    } else if (GrTextureParams::kNone_FilterMode == textureFilterMode) {
        tileFilterPad = 0;
    } else {
        tileFilterPad = 1;
    }
    params.setFilterMode(textureFilterMode);

    int maxTileSize = fContext->caps()->maxTextureSize() - 2 * tileFilterPad;
    int tileSize;

    SkIRect clippedSrcRect;
    if (this->shouldTileBitmap(bitmap, viewM, params, srcRectPtr, maxTileSize, &tileSize,
                               &clippedSrcRect)) {
        this->drawTiledBitmap(bitmap, viewM, srcRect, clippedSrcRect, params, paint, constraint,
                              tileSize, doBicubic);
    } else {
        // take the simple case
        bool needsTextureDomain = needs_texture_domain(bitmap,
                                                       srcRect,
                                                       params,
                                                       viewM,
                                                       doBicubic,
                                                       fRenderTarget->isUnifiedMultisampled());
        this->internalDrawBitmap(bitmap,
                                 viewM,
                                 srcRect,
                                 params,
                                 paint,
                                 constraint,
                                 doBicubic,
                                 needsTextureDomain);
    }
}

// Break 'bitmap' into several tiles to draw it since it has already
// been determined to be too large to fit in VRAM
void SkGpuDevice::drawTiledBitmap(const SkBitmap& bitmap,
                                  const SkMatrix& viewMatrix,
                                  const SkRect& srcRect,
                                  const SkIRect& clippedSrcIRect,
                                  const GrTextureParams& params,
                                  const SkPaint& paint,
                                  SkCanvas::SrcRectConstraint constraint,
                                  int tileSize,
                                  bool bicubic) {
    // The following pixel lock is technically redundant, but it is desirable
    // to lock outside of the tile loop to prevent redecoding the whole image
    // at each tile in cases where 'bitmap' holds an SkDiscardablePixelRef that
    // is larger than the limit of the discardable memory pool.
    SkAutoLockPixels alp(bitmap);
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
                bool needsTextureDomain = needs_texture_domain(
                                                         bitmap, srcRect, paramsTemp,
                                                         viewM, bicubic,
                                                         fRenderTarget->isUnifiedMultisampled());
                this->internalDrawBitmap(tmpB,
                                         viewM,
                                         tileR,
                                         paramsTemp,
                                         paint,
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
    SkASSERT(bitmap.width() <= fContext->caps()->maxTextureSize() &&
             bitmap.height() <= fContext->caps()->maxTextureSize());

    GrTexture* texture;
    AutoBitmapTexture abt(fContext, bitmap, &params, &texture);
    if (NULL == texture) {
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

    SkRect textureDomain = SkRect::MakeEmpty();

    // Construct a GrPaint by setting the bitmap texture as the first effect and then configuring
    // the rest from the SkPaint.
    GrPaint grPaint;
    SkAutoTUnref<GrFragmentProcessor> fp;

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
            fp.reset(GrBicubicEffect::Create(grPaint.getProcessorDataManager(), texture,
                                             SkMatrix::I(), textureDomain));
        } else {
            fp.reset(GrTextureDomainEffect::Create(grPaint.getProcessorDataManager(),
                                                   texture,
                                                   SkMatrix::I(),
                                                   textureDomain,
                                                   GrTextureDomain::kClamp_Mode,
                                                   params.filterMode()));
        }
    } else if (bicubic) {
        SkASSERT(GrTextureParams::kNone_FilterMode == params.filterMode());
        SkShader::TileMode tileModes[2] = { params.getTileModeX(), params.getTileModeY() };
        fp.reset(GrBicubicEffect::Create(grPaint.getProcessorDataManager(), texture, SkMatrix::I(),
                                         tileModes));
    } else {
        fp.reset(GrSimpleTextureEffect::Create(grPaint.getProcessorDataManager(), texture,
                                               SkMatrix::I(), params));
    }

    grPaint.addColorProcessor(fp);
    bool alphaOnly = !(kAlpha_8_SkColorType == bitmap.colorType());
    GrColor paintColor = (alphaOnly) ? SkColor2GrColorJustAlpha(paint.getColor()) :
                                       SkColor2GrColor(paint.getColor());
    if (!SkPaint2GrPaintNoShader(this->context(), fRenderTarget, paint, paintColor, false,
                                 &grPaint)) {
        return;
    }

    fDrawContext->drawNonAARectToRect(fRenderTarget, fClip, grPaint, viewMatrix, dstRect,
                                      paintRect);
}

bool SkGpuDevice::filterTexture(GrContext* context, GrTexture* texture,
                                int width, int height,
                                const SkImageFilter* filter,
                                const SkImageFilter::Context& ctx,
                                SkBitmap* result, SkIPoint* offset) {
    SkASSERT(filter);

    SkImageFilter::Proxy proxy(this);

    if (filter->canFilterImageGPU()) {
        return filter->filterImageGPU(&proxy, wrap_texture(texture, width, height),
                                      ctx, result, offset);
    } else {
        return false;
    }
}

void SkGpuDevice::drawSprite(const SkDraw& draw, const SkBitmap& bitmap,
                             int left, int top, const SkPaint& paint) {
    // drawSprite is defined to be in device coords.
    CHECK_SHOULD_DRAW(draw);

    SkAutoLockPixels alp(bitmap, !bitmap.getTexture());
    if (!bitmap.getTexture() && !bitmap.readyToDraw()) {
        return;
    }

    int w = bitmap.width();
    int h = bitmap.height();

    GrTexture* texture;
    // draw sprite uses the default texture params
    AutoBitmapTexture abt(fContext, bitmap, NULL, &texture);
    if (!texture) {
        return;
    }

    SkImageFilter* filter = paint.getImageFilter();
    // This bitmap will own the filtered result as a texture.
    SkBitmap filteredBitmap;

    if (filter) {
        SkIPoint offset = SkIPoint::Make(0, 0);
        SkMatrix matrix(*draw.fMatrix);
        matrix.postTranslate(SkIntToScalar(-left), SkIntToScalar(-top));
        SkIRect clipBounds = SkIRect::MakeWH(bitmap.width(), bitmap.height());
        SkAutoTUnref<SkImageFilter::Cache> cache(getImageFilterCache());
        // This cache is transient, and is freed (along with all its contained
        // textures) when it goes out of scope.
        SkImageFilter::Context ctx(matrix, clipBounds, cache);
        if (this->filterTexture(fContext, texture, w, h, filter, ctx, &filteredBitmap,
                                &offset)) {
            texture = (GrTexture*) filteredBitmap.getTexture();
            w = filteredBitmap.width();
            h = filteredBitmap.height();
            left += offset.x();
            top += offset.y();
        } else {
            return;
        }
    }

    GrPaint grPaint;
    grPaint.addColorTextureProcessor(texture, SkMatrix::I());

    if (!SkPaint2GrPaintNoShader(this->context(), fRenderTarget, paint,
                                 SkColor2GrColorJustAlpha(paint.getColor()), false, &grPaint)) {
        return;
    }

    fDrawContext->drawNonAARectToRect(fRenderTarget,
                                      fClip,
                                      grPaint,
                                      SkMatrix::I(),
                                      SkRect::MakeXYWH(SkIntToScalar(left),
                                                       SkIntToScalar(top),
                                                       SkIntToScalar(w),
                                                       SkIntToScalar(h)),
                                      SkRect::MakeXYWH(0,
                                                       0,
                                                       SK_Scalar1 * w / texture->width(),
                                                       SK_Scalar1 * h / texture->height()));
}

void SkGpuDevice::drawBitmapRect(const SkDraw& origDraw, const SkBitmap& bitmap,
                                 const SkRect* src, const SkRect& dst,
                                 const SkPaint& paint, SkCanvas::SrcRectConstraint constraint) {
    SkMatrix    matrix;
    SkRect      bitmapBounds, tmpSrc;

    bitmapBounds.set(0, 0,
                     SkIntToScalar(bitmap.width()),
                     SkIntToScalar(bitmap.height()));

    // Compute matrix from the two rectangles
    if (src) {
        tmpSrc = *src;
    } else {
        tmpSrc = bitmapBounds;
    }

    matrix.setRectToRect(tmpSrc, dst, SkMatrix::kFill_ScaleToFit);

    // clip the tmpSrc to the bounds of the bitmap. No check needed if src==null.
    if (src) {
        if (!bitmapBounds.contains(tmpSrc)) {
            if (!tmpSrc.intersect(bitmapBounds)) {
                return; // nothing to draw
            }
        }
    }

    SkRect tmpDst;
    matrix.mapRect(&tmpDst, tmpSrc);

    SkTCopyOnFirstWrite<SkDraw> draw(origDraw);
    if (0 != tmpDst.fLeft || 0 != tmpDst.fTop) {
        // Translate so that tempDst's top left is at the origin.
        matrix = *origDraw.fMatrix;
        matrix.preTranslate(tmpDst.fLeft, tmpDst.fTop);
        draw.writable()->fMatrix = &matrix;
    }
    SkSize dstSize;
    dstSize.fWidth = tmpDst.width();
    dstSize.fHeight = tmpDst.height();

    this->drawBitmapCommon(*draw, bitmap, &tmpSrc, &dstSize, paint, constraint);
}

void SkGpuDevice::drawDevice(const SkDraw& draw, SkBaseDevice* device,
                             int x, int y, const SkPaint& paint) {
    // clear of the source device must occur before CHECK_SHOULD_DRAW
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::drawDevice", fContext);
    SkGpuDevice* dev = static_cast<SkGpuDevice*>(device);

    // TODO: If the source device covers the whole of this device, we could
    // omit fNeedsClear -related flushing.
    // TODO: if source needs clear, we could maybe omit the draw fully.

    // drawDevice is defined to be in device coords.
    CHECK_SHOULD_DRAW(draw);

    GrRenderTarget* devRT = dev->accessRenderTarget();
    GrTexture* devTex;
    if (NULL == (devTex = devRT->asTexture())) {
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
        SkIRect clipBounds = SkIRect::MakeWH(devTex->width(), devTex->height());
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
    grPaint.addColorTextureProcessor(devTex, SkMatrix::I());

    if (!SkPaint2GrPaintNoShader(this->context(), fRenderTarget, paint,
                                 SkColor2GrColorJustAlpha(paint.getColor()), false, &grPaint)) {
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

    fDrawContext->drawNonAARectToRect(fRenderTarget, fClip, grPaint, SkMatrix::I(), dstRect,
                                      srcRect);
}

bool SkGpuDevice::canHandleImageFilter(const SkImageFilter* filter) {
    return filter->canFilterImageGPU();
}

bool SkGpuDevice::filterImage(const SkImageFilter* filter, const SkBitmap& src,
                              const SkImageFilter::Context& ctx,
                              SkBitmap* result, SkIPoint* offset) {
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
    AutoBitmapTexture abt(fContext, src, NULL, &texture);
    if (!texture) {
        return false;
    }

    return this->filterTexture(fContext, texture, src.width(), src.height(),
                               filter, ctx, result, offset);
}

static bool wrap_as_bm(const SkImage* image, SkBitmap* bm) {
    GrTexture* tex = as_IB(image)->getTexture();
    if (tex) {
        GrWrapTextureInBitmap(tex, image->width(), image->height(), image->isOpaque(), bm);
        return true;
    } else {
        return as_IB(image)->getROPixels(bm);
    }
}

void SkGpuDevice::drawImage(const SkDraw& draw, const SkImage* image, SkScalar x, SkScalar y,
                            const SkPaint& paint) {
    SkBitmap bm;
    if (wrap_as_bm(image, &bm)) {
        this->drawBitmap(draw, bm, SkMatrix::MakeTrans(x, y), paint);
    }
}

void SkGpuDevice::drawImageRect(const SkDraw& draw, const SkImage* image, const SkRect* src,
                                const SkRect& dst, const SkPaint& paint,
                                SkCanvas::SrcRectConstraint constraint) {
    SkBitmap bm;
    if (wrap_as_bm(image, &bm)) {
        this->drawBitmapRect(draw, bm, src, dst, paint, constraint);
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
    CHECK_SHOULD_DRAW(draw);
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::drawVertices", fContext);

    const uint16_t* outIndices;
    SkAutoTDeleteArray<uint16_t> outAlloc(NULL);
    GrPrimitiveType primType;
    GrPaint grPaint;

    // If both textures and vertex-colors are NULL, strokes hairlines with the paint's color.
    if ((NULL == texs || NULL == paint.getShader()) && NULL == colors) {

        texs = NULL;

        SkPaint copy(paint);
        copy.setStyle(SkPaint::kStroke_Style);
        copy.setStrokeWidth(0);

        // we ignore the shader if texs is null.
        if (!SkPaint2GrPaintNoShader(this->context(), fRenderTarget, copy,
                                     SkColor2GrColor(copy.getColor()), NULL == colors, &grPaint)) {
            return;
        }

        primType = kLines_GrPrimitiveType;
        int triangleCount = 0;
        int n = (NULL == indices) ? vertexCount : indexCount;
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

        outAlloc.reset(SkNEW_ARRAY(uint16_t, indexCount));
        outIndices = outAlloc.get();
        uint16_t* auxIndices = outAlloc.get();
        int i = 0;
        while (vertProc(&state)) {
            auxIndices[i]     = state.f0;
            auxIndices[i + 1] = state.f1;
            auxIndices[i + 2] = state.f1;
            auxIndices[i + 3] = state.f2;
            auxIndices[i + 4] = state.f2;
            auxIndices[i + 5] = state.f0;
            i += 6;
        }
    } else {
        outIndices = indices;
        primType = gVertexMode2PrimitiveType[vmode];

        if (NULL == texs || NULL == paint.getShader()) {
            if (!SkPaint2GrPaintNoShader(this->context(), fRenderTarget, paint,
                                         SkColor2GrColor(paint.getColor()),
                                         NULL == colors, &grPaint)) {
                return;
            }
        } else {
            if (!SkPaint2GrPaint(this->context(), fRenderTarget, paint, *draw.fMatrix,
                                 NULL == colors, &grPaint)) {
                return;
            }
        }
    }

#if 0
    if (xmode && texs && colors) {
        if (!SkXfermode::IsMode(xmode, SkXfermode::kModulate_Mode)) {
            SkDebugf("Unsupported vertex-color/texture xfer mode.\n");
            return;
        }
    }
#endif

    SkAutoSTMalloc<128, GrColor> convertedColors(0);
    if (colors) {
        // need to convert byte order and from non-PM to PM
        convertedColors.reset(vertexCount);
        SkColor color;
        for (int i = 0; i < vertexCount; ++i) {
            color = colors[i];
            if (paint.getAlpha() != 255) {
                color = SkColorSetA(color, SkMulDiv255Round(SkColorGetA(color), paint.getAlpha()));
            }
            convertedColors[i] = SkColor2GrColor(color);
        }
        colors = convertedColors.get();
    }
    fDrawContext->drawVertices(fRenderTarget,
                               fClip,
                               grPaint,
                               *draw.fMatrix,
                               primType,
                               vertexCount,
                               vertices,
                               texs,
                               colors,
                               outIndices,
                               indexCount);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawAtlas(const SkDraw& draw, const SkImage* atlas, const SkRSXform xform[],
                            const SkRect texRect[], const SkColor colors[], int count,
                            SkXfermode::Mode mode, const SkPaint& paint) {
    if (paint.isAntiAlias()) {
        this->INHERITED::drawAtlas(draw, atlas, xform, texRect, colors, count, mode, paint);
        return;
    }

    CHECK_SHOULD_DRAW(draw);
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::drawText", fContext);
    
    SkPaint p(paint);
    p.setShader(atlas->newShader(SkShader::kClamp_TileMode, SkShader::kClamp_TileMode))->unref();

    GrPaint grPaint;
    if (!SkPaint2GrPaint(this->context(), fRenderTarget, p, *draw.fMatrix, !colors, &grPaint)) {
        return;
    }
    
    SkDEBUGCODE(this->validate();)
    
#if 0
    if (colors) {
        if (SkXfermode::kModulate_Mode != mode) {
            SkDebugf("Unsupported vertex-color/texture xfer mode.\n");
            return;
        }
    }
#endif
    
    fDrawContext->drawAtlas(fRenderTarget, fClip, grPaint, *draw.fMatrix,
                            count, xform, texRect, colors);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawText(const SkDraw& draw, const void* text,
                           size_t byteLength, SkScalar x, SkScalar y,
                           const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw);
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::drawText", fContext);

    GrPaint grPaint;
    if (!SkPaint2GrPaint(this->context(), fRenderTarget, paint, *draw.fMatrix, true, &grPaint)) {
        return;
    }

    SkDEBUGCODE(this->validate();)

    fDrawContext->drawText(fRenderTarget, fClip, grPaint, paint, *draw.fMatrix,
                           (const char *)text, byteLength, x, y, draw.fClip->getBounds());
}

void SkGpuDevice::drawPosText(const SkDraw& draw, const void* text, size_t byteLength,
                              const SkScalar pos[], int scalarsPerPos,
                              const SkPoint& offset, const SkPaint& paint) {
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::drawPosText", fContext);
    CHECK_SHOULD_DRAW(draw);

    GrPaint grPaint;
    if (!SkPaint2GrPaint(this->context(), fRenderTarget, paint, *draw.fMatrix, true, &grPaint)) {
        return;
    }

    SkDEBUGCODE(this->validate();)

    fDrawContext->drawPosText(fRenderTarget, fClip, grPaint, paint, *draw.fMatrix,
                              (const char *)text, byteLength, pos, scalarsPerPos, offset,
                              draw.fClip->getBounds());
}

void SkGpuDevice::drawTextBlob(const SkDraw& draw, const SkTextBlob* blob, SkScalar x, SkScalar y,
                               const SkPaint& paint, SkDrawFilter* drawFilter) {
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::drawTextBlob", fContext);
    CHECK_SHOULD_DRAW(draw);

    SkDEBUGCODE(this->validate();)

    fDrawContext->drawTextBlob(fRenderTarget, fClip, paint, *draw.fMatrix,
                               blob, x, y, drawFilter, draw.fClip->getBounds());
}

///////////////////////////////////////////////////////////////////////////////

bool SkGpuDevice::onShouldDisableLCD(const SkPaint& paint) const {
    return GrTextContext::ShouldDisableLCD(paint);
}

void SkGpuDevice::flush() {
    DO_DEFERRED_CLEAR();
    fRenderTarget->prepareForExternalIO();
}

///////////////////////////////////////////////////////////////////////////////

SkBaseDevice* SkGpuDevice::onCreateDevice(const CreateInfo& cinfo, const SkPaint*) {
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
        texture.reset(fContext->textureProvider()->createTexture(desc, true));
    }

    if (texture) {
        SkSurfaceProps props(this->surfaceProps().flags(), cinfo.fPixelGeometry);
        return SkGpuDevice::Create(
            texture->asRenderTarget(), cinfo.fInfo.width(), cinfo.fInfo.height(), &props, init);
    } else {
        SkErrorInternals::SetError( kInternalError_SkError,
                                    "---- failed to create gpu device texture [%d %d]\n",
                                    cinfo.fInfo.width(), cinfo.fInfo.height());
        return NULL;
    }
}

SkSurface* SkGpuDevice::newSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    // TODO: Change the signature of newSurface to take a budgeted parameter.
    static const SkSurface::Budgeted kBudgeted = SkSurface::kNo_Budgeted;
    return SkSurface::NewRenderTarget(fContext, kBudgeted, info, fRenderTarget->desc().fSampleCnt,
                                      &props);
}

bool SkGpuDevice::EXPERIMENTAL_drawPicture(SkCanvas* mainCanvas, const SkPicture* mainPicture,
                                           const SkMatrix* matrix, const SkPaint* paint) {
#ifndef SK_IGNORE_GPU_LAYER_HOISTING
    // todo: should handle this natively
    if (paint) {
        return false;
    }

    const SkBigPicture::AccelData* data = NULL;
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
                        initialMatrix, NULL);

    GrLayerHoister::UnlockLayers(fContext, needRendering);
    GrLayerHoister::UnlockLayers(fContext, recycled);
    GrLayerHoister::UnlockLayers(fContext, atlasedNeedRendering);
    GrLayerHoister::UnlockLayers(fContext, atlasedRecycled);

    return true;
#else
    return false;
#endif
}

SkImageFilter::Cache* SkGpuDevice::getImageFilterCache() {
    // We always return a transient cache, so it is freed after each
    // filter traversal.
    return SkImageFilter::Cache::Create(kDefaultImageFilterCacheSize);
}

#endif
