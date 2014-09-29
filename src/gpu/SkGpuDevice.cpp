/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGpuDevice.h"

#include "effects/GrBicubicEffect.h"
#include "effects/GrDashingEffect.h"
#include "effects/GrTextureDomain.h"
#include "effects/GrSimpleTextureEffect.h"

#include "GrContext.h"
#include "GrBitmapTextContext.h"
#include "GrDistanceFieldTextContext.h"
#include "GrLayerCache.h"
#include "GrLayerHoister.h"
#include "GrPictureUtils.h"
#include "GrRecordReplaceDraw.h"
#include "GrStrokeInfo.h"
#include "GrTracing.h"

#include "SkGrTexturePixelRef.h"

#include "SkDeviceImageFilterProxy.h"
#include "SkDrawProcs.h"
#include "SkGlyphCache.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkPathEffect.h"
#include "SkPicture.h"
#include "SkPictureData.h"
#include "SkRecord.h"
#include "SkRRect.h"
#include "SkStroke.h"
#include "SkSurface.h"
#include "SkTLazy.h"
#include "SkUtils.h"
#include "SkVertState.h"
#include "SkXfermode.h"
#include "SkErrorInternals.h"

enum { kDefaultImageFilterCacheSize = 32 * 1024 * 1024 };

#define CACHE_COMPATIBLE_DEVICE_TEXTURES 1

#if 0
    extern bool (*gShouldDrawProc)();
    #define CHECK_SHOULD_DRAW(draw, forceI)                     \
        do {                                                    \
            if (gShouldDrawProc && !gShouldDrawProc()) return;  \
            this->prepareDraw(draw, forceI);                    \
        } while (0)
#else
    #define CHECK_SHOULD_DRAW(draw, forceI) this->prepareDraw(draw, forceI)
#endif

// This constant represents the screen alignment criterion in texels for
// requiring texture domain clamping to prevent color bleeding when drawing
// a sub region of a larger source image.
#define COLOR_BLEED_TOLERANCE 0.001f

#define DO_DEFERRED_CLEAR()             \
    do {                                \
        if (fNeedClear) {               \
            this->clear(SK_ColorTRANSPARENT); \
        }                               \
    } while (false)                     \

///////////////////////////////////////////////////////////////////////////////

#define CHECK_FOR_ANNOTATION(paint) \
    do { if (paint.getAnnotation()) { return; } } while (0)

///////////////////////////////////////////////////////////////////////////////


class SkGpuDevice::SkAutoCachedTexture : public ::SkNoncopyable {
public:
    SkAutoCachedTexture()
        : fDevice(NULL)
        , fTexture(NULL) {
    }

    SkAutoCachedTexture(SkGpuDevice* device,
                        const SkBitmap& bitmap,
                        const GrTextureParams* params,
                        GrTexture** texture)
        : fDevice(NULL)
        , fTexture(NULL) {
        SkASSERT(texture);
        *texture = this->set(device, bitmap, params);
    }

    ~SkAutoCachedTexture() {
        if (fTexture) {
            GrUnlockAndUnrefCachedBitmapTexture(fTexture);
        }
    }

    GrTexture* set(SkGpuDevice* device,
                   const SkBitmap& bitmap,
                   const GrTextureParams* params) {
        if (fTexture) {
            GrUnlockAndUnrefCachedBitmapTexture(fTexture);
            fTexture = NULL;
        }
        fDevice = device;
        GrTexture* result = (GrTexture*)bitmap.getTexture();
        if (NULL == result) {
            // Cannot return the native texture so look it up in our cache
            fTexture = GrLockAndRefCachedBitmapTexture(device->context(), bitmap, params);
            result = fTexture;
        }
        return result;
    }

private:
    SkGpuDevice* fDevice;
    GrTexture*   fTexture;
};

///////////////////////////////////////////////////////////////////////////////

struct GrSkDrawProcs : public SkDrawProcs {
public:
    GrContext* fContext;
    GrTextContext* fTextContext;
    GrFontScaler* fFontScaler;  // cached in the skia glyphcache
};

///////////////////////////////////////////////////////////////////////////////

SkGpuDevice* SkGpuDevice::Create(GrSurface* surface, const SkSurfaceProps& props, unsigned flags) {
    SkASSERT(surface);
    if (NULL == surface->asRenderTarget() || surface->wasDestroyed()) {
        return NULL;
    }
    return SkNEW_ARGS(SkGpuDevice, (surface, props, flags));
}

SkGpuDevice::SkGpuDevice(GrSurface* surface, const SkSurfaceProps& props, unsigned flags) {

    fDrawProcs = NULL;

    fContext = SkRef(surface->getContext());

    fNeedClear = flags & kNeedClear_Flag;

    fRenderTarget = SkRef(surface->asRenderTarget());

    SkPixelRef* pr = SkNEW_ARGS(SkGrPixelRef,
                                (surface->info(), surface, SkToBool(flags & kCached_Flag)));
    fLegacyBitmap.setInfo(surface->info());
    fLegacyBitmap.setPixelRef(pr)->unref();

    this->setPixelGeometry(props.pixelGeometry());

    bool useDFFonts = !!(flags & kDFFonts_Flag);
    fMainTextContext = fContext->createTextContext(fRenderTarget, this->getLeakyProperties(), useDFFonts);
    fFallbackTextContext = SkNEW_ARGS(GrBitmapTextContext, (fContext, this->getLeakyProperties()));
}

SkGpuDevice* SkGpuDevice::Create(GrContext* context, const SkImageInfo& origInfo,
                                 const SkSurfaceProps& props, int sampleCount) {
    if (kUnknown_SkColorType == origInfo.colorType() ||
        origInfo.width() < 0 || origInfo.height() < 0) {
        return NULL;
    }

    SkColorType ct = origInfo.colorType();
    SkAlphaType at = origInfo.alphaType();
    // TODO: perhaps we can loosen this check now that colortype is more detailed
    // e.g. can we support both RGBA and BGRA here?
    if (kRGB_565_SkColorType == ct) {
        at = kOpaque_SkAlphaType;  // force this setting
    } else {
        ct = kN32_SkColorType;
        if (kOpaque_SkAlphaType != at) {
            at = kPremul_SkAlphaType;  // force this setting
        }
    }
    const SkImageInfo info = SkImageInfo::Make(origInfo.width(), origInfo.height(), ct, at);

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit;
    desc.fWidth = info.width();
    desc.fHeight = info.height();
    desc.fConfig = SkImageInfo2GrPixelConfig(info);
    desc.fSampleCnt = sampleCount;

    SkAutoTUnref<GrTexture> texture(context->createUncachedTexture(desc, NULL, 0));
    if (!texture.get()) {
        return NULL;
    }

    return SkNEW_ARGS(SkGpuDevice, (texture.get(), props));
}

SkGpuDevice::~SkGpuDevice() {
    if (fDrawProcs) {
        delete fDrawProcs;
    }

    delete fMainTextContext;
    delete fFallbackTextContext;

    // The GrContext takes a ref on the target. We don't want to cause the render
    // target to be unnecessarily kept alive.
    if (fContext->getRenderTarget() == fRenderTarget) {
        fContext->setRenderTarget(NULL);
    }

    if (fContext->getClip() == &fClipData) {
        fContext->setClip(NULL);
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
    return fContext->readRenderTargetPixels(fRenderTarget, x, y, dstInfo.width(), dstInfo.height(),
                                            config, dstPixels, dstRowBytes, flags);
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

void SkGpuDevice::onAttachToCanvas(SkCanvas* canvas) {
    INHERITED::onAttachToCanvas(canvas);

    // Canvas promises that this ptr is valid until onDetachFromCanvas is called
    fClipData.fClipStack = canvas->getClipStack();
}

void SkGpuDevice::onDetachFromCanvas() {
    INHERITED::onDetachFromCanvas();
    fClipData.fClipStack = NULL;
}

// call this every draw call, to ensure that the context reflects our state,
// and not the state from some other canvas/device
void SkGpuDevice::prepareDraw(const SkDraw& draw, bool forceIdentity) {
    SkASSERT(fClipData.fClipStack);

    fContext->setRenderTarget(fRenderTarget);

    SkASSERT(draw.fClipStack && draw.fClipStack == fClipData.fClipStack);

    if (forceIdentity) {
        fContext->setIdentityMatrix();
    } else {
        fContext->setMatrix(*draw.fMatrix);
    }
    fClipData.fOrigin = this->getOrigin();

    fContext->setClip(&fClipData);

    DO_DEFERRED_CLEAR();
}

GrRenderTarget* SkGpuDevice::accessRenderTarget() {
    DO_DEFERRED_CLEAR();
    return fRenderTarget;
}

///////////////////////////////////////////////////////////////////////////////

SK_COMPILE_ASSERT(SkShader::kNone_BitmapType == 0, shader_type_mismatch);
SK_COMPILE_ASSERT(SkShader::kDefault_BitmapType == 1, shader_type_mismatch);
SK_COMPILE_ASSERT(SkShader::kRadial_BitmapType == 2, shader_type_mismatch);
SK_COMPILE_ASSERT(SkShader::kSweep_BitmapType == 3, shader_type_mismatch);
SK_COMPILE_ASSERT(SkShader::kTwoPointRadial_BitmapType == 4,
                  shader_type_mismatch);
SK_COMPILE_ASSERT(SkShader::kTwoPointConical_BitmapType == 5,
                  shader_type_mismatch);
SK_COMPILE_ASSERT(SkShader::kLinear_BitmapType == 6, shader_type_mismatch);
SK_COMPILE_ASSERT(SkShader::kLast_BitmapType == 6, shader_type_mismatch);

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::clear(SkColor color) {
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::clear", fContext);
    SkIRect rect = SkIRect::MakeWH(this->width(), this->height());
    fContext->clear(&rect, SkColor2GrColor(color), true, fRenderTarget);
    fNeedClear = false;
}

void SkGpuDevice::drawPaint(const SkDraw& draw, const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw, false);
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::drawPaint", fContext);

    GrPaint grPaint;
    SkPaint2GrPaintShader(this->context(), paint, true, &grPaint);

    fContext->drawPaint(grPaint);
}

// must be in SkCanvas::PointMode order
static const GrPrimitiveType gPointMode2PrimtiveType[] = {
    kPoints_GrPrimitiveType,
    kLines_GrPrimitiveType,
    kLineStrip_GrPrimitiveType
};

void SkGpuDevice::drawPoints(const SkDraw& draw, SkCanvas::PointMode mode,
                             size_t count, const SkPoint pts[], const SkPaint& paint) {
    CHECK_FOR_ANNOTATION(paint);
    CHECK_SHOULD_DRAW(draw, false);

    SkScalar width = paint.getStrokeWidth();
    if (width < 0) {
        return;
    }

    if (paint.getPathEffect() && 2 == count && SkCanvas::kLines_PointMode == mode) {
        GrStrokeInfo strokeInfo(paint, SkPaint::kStroke_Style);
        GrPaint grPaint;
        SkPaint2GrPaintShader(this->context(), paint, true, &grPaint);
        SkPath path;
        path.moveTo(pts[0]);
        path.lineTo(pts[1]);
        fContext->drawPath(grPaint, path, strokeInfo);
        return;
    }

    // we only handle hairlines and paints without path effects or mask filters,
    // else we let the SkDraw call our drawPath()
    if (width > 0 || paint.getPathEffect() || paint.getMaskFilter()) {
        draw.drawPoints(mode, count, pts, paint, true);
        return;
    }

    GrPaint grPaint;
    SkPaint2GrPaintShader(this->context(), paint, true, &grPaint);

    fContext->drawVertices(grPaint,
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
    CHECK_SHOULD_DRAW(draw, false);

    bool doStroke = paint.getStyle() != SkPaint::kFill_Style;
    SkScalar width = paint.getStrokeWidth();

    /*
        We have special code for hairline strokes, miter-strokes, bevel-stroke
        and fills. Anything else we just call our path code.
     */
    bool usePath = doStroke && width > 0 &&
                   (paint.getStrokeJoin() == SkPaint::kRound_Join ||
                    (paint.getStrokeJoin() == SkPaint::kBevel_Join && rect.isEmpty()));
    // another two reasons we might need to call drawPath...

    if (paint.getMaskFilter()) {
        usePath = true;
    }

    if (!usePath && paint.isAntiAlias() && !fContext->getMatrix().rectStaysRect()) {
#if defined(SHADER_AA_FILL_RECT) || !defined(IGNORE_ROT_AA_RECT_OPT)
        if (doStroke) {
#endif
            usePath = true;
#if defined(SHADER_AA_FILL_RECT) || !defined(IGNORE_ROT_AA_RECT_OPT)
        } else {
            usePath = !fContext->getMatrix().preservesRightAngles();
        }
#endif
    }
    // until we can both stroke and fill rectangles
    if (paint.getStyle() == SkPaint::kStrokeAndFill_Style) {
        usePath = true;
    }

    GrStrokeInfo strokeInfo(paint);

    const SkPathEffect* pe = paint.getPathEffect();
    if (!usePath && pe && !strokeInfo.isDashed()) {
        usePath = true;
    }

    if (usePath) {
        SkPath path;
        path.addRect(rect);
        this->drawPath(draw, path, paint, NULL, true);
        return;
    }

    GrPaint grPaint;
    SkPaint2GrPaintShader(this->context(), paint, true, &grPaint);

    fContext->drawRect(grPaint, rect, &strokeInfo);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawRRect(const SkDraw& draw, const SkRRect& rect,
                           const SkPaint& paint) {
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::drawRRect", fContext);
    CHECK_FOR_ANNOTATION(paint);
    CHECK_SHOULD_DRAW(draw, false);

    GrPaint grPaint;
    SkPaint2GrPaintShader(this->context(), paint, true, &grPaint);

    GrStrokeInfo strokeInfo(paint);
    if (paint.getMaskFilter()) {
        // try to hit the fast path for drawing filtered round rects

        SkRRect devRRect;
        if (rect.transform(fContext->getMatrix(), &devRRect)) {
            if (devRRect.allCornersCircular()) {
                SkRect maskRect;
                if (paint.getMaskFilter()->canFilterMaskGPU(devRRect.rect(),
                                            draw.fClip->getBounds(),
                                            fContext->getMatrix(),
                                            &maskRect)) {
                    SkIRect finalIRect;
                    maskRect.roundOut(&finalIRect);
                    if (draw.fClip->quickReject(finalIRect)) {
                        // clipped out
                        return;
                    }
                    if (paint.getMaskFilter()->directFilterRRectMaskGPU(fContext, &grPaint,
                                                                        strokeInfo.getStrokeRec(),
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
        path.addRRect(rect);
        this->drawPath(draw, path, paint, NULL, true);
        return;
    }

    fContext->drawRRect(grPaint, rect, strokeInfo);
}

void SkGpuDevice::drawDRRect(const SkDraw& draw, const SkRRect& outer,
                              const SkRRect& inner, const SkPaint& paint) {
    SkStrokeRec stroke(paint);
    if (stroke.isFillStyle()) {

        CHECK_FOR_ANNOTATION(paint);
        CHECK_SHOULD_DRAW(draw, false);

        GrPaint grPaint;
        SkPaint2GrPaintShader(this->context(), paint, true, &grPaint);

        if (NULL == paint.getMaskFilter() && NULL == paint.getPathEffect()) {
            fContext->drawDRRect(grPaint, outer, inner);
            return;
        }
    }

    SkPath path;
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
    CHECK_SHOULD_DRAW(draw, false);

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
        path.addOval(oval);
        this->drawPath(draw, path, paint, NULL, true);
        return;
    }

    GrPaint grPaint;
    SkPaint2GrPaintShader(this->context(), paint, true, &grPaint);

    fContext->drawOval(grPaint, oval, strokeInfo);
}

#include "SkMaskFilter.h"

///////////////////////////////////////////////////////////////////////////////

// helpers for applying mask filters
namespace {

// Draw a mask using the supplied paint. Since the coverage/geometry
// is already burnt into the mask this boils down to a rect draw.
// Return true if the mask was successfully drawn.
bool draw_mask(GrContext* context, const SkRect& maskRect,
               GrPaint* grp, GrTexture* mask) {
    GrContext::AutoMatrix am;
    if (!am.setIdentity(context, grp)) {
        return false;
    }

    SkMatrix matrix;
    matrix.setTranslate(-maskRect.fLeft, -maskRect.fTop);
    matrix.postIDiv(mask->width(), mask->height());

    grp->addCoverageProcessor(GrSimpleTextureEffect::Create(mask, matrix))->unref();
    context->drawRect(*grp, maskRect);
    return true;
}

bool draw_with_mask_filter(GrContext* context, const SkPath& devPath,
                           SkMaskFilter* filter, const SkRegion& clip,
                           GrPaint* grp, SkPaint::Style style) {
    SkMask  srcM, dstM;

    if (!SkDraw::DrawToMask(devPath, &clip.getBounds(), filter, &context->getMatrix(), &srcM,
                            SkMask::kComputeBoundsAndRenderImage_CreateMode, style)) {
        return false;
    }
    SkAutoMaskFreeImage autoSrc(srcM.fImage);

    if (!filter->filterMask(&dstM, srcM, context->getMatrix(), NULL)) {
        return false;
    }
    // this will free-up dstM when we're done (allocated in filterMask())
    SkAutoMaskFreeImage autoDst(dstM.fImage);

    if (clip.quickReject(dstM.fBounds)) {
        return false;
    }

    // we now have a device-aligned 8bit mask in dstM, ready to be drawn using
    // the current clip (and identity matrix) and GrPaint settings
    GrTextureDesc desc;
    desc.fWidth = dstM.fBounds.width();
    desc.fHeight = dstM.fBounds.height();
    desc.fConfig = kAlpha_8_GrPixelConfig;

    GrAutoScratchTexture ast(context, desc);
    GrTexture* texture = ast.texture();

    if (NULL == texture) {
        return false;
    }
    texture->writePixels(0, 0, desc.fWidth, desc.fHeight, desc.fConfig,
                               dstM.fImage, dstM.fRowBytes);

    SkRect maskRect = SkRect::Make(dstM.fBounds);

    return draw_mask(context, maskRect, grp, texture);
}

// Create a mask of 'devPath' and place the result in 'mask'. Return true on
// success; false otherwise.
bool create_mask_GPU(GrContext* context,
                     const SkRect& maskRect,
                     const SkPath& devPath,
                     const GrStrokeInfo& strokeInfo,
                     bool doAA,
                     GrAutoScratchTexture* mask) {
    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit;
    desc.fWidth = SkScalarCeilToInt(maskRect.width());
    desc.fHeight = SkScalarCeilToInt(maskRect.height());
    // We actually only need A8, but it often isn't supported as a
    // render target so default to RGBA_8888
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    if (context->isConfigRenderable(kAlpha_8_GrPixelConfig, false)) {
        desc.fConfig = kAlpha_8_GrPixelConfig;
    }

    mask->set(context, desc);
    if (NULL == mask->texture()) {
        return false;
    }

    GrTexture* maskTexture = mask->texture();
    SkRect clipRect = SkRect::MakeWH(maskRect.width(), maskRect.height());

    GrContext::AutoRenderTarget art(context, maskTexture->asRenderTarget());
    GrContext::AutoClip ac(context, clipRect);

    context->clear(NULL, 0x0, true);

    GrPaint tempPaint;
    if (doAA) {
        tempPaint.setAntiAlias(true);
        // AA uses the "coverage" stages on GrDrawTarget. Coverage with a dst
        // blend coeff of zero requires dual source blending support in order
        // to properly blend partially covered pixels. This means the AA
        // code path may not be taken. So we use a dst blend coeff of ISA. We
        // could special case AA draws to a dst surface with known alpha=0 to
        // use a zero dst coeff when dual source blending isn't available.
        tempPaint.setBlendFunc(kOne_GrBlendCoeff, kISC_GrBlendCoeff);
    }

    GrContext::AutoMatrix am;

    // Draw the mask into maskTexture with the path's top-left at the origin using tempPaint.
    SkMatrix translate;
    translate.setTranslate(-maskRect.fLeft, -maskRect.fTop);
    am.set(context, translate);
    context->drawPath(tempPaint, devPath, strokeInfo);
    return true;
}

SkBitmap wrap_texture(GrTexture* texture) {
    SkBitmap result;
    result.setInfo(texture->info());
    result.setPixelRef(SkNEW_ARGS(SkGrPixelRef, (result.info(), texture)))->unref();
    return result;
}

};

void SkGpuDevice::drawPath(const SkDraw& draw, const SkPath& origSrcPath,
                           const SkPaint& paint, const SkMatrix* prePathMatrix,
                           bool pathIsMutable) {
    CHECK_FOR_ANNOTATION(paint);
    CHECK_SHOULD_DRAW(draw, false);
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::drawPath", fContext);

    GrPaint grPaint;
    SkPaint2GrPaintShader(this->context(), paint, true, &grPaint);

    // If we have a prematrix, apply it to the path, optimizing for the case
    // where the original path can in fact be modified in place (even though
    // its parameter type is const).
    SkPath* pathPtr = const_cast<SkPath*>(&origSrcPath);
    SkTLazy<SkPath> tmpPath;
    SkTLazy<SkPath> effectPath;

    if (prePathMatrix) {
        SkPath* result = pathPtr;

        if (!pathIsMutable) {
            result = tmpPath.init();
            pathIsMutable = true;
        }
        // should I push prePathMatrix on our MV stack temporarily, instead
        // of applying it here? See SkDraw.cpp
        pathPtr->transform(*prePathMatrix, result);
        pathPtr = result;
    }
    // at this point we're done with prePathMatrix
    SkDEBUGCODE(prePathMatrix = (const SkMatrix*)0x50FF8001;)

    GrStrokeInfo strokeInfo(paint);
    SkPathEffect* pathEffect = paint.getPathEffect();
    const SkRect* cullRect = NULL;  // TODO: what is our bounds?
    SkStrokeRec* strokePtr = strokeInfo.getStrokeRecPtr();
    if (pathEffect && pathEffect->filterPath(effectPath.init(), *pathPtr, strokePtr,
                                             cullRect)) {
        pathPtr = effectPath.get();
        pathIsMutable = true;
        strokeInfo.removeDash();
    }

    const SkStrokeRec& stroke = strokeInfo.getStrokeRec();
    if (paint.getMaskFilter()) {
        if (!stroke.isHairlineStyle()) {
            SkPath* strokedPath = pathIsMutable ? pathPtr : tmpPath.init();
            if (stroke.applyToPath(strokedPath, *pathPtr)) {
                pathPtr = strokedPath;
                pathIsMutable = true;
                strokeInfo.setFillStyle();
            }
        }

        // avoid possibly allocating a new path in transform if we can
        SkPath* devPathPtr = pathIsMutable ? pathPtr : tmpPath.init();

        // transform the path into device space
        pathPtr->transform(fContext->getMatrix(), devPathPtr);

        SkRect maskRect;
        if (paint.getMaskFilter()->canFilterMaskGPU(devPathPtr->getBounds(),
                                                    draw.fClip->getBounds(),
                                                    fContext->getMatrix(),
                                                    &maskRect)) {
            // The context's matrix may change while creating the mask, so save the CTM here to
            // pass to filterMaskGPU.
            const SkMatrix ctm = fContext->getMatrix();

            SkIRect finalIRect;
            maskRect.roundOut(&finalIRect);
            if (draw.fClip->quickReject(finalIRect)) {
                // clipped out
                return;
            }

            if (paint.getMaskFilter()->directFilterMaskGPU(fContext, &grPaint,
                                                           stroke, *devPathPtr)) {
                // the mask filter was able to draw itself directly, so there's nothing
                // left to do.
                return;
            }

            GrAutoScratchTexture mask;

            if (create_mask_GPU(fContext, maskRect, *devPathPtr, strokeInfo,
                                grPaint.isAntiAlias(), &mask)) {
                GrTexture* filtered;

                if (paint.getMaskFilter()->filterMaskGPU(mask.texture(),
                                                         ctm, maskRect, &filtered, true)) {
                    // filterMaskGPU gives us ownership of a ref to the result
                    SkAutoTUnref<GrTexture> atu(filtered);

                    // If the scratch texture that we used as the filter src also holds the filter
                    // result then we must detach so that this texture isn't recycled for a later
                    // draw.
                    if (filtered == mask.texture()) {
                        mask.detach();
                        filtered->unref(); // detach transfers GrAutoScratchTexture's ref to us.
                    }

                    if (draw_mask(fContext, maskRect, &grPaint, filtered)) {
                        // This path is completely drawn
                        return;
                    }
                }
            }
        }

        // draw the mask on the CPU - this is a fallthrough path in case the
        // GPU path fails
        SkPaint::Style style = stroke.isHairlineStyle() ? SkPaint::kStroke_Style :
                                                          SkPaint::kFill_Style;
        draw_with_mask_filter(fContext, *devPathPtr, paint.getMaskFilter(),
                              *draw.fClip, &grPaint, style);
        return;
    }

    fContext->drawPath(grPaint, *pathPtr, strokeInfo);
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
static void determine_clipped_src_rect(const GrContext* context,
                                       const SkBitmap& bitmap,
                                       const SkRect* srcRectPtr,
                                       SkIRect* clippedSrcIRect) {
    const GrClipData* clip = context->getClip();
    clip->getConservativeBounds(context->getRenderTarget(), clippedSrcIRect, NULL);
    SkMatrix inv;
    if (!context->getMatrix().invert(&inv)) {
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
        determine_clipped_src_rect(fContext, bitmap, srcRectPtr, clippedSrcRect);
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
    determine_clipped_src_rect(fContext, bitmap, srcRectPtr, clippedSrcRect);
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
    this->drawBitmapCommon(*draw, bitmap, NULL, NULL, paint, SkCanvas::kNone_DrawBitmapRectFlag);
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
                            const SkMatrix& m) {
    // Only gets called if has_aligned_samples returned false.
    // So we can assume that sampling is axis aligned but not texel aligned.
    SkASSERT(!has_aligned_samples(srcRect, transformedRect));
    SkRect innerSrcRect(srcRect), innerTransformedRect,
        outerTransformedRect(transformedRect);
    innerSrcRect.inset(SK_ScalarHalf, SK_ScalarHalf);
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
                                 bool bicubic) {
    bool needsTextureDomain = false;

    if (bicubic || params.filterMode() != GrTextureParams::kNone_FilterMode) {
        // Need texture domain if drawing a sub rect
        needsTextureDomain = srcRect.width() < bitmap.width() ||
                             srcRect.height() < bitmap.height();
        if (!bicubic && needsTextureDomain && contextMatrix.rectStaysRect()) {
            // sampling is axis-aligned
            SkRect transformedRect;
            contextMatrix.mapRect(&transformedRect, srcRect);

            if (has_aligned_samples(srcRect, transformedRect)) {
                params.setFilterMode(GrTextureParams::kNone_FilterMode);
                needsTextureDomain = false;
            } else {
                needsTextureDomain = may_color_bleed(srcRect, transformedRect, contextMatrix);
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
                                   SkCanvas::DrawBitmapRectFlags flags) {
    CHECK_SHOULD_DRAW(draw, false);

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
        flags = (SkCanvas::DrawBitmapRectFlags) (flags | SkCanvas::kBleed_DrawBitmapRectFlag);
    } else {
        SkASSERT(dstSizePtr);
        srcRect = *srcRectPtr;
        dstSize = *dstSizePtr;
        if (srcRect.fLeft <= 0 && srcRect.fTop <= 0 &&
            srcRect.fRight >= bitmap.width() && srcRect.fBottom >= bitmap.height()) {
            flags = (SkCanvas::DrawBitmapRectFlags) (flags | SkCanvas::kBleed_DrawBitmapRectFlag);
        }
    }

    if (paint.getMaskFilter()){
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
            if (!(SkCanvas::kBleed_DrawBitmapRectFlag & flags)) {
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
    fContext->concatMatrix(m);

    GrTextureParams params;
    SkPaint::FilterLevel paintFilterLevel = paint.getFilterLevel();
    GrTextureParams::FilterMode textureFilterMode;

    bool doBicubic = false;

    switch(paintFilterLevel) {
        case SkPaint::kNone_FilterLevel:
            textureFilterMode = GrTextureParams::kNone_FilterMode;
            break;
        case SkPaint::kLow_FilterLevel:
            textureFilterMode = GrTextureParams::kBilerp_FilterMode;
            break;
        case SkPaint::kMedium_FilterLevel:
            if (fContext->getMatrix().getMinScale() < SK_Scalar1) {
                textureFilterMode = GrTextureParams::kMipMap_FilterMode;
            } else {
                // Don't trigger MIP level generation unnecessarily.
                textureFilterMode = GrTextureParams::kBilerp_FilterMode;
            }
            break;
        case SkPaint::kHigh_FilterLevel:
            // Minification can look bad with the bicubic effect.
            doBicubic =
                GrBicubicEffect::ShouldUseBicubic(fContext->getMatrix(), &textureFilterMode);
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

    int maxTileSize = fContext->getMaxTextureSize() - 2 * tileFilterPad;
    int tileSize;

    SkIRect clippedSrcRect;
    if (this->shouldTileBitmap(bitmap, params, srcRectPtr, maxTileSize, &tileSize,
                               &clippedSrcRect)) {
        this->drawTiledBitmap(bitmap, srcRect, clippedSrcRect, params, paint, flags, tileSize,
                              doBicubic);
    } else {
        // take the simple case
        bool needsTextureDomain = needs_texture_domain(bitmap,
                                                       srcRect,
                                                       params,
                                                       fContext->getMatrix(),
                                                       doBicubic);
        this->internalDrawBitmap(bitmap,
                                 srcRect,
                                 params,
                                 paint,
                                 flags,
                                 doBicubic,
                                 needsTextureDomain);
    }
}

// Break 'bitmap' into several tiles to draw it since it has already
// been determined to be too large to fit in VRAM
void SkGpuDevice::drawTiledBitmap(const SkBitmap& bitmap,
                                  const SkRect& srcRect,
                                  const SkIRect& clippedSrcIRect,
                                  const GrTextureParams& params,
                                  const SkPaint& paint,
                                  SkCanvas::DrawBitmapRectFlags flags,
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
            SkMatrix tmpM;
            GrContext::AutoMatrix am;
            tmpM.setTranslate(offset.fX - srcRect.fLeft, offset.fY - srcRect.fTop);
            am.setPreConcat(fContext, tmpM);

            if (SkPaint::kNone_FilterLevel != paint.getFilterLevel() || bicubic) {
                SkIRect iClampRect;

                if (SkCanvas::kBleed_DrawBitmapRectFlag & flags) {
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
                bool needsTextureDomain = needs_texture_domain(bitmap,
                                                               srcRect,
                                                               paramsTemp,
                                                               fContext->getMatrix(),
                                                               bicubic);
                this->internalDrawBitmap(tmpB,
                                         tileR,
                                         paramsTemp,
                                         paint,
                                         flags,
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
                                     const SkRect& srcRect,
                                     const GrTextureParams& params,
                                     const SkPaint& paint,
                                     SkCanvas::DrawBitmapRectFlags flags,
                                     bool bicubic,
                                     bool needsTextureDomain) {
    SkASSERT(bitmap.width() <= fContext->getMaxTextureSize() &&
             bitmap.height() <= fContext->getMaxTextureSize());

    GrTexture* texture;
    SkAutoCachedTexture act(this, bitmap, &params, &texture);
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
    SkAutoTUnref<GrFragmentProcessor> fp;
    if (needsTextureDomain && !(flags & SkCanvas::kBleed_DrawBitmapRectFlag)) {
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
            fp.reset(GrBicubicEffect::Create(texture, SkMatrix::I(), textureDomain));
        } else {
            fp.reset(GrTextureDomainEffect::Create(texture,
                                                       SkMatrix::I(),
                                                       textureDomain,
                                                       GrTextureDomain::kClamp_Mode,
                                                       params.filterMode()));
        }
    } else if (bicubic) {
        SkASSERT(GrTextureParams::kNone_FilterMode == params.filterMode());
        SkShader::TileMode tileModes[2] = { params.getTileModeX(), params.getTileModeY() };
        fp.reset(GrBicubicEffect::Create(texture, SkMatrix::I(), tileModes));
    } else {
        fp.reset(GrSimpleTextureEffect::Create(texture, SkMatrix::I(), params));
    }

    // Construct a GrPaint by setting the bitmap texture as the first effect and then configuring
    // the rest from the SkPaint.
    GrPaint grPaint;
    grPaint.addColorProcessor(fp);
    bool alphaOnly = !(kAlpha_8_SkColorType == bitmap.colorType());
    GrColor paintColor = (alphaOnly) ? SkColor2GrColorJustAlpha(paint.getColor()) :
                                       SkColor2GrColor(paint.getColor());
    SkPaint2GrPaintNoShader(this->context(), paint, paintColor, false, &grPaint);

    fContext->drawRectToRect(grPaint, dstRect, paintRect);
}

static bool filter_texture(SkBaseDevice* device, GrContext* context,
                           GrTexture* texture, const SkImageFilter* filter,
                           int w, int h, const SkImageFilter::Context& ctx,
                           SkBitmap* result, SkIPoint* offset) {
    SkASSERT(filter);
    SkDeviceImageFilterProxy proxy(device);

    if (filter->canFilterImageGPU()) {
        // Save the render target and set it to NULL, so we don't accidentally draw to it in the
        // filter.  Also set the clip wide open and the matrix to identity.
        GrContext::AutoWideOpenIdentityDraw awo(context, NULL);
        return filter->filterImageGPU(&proxy, wrap_texture(texture), ctx, result, offset);
    } else {
        return false;
    }
}

void SkGpuDevice::drawSprite(const SkDraw& draw, const SkBitmap& bitmap,
                             int left, int top, const SkPaint& paint) {
    // drawSprite is defined to be in device coords.
    CHECK_SHOULD_DRAW(draw, true);

    SkAutoLockPixels alp(bitmap, !bitmap.getTexture());
    if (!bitmap.getTexture() && !bitmap.readyToDraw()) {
        return;
    }

    int w = bitmap.width();
    int h = bitmap.height();

    GrTexture* texture;
    // draw sprite uses the default texture params
    SkAutoCachedTexture act(this, bitmap, NULL, &texture);

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
        if (filter_texture(this, fContext, texture, filter, w, h, ctx, &filteredBitmap,
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

    SkPaint2GrPaintNoShader(this->context(), paint, SkColor2GrColorJustAlpha(paint.getColor()),
                            false, &grPaint);

    fContext->drawRectToRect(grPaint,
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
                                 const SkPaint& paint,
                                 SkCanvas::DrawBitmapRectFlags flags) {
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

    this->drawBitmapCommon(*draw, bitmap, &tmpSrc, &dstSize, paint, flags);
}

void SkGpuDevice::drawDevice(const SkDraw& draw, SkBaseDevice* device,
                             int x, int y, const SkPaint& paint) {
    // clear of the source device must occur before CHECK_SHOULD_DRAW
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::drawDevice", fContext);
    SkGpuDevice* dev = static_cast<SkGpuDevice*>(device);
    if (dev->fNeedClear) {
        // TODO: could check here whether we really need to draw at all
        dev->clear(0x0);
    }

    // drawDevice is defined to be in device coords.
    CHECK_SHOULD_DRAW(draw, true);

    GrRenderTarget* devRT = dev->accessRenderTarget();
    GrTexture* devTex;
    if (NULL == (devTex = devRT->asTexture())) {
        return;
    }

    const SkBitmap& bm = dev->accessBitmap(false);
    int w = bm.width();
    int h = bm.height();

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
        if (filter_texture(this, fContext, devTex, filter, w, h, ctx, &filteredBitmap,
                           &offset)) {
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

    SkPaint2GrPaintNoShader(this->context(), paint, SkColor2GrColorJustAlpha(paint.getColor()),
                            false, &grPaint);

    SkRect dstRect = SkRect::MakeXYWH(SkIntToScalar(x),
                                      SkIntToScalar(y),
                                      SkIntToScalar(w),
                                      SkIntToScalar(h));

    // The device being drawn may not fill up its texture (e.g. saveLayer uses approximate
    // scratch texture).
    SkRect srcRect = SkRect::MakeWH(SK_Scalar1 * w / devTex->width(),
                                    SK_Scalar1 * h / devTex->height());

    fContext->drawRectToRect(grPaint, dstRect, srcRect);
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
    SkAutoCachedTexture act(this, src, NULL, &texture);

    return filter_texture(this, fContext, texture, filter, src.width(), src.height(), ctx,
                          result, offset);
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
    CHECK_SHOULD_DRAW(draw, false);

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
        SkPaint2GrPaintNoShader(this->context(), copy, SkColor2GrColor(copy.getColor()),
                                NULL == colors, &grPaint);

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
            SkPaint2GrPaintNoShader(this->context(), paint, SkColor2GrColor(paint.getColor()),
                                    NULL == colors, &grPaint);
        } else {
            SkPaint2GrPaintShader(this->context(), paint, NULL == colors, &grPaint);
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
    fContext->drawVertices(grPaint,
                           primType,
                           vertexCount,
                           vertices,
                           texs,
                           colors,
                           outIndices,
                           indexCount);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawText(const SkDraw& draw, const void* text,
                          size_t byteLength, SkScalar x, SkScalar y,
                          const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw, false);
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::drawText", fContext);

    if (fMainTextContext->canDraw(paint)) {
        GrPaint grPaint;
        SkPaint2GrPaintShader(this->context(), paint, true, &grPaint);

        SkDEBUGCODE(this->validate();)

        fMainTextContext->drawText(grPaint, paint, (const char *)text, byteLength, x, y);
    } else if (fFallbackTextContext && fFallbackTextContext->canDraw(paint)) {
        GrPaint grPaint;
        SkPaint2GrPaintShader(this->context(), paint, true, &grPaint);

        SkDEBUGCODE(this->validate();)

        fFallbackTextContext->drawText(grPaint, paint, (const char *)text, byteLength, x, y);
    } else {
        // this guy will just call our drawPath()
        draw.drawText_asPaths((const char*)text, byteLength, x, y, paint);
    }
}

void SkGpuDevice::drawPosText(const SkDraw& draw, const void* text,
                             size_t byteLength, const SkScalar pos[],
                             SkScalar constY, int scalarsPerPos,
                             const SkPaint& paint) {
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice::drawPosText", fContext);
    CHECK_SHOULD_DRAW(draw, false);

    if (fMainTextContext->canDraw(paint)) {
        GrPaint grPaint;
        SkPaint2GrPaintShader(this->context(), paint, true, &grPaint);

        SkDEBUGCODE(this->validate();)

        fMainTextContext->drawPosText(grPaint, paint, (const char *)text, byteLength, pos,
                                      constY, scalarsPerPos);
    } else if (fFallbackTextContext && fFallbackTextContext->canDraw(paint)) {
        GrPaint grPaint;
        SkPaint2GrPaintShader(this->context(), paint, true, &grPaint);

        SkDEBUGCODE(this->validate();)

        fFallbackTextContext->drawPosText(grPaint, paint, (const char *)text, byteLength, pos,
                                          constY, scalarsPerPos);
    } else {
        draw.drawPosText_asPaths((const char*)text, byteLength, pos, constY,
                                 scalarsPerPos, paint);
    }
}

void SkGpuDevice::drawTextOnPath(const SkDraw& draw, const void* text,
                                size_t len, const SkPath& path,
                                const SkMatrix* m, const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw, false);

    SkASSERT(draw.fDevice == this);
    draw.drawTextOnPath((const char*)text, len, path, m, paint);
}

///////////////////////////////////////////////////////////////////////////////

bool SkGpuDevice::filterTextFlags(const SkPaint& paint, TextFlags* flags) {
    if (!paint.isLCDRenderText()) {
        // we're cool with the paint as is
        return false;
    }

    if (paint.getShader() ||
        paint.getXfermode() || // unless its srcover
        paint.getMaskFilter() ||
        paint.getRasterizer() ||
        paint.getColorFilter() ||
        paint.getPathEffect() ||
        paint.isFakeBoldText() ||
        paint.getStyle() != SkPaint::kFill_Style) {
        // turn off lcd, but turn on kGenA8
        flags->fFlags = paint.getFlags() & ~SkPaint::kLCDRenderText_Flag;
        flags->fFlags |= SkPaint::kGenA8FromLCD_Flag;
        return true;
    }
    // we're cool with the paint as is
    return false;
}

void SkGpuDevice::flush() {
    DO_DEFERRED_CLEAR();
    fContext->resolveRenderTarget(fRenderTarget);
}

///////////////////////////////////////////////////////////////////////////////

SkBaseDevice* SkGpuDevice::onCreateDevice(const SkImageInfo& info, Usage usage) {
    GrTextureDesc desc;
    desc.fConfig = fRenderTarget->config();
    desc.fFlags = kRenderTarget_GrTextureFlagBit;
    desc.fWidth = info.width();
    desc.fHeight = info.height();
    desc.fSampleCnt = fRenderTarget->numSamples();

    SkAutoTUnref<GrTexture> texture;
    // Skia's convention is to only clear a device if it is non-opaque.
    unsigned flags = info.isOpaque() ? 0 : kNeedClear_Flag;

#if CACHE_COMPATIBLE_DEVICE_TEXTURES
    // layers are never draw in repeat modes, so we can request an approx
    // match and ignore any padding.
    flags |= kCached_Flag;
    const GrContext::ScratchTexMatch match = (kSaveLayer_Usage == usage) ?
                                                GrContext::kApprox_ScratchTexMatch :
                                                GrContext::kExact_ScratchTexMatch;
    texture.reset(fContext->lockAndRefScratchTexture(desc, match));
#else
    texture.reset(fContext->createUncachedTexture(desc, NULL, 0));
#endif
    if (texture.get()) {
        return SkGpuDevice::Create(texture, SkSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType), flags);
    } else {
        GrPrintf("---- failed to create compatible device texture [%d %d]\n",
                 info.width(), info.height());
        return NULL;
    }
}

SkSurface* SkGpuDevice::newSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    return SkSurface::NewRenderTarget(fContext, info, fRenderTarget->numSamples(), &props);
}

void SkGpuDevice::EXPERIMENTAL_optimize(const SkPicture* picture) {
    fContext->getLayerCache()->processDeletedPictures();

    if (picture->fData.get() && !picture->fData->suitableForLayerOptimization()) {
        return;
    }

    SkPicture::AccelData::Key key = GrAccelData::ComputeAccelDataKey();

    const SkPicture::AccelData* existing = picture->EXPERIMENTAL_getAccelData(key);
    if (existing) {
        return;
    }

    GPUOptimize(picture);

    fContext->getLayerCache()->trackPicture(picture);
}

bool SkGpuDevice::EXPERIMENTAL_drawPicture(SkCanvas* mainCanvas, const SkPicture* mainPicture,
                                           const SkMatrix* matrix, const SkPaint* paint) {
    // todo: should handle these natively
    if (matrix || paint) {
        return false;
    }

    fContext->getLayerCache()->processDeletedPictures();

    SkRect clipBounds;
    if (!mainCanvas->getClipBounds(&clipBounds)) {
        return true;
    }

    SkTDArray<GrLayerHoister::HoistedLayer> atlased, nonAtlased;

    if (!GrLayerHoister::FindLayersToHoist(mainPicture, clipBounds, &atlased, &nonAtlased,
                                           fContext->getLayerCache())) {
        return false;
    }

    GrReplacements replacements;

    GrLayerHoister::DrawLayers(atlased, nonAtlased, &replacements);

    // Render the entire picture using new layers
    GrRecordReplaceDraw(*mainPicture->fRecord, mainCanvas, mainPicture->fBBH.get(), 
                        &replacements, NULL);

    GrLayerHoister::UnlockLayers(fContext->getLayerCache(), atlased, nonAtlased);

    return true;
}

SkImageFilter::Cache* SkGpuDevice::getImageFilterCache() {
    // We always return a transient cache, so it is freed after each
    // filter traversal.
    return SkImageFilter::Cache::Create(kDefaultImageFilterCacheSize);
}
