/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGpuDevice.h"

#include "effects/GrTextureDomainEffect.h"
#include "effects/GrSimpleTextureEffect.h"

#include "GrContext.h"
#include "GrTextContext.h"

#include "SkGrTexturePixelRef.h"

#include "SkColorFilter.h"
#include "SkDeviceImageFilterProxy.h"
#include "SkDrawProcs.h"
#include "SkGlyphCache.h"
#include "SkImageFilter.h"
#include "SkPathEffect.h"
#include "SkStroke.h"
#include "SkUtils.h"

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

// we use the same effect slot on GrPaint for bitmaps and shaders (since drawBitmap, drawSprite,
// and drawDevice ignore SkShader)
enum {
    kShaderEffectIdx = 0,
    kBitmapEffectIdx = 0,
    kColorFilterEffectIdx = 1,
    kXfermodeEffectIdx = 2,
};

#define MAX_BLUR_SIGMA 4.0f
// FIXME:  This value comes from from SkBlurMaskFilter.cpp.
// Should probably be put in a common header someplace.
#define MAX_BLUR_RADIUS SkIntToScalar(128)
// This constant approximates the scaling done in the software path's
// "high quality" mode, in SkBlurMask::Blur() (1 / sqrt(3)).
// IMHO, it actually should be 1:  we blur "less" than we should do
// according to the CSS and canvas specs, simply because Safari does the same.
// Firefox used to do the same too, until 4.0 where they fixed it.  So at some
// point we should probably get rid of these scaling constants and rebaseline
// all the blur tests.
#define BLUR_SIGMA_SCALE 0.6f
// This constant represents the screen alignment criterion in texels for
// requiring texture domain clamping to prevent color bleeding when drawing
// a sub region of a larger source image.
#define COLOR_BLEED_TOLERANCE SkFloatToScalar(0.001f)

#define DO_DEFERRED_CLEAR()             \
    do {                                \
        if (fNeedClear) {               \
            this->clear(SK_ColorTRANSPARENT); \
        }                               \
    } while (false)                     \

///////////////////////////////////////////////////////////////////////////////

#define CHECK_FOR_NODRAW_ANNOTATION(paint) \
    do { if (paint.isNoDrawAnnotation()) { return; } } while (0)

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
        GrAssert(NULL != texture);
        *texture = this->set(device, bitmap, params);
    }

    ~SkAutoCachedTexture() {
        if (NULL != fTexture) {
            GrUnlockAndUnrefCachedBitmapTexture(fTexture);
        }
    }

    GrTexture* set(SkGpuDevice* device,
                   const SkBitmap& bitmap,
                   const GrTextureParams* params) {
        if (NULL != fTexture) {
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

static SkBitmap::Config grConfig2skConfig(GrPixelConfig config, bool* isOpaque) {
    switch (config) {
        case kAlpha_8_GrPixelConfig:
            *isOpaque = false;
            return SkBitmap::kA8_Config;
        case kRGB_565_GrPixelConfig:
            *isOpaque = true;
            return SkBitmap::kRGB_565_Config;
        case kRGBA_4444_GrPixelConfig:
            *isOpaque = false;
            return SkBitmap::kARGB_4444_Config;
        case kSkia8888_GrPixelConfig:
            // we don't currently have a way of knowing whether
            // a 8888 is opaque based on the config.
            *isOpaque = false;
            return SkBitmap::kARGB_8888_Config;
        default:
            *isOpaque = false;
            return SkBitmap::kNo_Config;
    }
}

static SkBitmap make_bitmap(GrContext* context, GrRenderTarget* renderTarget) {
    GrPixelConfig config = renderTarget->config();

    bool isOpaque;
    SkBitmap bitmap;
    bitmap.setConfig(grConfig2skConfig(config, &isOpaque),
                     renderTarget->width(), renderTarget->height());
    bitmap.setIsOpaque(isOpaque);
    return bitmap;
}

SkGpuDevice* SkGpuDevice::Create(GrSurface* surface) {
    GrAssert(NULL != surface);
    if (NULL == surface->asRenderTarget() || NULL == surface->getContext()) {
        return NULL;
    }
    if (surface->asTexture()) {
        return SkNEW_ARGS(SkGpuDevice, (surface->getContext(), surface->asTexture()));
    } else {
        return SkNEW_ARGS(SkGpuDevice, (surface->getContext(), surface->asRenderTarget()));
    }
}

SkGpuDevice::SkGpuDevice(GrContext* context, GrTexture* texture)
: SkDevice(make_bitmap(context, texture->asRenderTarget())) {
    this->initFromRenderTarget(context, texture->asRenderTarget(), false);
}

SkGpuDevice::SkGpuDevice(GrContext* context, GrRenderTarget* renderTarget)
: SkDevice(make_bitmap(context, renderTarget)) {
    this->initFromRenderTarget(context, renderTarget, false);
}

void SkGpuDevice::initFromRenderTarget(GrContext* context,
                                       GrRenderTarget* renderTarget,
                                       bool cached) {
    fDrawProcs = NULL;

    fContext = context;
    fContext->ref();

    fRenderTarget = NULL;
    fNeedClear = false;

    GrAssert(NULL != renderTarget);
    fRenderTarget = renderTarget;
    fRenderTarget->ref();

    // Hold onto to the texture in the pixel ref (if there is one) because the texture holds a ref
    // on the RT but not vice-versa.
    // TODO: Remove this trickery once we figure out how to make SkGrPixelRef do this without
    // busting chrome (for a currently unknown reason).
    GrSurface* surface = fRenderTarget->asTexture();
    if (NULL == surface) {
        surface = fRenderTarget;
    }
    SkPixelRef* pr = SkNEW_ARGS(SkGrPixelRef, (surface, cached));

    this->setPixelRef(pr, 0)->unref();
}

SkGpuDevice::SkGpuDevice(GrContext* context,
                         SkBitmap::Config config,
                         int width,
                         int height,
                         int sampleCount)
    : SkDevice(config, width, height, false /*isOpaque*/) {

    fDrawProcs = NULL;

    fContext = context;
    fContext->ref();

    fRenderTarget = NULL;
    fNeedClear = false;

    if (config != SkBitmap::kRGB_565_Config) {
        config = SkBitmap::kARGB_8888_Config;
    }

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = SkBitmapConfig2GrPixelConfig(config);
    desc.fSampleCnt = sampleCount;

    SkAutoTUnref<GrTexture> texture(fContext->createUncachedTexture(desc, NULL, 0));

    if (NULL != texture) {
        fRenderTarget = texture->asRenderTarget();
        fRenderTarget->ref();

        GrAssert(NULL != fRenderTarget);

        // wrap the bitmap with a pixelref to expose our texture
        SkGrPixelRef* pr = SkNEW_ARGS(SkGrPixelRef, (texture));
        this->setPixelRef(pr, 0)->unref();
    } else {
        GrPrintf("--- failed to create gpu-offscreen [%d %d]\n",
                 width, height);
        GrAssert(false);
    }
}

SkGpuDevice::~SkGpuDevice() {
    if (fDrawProcs) {
        delete fDrawProcs;
    }

    // The GrContext takes a ref on the target. We don't want to cause the render
    // target to be unnecessarily kept alive.
    if (fContext->getRenderTarget() == fRenderTarget) {
        fContext->setRenderTarget(NULL);
    }

    if (fContext->getClip() == &fClipData) {
        fContext->setClip(NULL);
    }

    SkSafeUnref(fRenderTarget);
    fContext->unref();
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::makeRenderTargetCurrent() {
    DO_DEFERRED_CLEAR();
    fContext->setRenderTarget(fRenderTarget);
}

///////////////////////////////////////////////////////////////////////////////

namespace {
GrPixelConfig config8888_to_grconfig_and_flags(SkCanvas::Config8888 config8888, uint32_t* flags) {
    switch (config8888) {
        case SkCanvas::kNative_Premul_Config8888:
            *flags = 0;
            return kSkia8888_GrPixelConfig;
        case SkCanvas::kNative_Unpremul_Config8888:
            *flags = GrContext::kUnpremul_PixelOpsFlag;
            return kSkia8888_GrPixelConfig;
        case SkCanvas::kBGRA_Premul_Config8888:
            *flags = 0;
            return kBGRA_8888_GrPixelConfig;
        case SkCanvas::kBGRA_Unpremul_Config8888:
            *flags = GrContext::kUnpremul_PixelOpsFlag;
            return kBGRA_8888_GrPixelConfig;
        case SkCanvas::kRGBA_Premul_Config8888:
            *flags = 0;
            return kRGBA_8888_GrPixelConfig;
        case SkCanvas::kRGBA_Unpremul_Config8888:
            *flags = GrContext::kUnpremul_PixelOpsFlag;
            return kRGBA_8888_GrPixelConfig;
        default:
            GrCrash("Unexpected Config8888.");
            *flags = 0; // suppress warning
            return kSkia8888_GrPixelConfig;
    }
}
}

bool SkGpuDevice::onReadPixels(const SkBitmap& bitmap,
                               int x, int y,
                               SkCanvas::Config8888 config8888) {
    DO_DEFERRED_CLEAR();
    SkASSERT(SkBitmap::kARGB_8888_Config == bitmap.config());
    SkASSERT(!bitmap.isNull());
    SkASSERT(SkIRect::MakeWH(this->width(), this->height()).contains(SkIRect::MakeXYWH(x, y, bitmap.width(), bitmap.height())));

    SkAutoLockPixels alp(bitmap);
    GrPixelConfig config;
    uint32_t flags;
    config = config8888_to_grconfig_and_flags(config8888, &flags);
    return fContext->readRenderTargetPixels(fRenderTarget,
                                            x, y,
                                            bitmap.width(),
                                            bitmap.height(),
                                            config,
                                            bitmap.getPixels(),
                                            bitmap.rowBytes(),
                                            flags);
}

void SkGpuDevice::writePixels(const SkBitmap& bitmap, int x, int y,
                              SkCanvas::Config8888 config8888) {
    SkAutoLockPixels alp(bitmap);
    if (!bitmap.readyToDraw()) {
        return;
    }

    GrPixelConfig config;
    uint32_t flags;
    if (SkBitmap::kARGB_8888_Config == bitmap.config()) {
        config = config8888_to_grconfig_and_flags(config8888, &flags);
    } else {
        flags = 0;
        config= SkBitmapConfig2GrPixelConfig(bitmap.config());
    }

    fRenderTarget->writePixels(x, y, bitmap.width(), bitmap.height(),
                               config, bitmap.getPixels(), bitmap.rowBytes(), flags);
}

namespace {
void purgeClipCB(int genID, void* ) {

    if (SkClipStack::kInvalidGenID == genID ||
        SkClipStack::kEmptyGenID == genID ||
        SkClipStack::kWideOpenGenID == genID) {
        // none of these cases will have a cached clip mask
        return;
    }

}
};

void SkGpuDevice::onAttachToCanvas(SkCanvas* canvas) {
    INHERITED::onAttachToCanvas(canvas);

    // Canvas promises that this ptr is valid until onDetachFromCanvas is called
    fClipData.fClipStack = canvas->getClipStack();

    fClipData.fClipStack->addPurgeClipCallback(purgeClipCB, fContext);
}

void SkGpuDevice::onDetachFromCanvas() {
    INHERITED::onDetachFromCanvas();

    // TODO: iterate through the clip stack and clean up any cached clip masks
    fClipData.fClipStack->removePurgeClipCallback(purgeClipCB, fContext);

    fClipData.fClipStack = NULL;
}

#ifdef SK_DEBUG
static void check_bounds(const GrClipData& clipData,
                         const SkRegion& clipRegion,
                         int renderTargetWidth,
                         int renderTargetHeight) {

    SkIRect devBound;

    devBound.setLTRB(0, 0, renderTargetWidth, renderTargetHeight);

    SkClipStack::BoundsType boundType;
    SkRect canvTemp;

    clipData.fClipStack->getBounds(&canvTemp, &boundType);
    if (SkClipStack::kNormal_BoundsType == boundType) {
        SkIRect devTemp;

        canvTemp.roundOut(&devTemp);

        devTemp.offset(-clipData.fOrigin.fX, -clipData.fOrigin.fY);

        if (!devBound.intersect(devTemp)) {
            devBound.setEmpty();
        }
    }

    GrAssert(devBound.contains(clipRegion.getBounds()));
}
#endif

///////////////////////////////////////////////////////////////////////////////

// call this every draw call, to ensure that the context reflects our state,
// and not the state from some other canvas/device
void SkGpuDevice::prepareDraw(const SkDraw& draw, bool forceIdentity) {
    GrAssert(NULL != fClipData.fClipStack);

    fContext->setRenderTarget(fRenderTarget);

    SkASSERT(draw.fClipStack && draw.fClipStack == fClipData.fClipStack);

    if (forceIdentity) {
        fContext->setIdentityMatrix();
    } else {
        fContext->setMatrix(*draw.fMatrix);
    }
    fClipData.fOrigin = this->getOrigin();

#ifdef SK_DEBUG
    check_bounds(fClipData, *draw.fClip, fRenderTarget->width(), fRenderTarget->height());
#endif

    fContext->setClip(&fClipData);

    DO_DEFERRED_CLEAR();
}

SkGpuRenderTarget* SkGpuDevice::accessRenderTarget() {
    DO_DEFERRED_CLEAR();
    return (SkGpuRenderTarget*)fRenderTarget;
}

bool SkGpuDevice::bindDeviceAsTexture(GrPaint* paint) {
    GrTexture* texture = fRenderTarget->asTexture();
    if (NULL != texture) {
        paint->colorStage(kBitmapEffectIdx)->setEffect(
            GrSimpleTextureEffect::Create(texture, SkMatrix::I()))->unref();
        return true;
    }
    return false;
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

namespace {

// converts a SkPaint to a GrPaint, ignoring the skPaint's shader
// justAlpha indicates that skPaint's alpha should be used rather than the color
// Callers may subsequently modify the GrPaint. Setting constantColor indicates
// that the final paint will draw the same color at every pixel. This allows
// an optimization where the the color filter can be applied to the skPaint's
// color once while converting to GrPaint and then ignored.
inline bool skPaint2GrPaintNoShader(SkGpuDevice* dev,
                                    const SkPaint& skPaint,
                                    bool justAlpha,
                                    bool constantColor,
                                    GrPaint* grPaint) {

    grPaint->setDither(skPaint.isDither());
    grPaint->setAntiAlias(skPaint.isAntiAlias());

    SkXfermode::Coeff sm;
    SkXfermode::Coeff dm;

    SkXfermode* mode = skPaint.getXfermode();
    GrEffectRef* xferEffect = NULL;
    if (SkXfermode::AsNewEffectOrCoeff(mode, dev->context(), &xferEffect, &sm, &dm)) {
        if (NULL != xferEffect) {
            grPaint->colorStage(kXfermodeEffectIdx)->setEffect(xferEffect)->unref();
            // This may not be the right place to have this logic but we set the GPU blend to
            // src-over so that fractional coverage will be accounted for correctly.
            sm = SkXfermode::kOne_Coeff;
            dm = SkXfermode::kISA_Coeff;
        }
    } else {
        //SkDEBUGCODE(SkDebugf("Unsupported xfer mode.\n");)
#if 0
        return false;
#else
        // Fall back to src-over
        sm = SkXfermode::kOne_Coeff;
        dm = SkXfermode::kISA_Coeff;
#endif
    }
    grPaint->setBlendFunc(sk_blend_to_grblend(sm), sk_blend_to_grblend(dm));

    if (justAlpha) {
        uint8_t alpha = skPaint.getAlpha();
        grPaint->setColor(GrColorPackRGBA(alpha, alpha, alpha, alpha));
        // justAlpha is currently set to true only if there is a texture,
        // so constantColor should not also be true.
        GrAssert(!constantColor);
    } else {
        grPaint->setColor(SkColor2GrColor(skPaint.getColor()));
        GrAssert(!grPaint->isColorStageEnabled(kShaderEffectIdx));
    }

    SkColorFilter* colorFilter = skPaint.getColorFilter();
    if (NULL != colorFilter) {
        // if the source color is a constant then apply the filter here once rather than per pixel
        // in a shader.
        if (constantColor) {
            SkColor filtered = colorFilter->filterColor(skPaint.getColor());
            grPaint->setColor(SkColor2GrColor(filtered));
        } else {
            SkAutoTUnref<GrEffectRef> effect(colorFilter->asNewEffect(dev->context()));
            if (NULL != effect.get()) {
                grPaint->colorStage(kColorFilterEffectIdx)->setEffect(effect);
            } else {
                // TODO: rewrite this using asNewEffect()
                SkColor color;
                SkXfermode::Mode filterMode;
                if (colorFilter->asColorMode(&color, &filterMode)) {
                    grPaint->setXfermodeColorFilter(filterMode, SkColor2GrColor(color));
                }
            }
        }
    }

    return true;
}

// This function is similar to skPaint2GrPaintNoShader but also converts
// skPaint's shader to a GrTexture/GrEffectStage if possible. The texture to
// be used is set on grPaint and returned in param act. constantColor has the
// same meaning as in skPaint2GrPaintNoShader.
inline bool skPaint2GrPaintShader(SkGpuDevice* dev,
                                  const SkPaint& skPaint,
                                  bool constantColor,
                                  GrPaint* grPaint) {
    SkShader* shader = skPaint.getShader();
    if (NULL == shader) {
        return skPaint2GrPaintNoShader(dev, skPaint, false, constantColor, grPaint);
    } else if (!skPaint2GrPaintNoShader(dev, skPaint, true, false, grPaint)) {
        return false;
    }

    SkAutoTUnref<GrEffectRef> effect(shader->asNewEffect(dev->context(), skPaint));
    if (NULL != effect.get()) {
        grPaint->colorStage(kShaderEffectIdx)->setEffect(effect);
        return true;
    }

    // We still don't have SkColorShader::asNewEffect() implemented.
    SkShader::GradientInfo info;
    SkColor                color;

    info.fColors = &color;
    info.fColorOffsets = NULL;
    info.fColorCount = 1;
    if (SkShader::kColor_GradientType == shader->asAGradient(&info)) {
        SkPaint copy(skPaint);
        copy.setShader(NULL);
        // modulate the paint alpha by the shader's solid color alpha
        U8CPU newA = SkMulDiv255Round(SkColorGetA(color), copy.getAlpha());
        copy.setColor(SkColorSetA(color, newA));
        return skPaint2GrPaintNoShader(dev, copy, false, constantColor, grPaint);
    }
    return false;
}
}

///////////////////////////////////////////////////////////////////////////////
void SkGpuDevice::clear(SkColor color) {
    SkIRect rect = SkIRect::MakeWH(this->width(), this->height());
    fContext->clear(&rect, SkColor2GrColor(color), fRenderTarget);
    fNeedClear = false;
}

void SkGpuDevice::drawPaint(const SkDraw& draw, const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw, false);

    GrPaint grPaint;
    if (!skPaint2GrPaintShader(this, paint, true, &grPaint)) {
        return;
    }

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
    CHECK_FOR_NODRAW_ANNOTATION(paint);
    CHECK_SHOULD_DRAW(draw, false);

    SkScalar width = paint.getStrokeWidth();
    if (width < 0) {
        return;
    }

    // we only handle hairlines and paints without path effects or mask filters,
    // else we let the SkDraw call our drawPath()
    if (width > 0 || paint.getPathEffect() || paint.getMaskFilter()) {
        draw.drawPoints(mode, count, pts, paint, true);
        return;
    }

    GrPaint grPaint;
    if (!skPaint2GrPaintShader(this, paint, true, &grPaint)) {
        return;
    }

    fContext->drawVertices(grPaint,
                           gPointMode2PrimtiveType[mode],
                           count,
                           (GrPoint*)pts,
                           NULL,
                           NULL,
                           NULL,
                           0);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawRect(const SkDraw& draw, const SkRect& rect,
                           const SkPaint& paint) {
    CHECK_FOR_NODRAW_ANNOTATION(paint);
    CHECK_SHOULD_DRAW(draw, false);

    bool doStroke = paint.getStyle() != SkPaint::kFill_Style;
    SkScalar width = paint.getStrokeWidth();

    /*
        We have special code for hairline strokes, miter-strokes, and fills.
        Anything else we just call our path code.
     */
    bool usePath = doStroke && width > 0 &&
                    paint.getStrokeJoin() != SkPaint::kMiter_Join;
    // another two reasons we might need to call drawPath...
    if (paint.getMaskFilter() || paint.getPathEffect()) {
        usePath = true;
    }
    // until we aa rotated rects...
    if (!usePath && paint.isAntiAlias() && !fContext->getMatrix().rectStaysRect()) {
        usePath = true;
    }
    // small miter limit means right angles show bevel...
    if (SkPaint::kMiter_Join == paint.getStrokeJoin() &&
        paint.getStrokeMiter() < SK_ScalarSqrt2)
    {
        usePath = true;
    }
    // until we can both stroke and fill rectangles
    if (paint.getStyle() == SkPaint::kStrokeAndFill_Style) {
        usePath = true;
    }

    if (usePath) {
        SkPath path;
        path.addRect(rect);
        this->drawPath(draw, path, paint, NULL, true);
        return;
    }

    GrPaint grPaint;
    if (!skPaint2GrPaintShader(this, paint, true, &grPaint)) {
        return;
    }
    fContext->drawRect(grPaint, rect, doStroke ? width : -1);
}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawOval(const SkDraw& draw, const SkRect& oval,
                           const SkPaint& paint) {
    CHECK_FOR_NODRAW_ANNOTATION(paint);
    CHECK_SHOULD_DRAW(draw, false);

    bool usePath = false;
    // some basic reasons we might need to call drawPath...
    if (paint.getMaskFilter() || paint.getPathEffect()) {
        usePath = true;
    }

    if (usePath) {
        SkPath path;
        path.addOval(oval);
        this->drawPath(draw, path, paint, NULL, true);
        return;
    }

    GrPaint grPaint;
    if (!skPaint2GrPaintShader(this, paint, true, &grPaint)) {
        return;
    }
    SkStrokeRec stroke(paint);

    fContext->drawOval(grPaint, oval, stroke);
}

#include "SkMaskFilter.h"
#include "SkBounder.h"

///////////////////////////////////////////////////////////////////////////////

// helpers for applying mask filters
namespace {

// We prefer to blur small rect with small radius via CPU.
#define MIN_GPU_BLUR_SIZE SkIntToScalar(64)
#define MIN_GPU_BLUR_RADIUS SkIntToScalar(32)
inline bool shouldDrawBlurWithCPU(const SkRect& rect, SkScalar radius) {
    if (rect.width() <= MIN_GPU_BLUR_SIZE &&
        rect.height() <= MIN_GPU_BLUR_SIZE &&
        radius <= MIN_GPU_BLUR_RADIUS) {
        return true;
    }
    return false;
}

bool drawWithGPUMaskFilter(GrContext* context, const SkPath& devPath, const SkStrokeRec& stroke,
                           SkMaskFilter* filter, const SkRegion& clip,
                           SkBounder* bounder, GrPaint* grp) {
    SkMaskFilter::BlurInfo info;
    SkMaskFilter::BlurType blurType = filter->asABlur(&info);
    if (SkMaskFilter::kNone_BlurType == blurType) {
        return false;
    }
    SkScalar radius = info.fIgnoreTransform ? info.fRadius
                                            : context->getMatrix().mapRadius(info.fRadius);
    radius = SkMinScalar(radius, MAX_BLUR_RADIUS);
    if (radius <= 0) {
        return false;
    }

    SkRect srcRect = devPath.getBounds();
    if (shouldDrawBlurWithCPU(srcRect, radius)) {
        return false;
    }

    float sigma = SkScalarToFloat(radius) * BLUR_SIGMA_SCALE;
    float sigma3 = sigma * 3.0f;

    SkRect clipRect;
    clipRect.set(clip.getBounds());

    // Outset srcRect and clipRect by 3 * sigma, to compute affected blur area.
    srcRect.inset(SkFloatToScalar(-sigma3), SkFloatToScalar(-sigma3));
    clipRect.inset(SkFloatToScalar(-sigma3), SkFloatToScalar(-sigma3));
    srcRect.intersect(clipRect);
    SkRect finalRect = srcRect;
    SkIRect finalIRect;
    finalRect.roundOut(&finalIRect);
    if (clip.quickReject(finalIRect)) {
        return true;
    }
    if (bounder && !bounder->doIRect(finalIRect)) {
        return true;
    }
    GrPoint offset = GrPoint::Make(-srcRect.fLeft, -srcRect.fTop);
    srcRect.offset(offset);
    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit;
    desc.fWidth = SkScalarCeilToInt(srcRect.width());
    desc.fHeight = SkScalarCeilToInt(srcRect.height());
    // We actually only need A8, but it often isn't supported as a
    // render target so default to RGBA_8888
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    if (context->isConfigRenderable(kAlpha_8_GrPixelConfig)) {
        desc.fConfig = kAlpha_8_GrPixelConfig;
    }

    GrAutoScratchTexture pathEntry(context, desc);
    GrTexture* pathTexture = pathEntry.texture();
    if (NULL == pathTexture) {
        return false;
    }

    SkAutoTUnref<GrTexture> blurTexture;

    {
        GrContext::AutoRenderTarget art(context, pathTexture->asRenderTarget());
        GrContext::AutoClip ac(context, srcRect);

        context->clear(NULL, 0);

        GrPaint tempPaint;
        if (grp->isAntiAlias()) {
            tempPaint.setAntiAlias(true);
            // AA uses the "coverage" stages on GrDrawTarget. Coverage with a dst
            // blend coeff of zero requires dual source blending support in order
            // to properly blend partially covered pixels. This means the AA
            // code path may not be taken. So we use a dst blend coeff of ISA. We
            // could special case AA draws to a dst surface with known alpha=0 to
            // use a zero dst coeff when dual source blending isn't available.f
            tempPaint.setBlendFunc(kOne_GrBlendCoeff, kISC_GrBlendCoeff);
        }

        GrContext::AutoMatrix am;

        // Draw hard shadow to pathTexture with path top-left at origin using tempPaint.
        SkMatrix translate;
        translate.setTranslate(offset.fX, offset.fY);
        am.set(context, translate);
        context->drawPath(tempPaint, devPath, stroke);

        // If we're doing a normal blur, we can clobber the pathTexture in the
        // gaussianBlur.  Otherwise, we need to save it for later compositing.
        bool isNormalBlur = blurType == SkMaskFilter::kNormal_BlurType;
        blurTexture.reset(context->gaussianBlur(pathTexture, isNormalBlur,
                                                srcRect, sigma, sigma));
        if (NULL == blurTexture) {
            return false;
        }

        if (!isNormalBlur) {
            context->setIdentityMatrix();
            GrPaint paint;
            SkMatrix matrix;
            matrix.setIDiv(pathTexture->width(), pathTexture->height());
            // Blend pathTexture over blurTexture.
            context->setRenderTarget(blurTexture->asRenderTarget());
            paint.colorStage(0)->setEffect(
                GrSimpleTextureEffect::Create(pathTexture, matrix))->unref();
            if (SkMaskFilter::kInner_BlurType == blurType) {
                // inner:  dst = dst * src
                paint.setBlendFunc(kDC_GrBlendCoeff, kZero_GrBlendCoeff);
            } else if (SkMaskFilter::kSolid_BlurType == blurType) {
                // solid:  dst = src + dst - src * dst
                //             = (1 - dst) * src + 1 * dst
                paint.setBlendFunc(kIDC_GrBlendCoeff, kOne_GrBlendCoeff);
            } else if (SkMaskFilter::kOuter_BlurType == blurType) {
                // outer:  dst = dst * (1 - src)
                //             = 0 * src + (1 - src) * dst
                paint.setBlendFunc(kZero_GrBlendCoeff, kISC_GrBlendCoeff);
            }
            context->drawRect(paint, srcRect);
        }
    }

    GrContext::AutoMatrix am;
    if (!am.setIdentity(context, grp)) {
        return false;
    }

    static const int MASK_IDX = GrPaint::kMaxCoverageStages - 1;
    // we assume the last mask index is available for use
    GrAssert(!grp->isCoverageStageEnabled(MASK_IDX));

    SkMatrix matrix;
    matrix.setTranslate(-finalRect.fLeft, -finalRect.fTop);
    matrix.postIDiv(blurTexture->width(), blurTexture->height());

    grp->coverageStage(MASK_IDX)->reset();
    grp->coverageStage(MASK_IDX)->setEffect(
        GrSimpleTextureEffect::Create(blurTexture, matrix))->unref();
    context->drawRect(*grp, finalRect);
    return true;
}

bool drawWithMaskFilter(GrContext* context, const SkPath& devPath,
                        SkMaskFilter* filter, const SkRegion& clip, SkBounder* bounder,
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
    if (bounder && !bounder->doIRect(dstM.fBounds)) {
        return false;
    }

    // we now have a device-aligned 8bit mask in dstM, ready to be drawn using
    // the current clip (and identity matrix) and GrPaint settings
    GrContext::AutoMatrix am;
    am.setIdentity(context, grp);

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

    static const int MASK_IDX = GrPaint::kMaxCoverageStages - 1;
    // we assume the last mask index is available for use
    GrAssert(!grp->isCoverageStageEnabled(MASK_IDX));

    SkMatrix m;
    m.setTranslate(-dstM.fBounds.fLeft*SK_Scalar1, -dstM.fBounds.fTop*SK_Scalar1);
    m.postIDiv(texture->width(), texture->height());

    grp->coverageStage(MASK_IDX)->setEffect(GrSimpleTextureEffect::Create(texture, m))->unref();
    GrRect d;
    d.setLTRB(SkIntToScalar(dstM.fBounds.fLeft),
              SkIntToScalar(dstM.fBounds.fTop),
              SkIntToScalar(dstM.fBounds.fRight),
              SkIntToScalar(dstM.fBounds.fBottom));

    context->drawRect(*grp, d);
    return true;
}

}

///////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawPath(const SkDraw& draw, const SkPath& origSrcPath,
                           const SkPaint& paint, const SkMatrix* prePathMatrix,
                           bool pathIsMutable) {
    CHECK_FOR_NODRAW_ANNOTATION(paint);
    CHECK_SHOULD_DRAW(draw, false);

    GrPaint grPaint;
    if (!skPaint2GrPaintShader(this, paint, true, &grPaint)) {
        return;
    }

    // can we cheat, and treat a thin stroke as a hairline w/ coverage
    // if we can, we draw lots faster (raster device does this same test)
    SkScalar hairlineCoverage;
    bool doHairLine = SkDrawTreatAsHairline(paint, fContext->getMatrix(), &hairlineCoverage);
    if (doHairLine) {
        grPaint.setCoverage(SkScalarRoundToInt(hairlineCoverage * grPaint.getCoverage()));
    }

    // If we have a prematrix, apply it to the path, optimizing for the case
    // where the original path can in fact be modified in place (even though
    // its parameter type is const).
    SkPath* pathPtr = const_cast<SkPath*>(&origSrcPath);
    SkPath  tmpPath, effectPath;

    if (prePathMatrix) {
        SkPath* result = pathPtr;

        if (!pathIsMutable) {
            result = &tmpPath;
            pathIsMutable = true;
        }
        // should I push prePathMatrix on our MV stack temporarily, instead
        // of applying it here? See SkDraw.cpp
        pathPtr->transform(*prePathMatrix, result);
        pathPtr = result;
    }
    // at this point we're done with prePathMatrix
    SkDEBUGCODE(prePathMatrix = (const SkMatrix*)0x50FF8001;)

    SkStrokeRec stroke(paint);
    SkPathEffect* pathEffect = paint.getPathEffect();
    const SkRect* cullRect = NULL;  // TODO: what is our bounds?
    if (pathEffect && pathEffect->filterPath(&effectPath, *pathPtr, &stroke,
                                             cullRect)) {
        pathPtr = &effectPath;
    }

    if (!pathEffect && doHairLine) {
        stroke.setHairlineStyle();
    }

    if (paint.getMaskFilter()) {
        if (!stroke.isHairlineStyle()) {
            if (stroke.applyToPath(&tmpPath, *pathPtr)) {
                pathPtr = &tmpPath;
                stroke.setFillStyle();
            }
        }

        // avoid possibly allocating a new path in transform if we can
        SkPath* devPathPtr = pathIsMutable ? pathPtr : &tmpPath;

        // transform the path into device space
        pathPtr->transform(fContext->getMatrix(), devPathPtr);
        if (!drawWithGPUMaskFilter(fContext, *devPathPtr, stroke, paint.getMaskFilter(),
                                   *draw.fClip, draw.fBounder, &grPaint)) {
            SkPaint::Style style = stroke.isHairlineStyle() ? SkPaint::kStroke_Style :
                                                              SkPaint::kFill_Style;
            drawWithMaskFilter(fContext, *devPathPtr, paint.getMaskFilter(),
                               *draw.fClip, draw.fBounder, &grPaint, style);
        }
        return;
    }

    fContext->drawPath(grPaint, *pathPtr, stroke);
}

namespace {

inline int get_tile_count(int l, int t, int r, int b, int tileSize)  {
    int tilesX = (r / tileSize) - (l / tileSize) + 1;
    int tilesY = (b / tileSize) - (t / tileSize) + 1;
    return tilesX * tilesY;
}

inline int determine_tile_size(const SkBitmap& bitmap,
                               const SkRect& src,
                               int maxTextureSize) {
    static const int kSmallTileSize = 1 << 10;
    if (maxTextureSize <= kSmallTileSize) {
        return maxTextureSize;
    }

    size_t maxTexTotalTileSize;
    size_t smallTotalTileSize;

    SkIRect iSrc;
    src.roundOut(&iSrc);

    maxTexTotalTileSize = get_tile_count(iSrc.fLeft,
                                         iSrc.fTop,
                                         iSrc.fRight,
                                         iSrc.fBottom,
                                         maxTextureSize);
    smallTotalTileSize = get_tile_count(iSrc.fLeft,
                                        iSrc.fTop,
                                        iSrc.fRight,
                                        iSrc.fBottom,
                                        kSmallTileSize);

    maxTexTotalTileSize *= maxTextureSize * maxTextureSize;
    smallTotalTileSize *= kSmallTileSize * kSmallTileSize;

    if (maxTexTotalTileSize > 2 * smallTotalTileSize) {
        return kSmallTileSize;
    } else {
        return maxTextureSize;
    }
}
}

bool SkGpuDevice::shouldTileBitmap(const SkBitmap& bitmap,
                                   const GrTextureParams& params,
                                   const SkRect* srcRectPtr) const {
    // if bitmap is explictly texture backed then just use the texture
    if (NULL != bitmap.getTexture()) {
        return false;
    }
    // if it's larger than the max texture size, then we have no choice but
    // tiling
    const int maxTextureSize = fContext->getMaxTextureSize();
    if (bitmap.width() > maxTextureSize ||
        bitmap.height() > maxTextureSize) {
        return true;
    }
    // if we are going to have to draw the whole thing, then don't tile
    if (NULL == srcRectPtr) {
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
    fContext->getTextureCacheLimits(NULL, &cacheSize);
    if (bmpSize < cacheSize / 2) {
        return false;
    }

    SkScalar fracUsed = SkScalarMul(srcRectPtr->width() / bitmap.width(),
                                    srcRectPtr->height() / bitmap.height());
    if (fracUsed <= SK_ScalarHalf) {
        return true;
    } else {
        return false;
    }
}

void SkGpuDevice::drawBitmap(const SkDraw& draw,
                             const SkBitmap& bitmap,
                             const SkIRect* srcRectPtr,
                             const SkMatrix& m,
                             const SkPaint& paint) {

    SkRect  tmp;
    SkRect* tmpPtr = NULL;

    // convert from SkIRect to SkRect
    if (NULL != srcRectPtr) {
        tmp.set(*srcRectPtr);
        tmpPtr = &tmp;
    }

    // We cannot call drawBitmapRect here since 'm' could be anything
    this->drawBitmapCommon(draw, bitmap, tmpPtr, m, paint);
}

void SkGpuDevice::drawBitmapCommon(const SkDraw& draw,
                                   const SkBitmap& bitmap,
                                   const SkRect* srcRectPtr,
                                   const SkMatrix& m,
                                   const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw, false);

    SkRect srcRect;
    if (NULL == srcRectPtr) {
        srcRect.set(0, 0, SkIntToScalar(bitmap.width()), SkIntToScalar(bitmap.height()));
    } else {
        srcRect = *srcRectPtr;
    }

    if (paint.getMaskFilter()){
        // Convert the bitmap to a shader so that the rect can be drawn
        // through drawRect, which supports mask filters.
        SkMatrix        newM(m);
        SkBitmap        tmp;    // subset of bitmap, if necessary
        const SkBitmap* bitmapPtr = &bitmap;
        if (NULL != srcRectPtr) {
            SkIRect iSrc;
            srcRect.roundOut(&iSrc);
            if (!bitmap.extractSubset(&tmp, iSrc)) {
                return;     // extraction failed
            }
            bitmapPtr = &tmp;
            srcRect.offset(SkIntToScalar(-iSrc.fLeft), SkIntToScalar(-iSrc.fTop));
            // The source rect has changed so update the matrix
            newM.preTranslate(SkIntToScalar(iSrc.fLeft), SkIntToScalar(iSrc.fTop));
        }

        SkPaint paintWithTexture(paint);
        paintWithTexture.setShader(SkShader::CreateBitmapShader(*bitmapPtr,
            SkShader::kClamp_TileMode, SkShader::kClamp_TileMode))->unref();

        // Transform 'newM' needs to be concatenated to the current matrix,
        // rather than transforming the primitive directly, so that 'newM' will
        // also affect the behavior of the mask filter.
        SkMatrix drawMatrix;
        drawMatrix.setConcat(fContext->getMatrix(), newM);
        SkDraw transformedDraw(draw);
        transformedDraw.fMatrix = &drawMatrix;

        this->drawRect(transformedDraw, srcRect, paintWithTexture);

        return;
    }

    GrPaint grPaint;

    bool alphaOnly = !(SkBitmap::kA8_Config == bitmap.config());
    if (!skPaint2GrPaintNoShader(this, paint, alphaOnly, false, &grPaint)) {
        return;
    }
    GrTextureParams params;
    params.setBilerp(paint.isFilterBitmap());

    if (!this->shouldTileBitmap(bitmap, params, srcRectPtr)) {
        // take the simple case
        this->internalDrawBitmap(bitmap, srcRect, m, params, &grPaint);
    } else {
        this->drawTiledBitmap(bitmap, srcRect, m, params, &grPaint);
    }
}

// Break 'bitmap' into several tiles to draw it since it has already
// been determined to be too large to fit in VRAM
void SkGpuDevice::drawTiledBitmap(const SkBitmap& bitmap,
                                  const SkRect& srcRect,
                                  const SkMatrix& m,
                                  const GrTextureParams& params,
                                  GrPaint* grPaint) {
    const int maxTextureSize = fContext->getMaxTextureSize();

    int tileSize = determine_tile_size(bitmap, srcRect, maxTextureSize);

    // compute clip bounds in local coordinates
    SkRect clipRect;
    {
        const GrRenderTarget* rt = fContext->getRenderTarget();
        clipRect.setWH(SkIntToScalar(rt->width()), SkIntToScalar(rt->height()));
        if (!fContext->getClip()->fClipStack->intersectRectWithClip(&clipRect)) {
            return;
        }
        SkMatrix matrix, inverse;
        matrix.setConcat(fContext->getMatrix(), m);
        if (!matrix.invert(&inverse)) {
            return;
        }
        inverse.mapRect(&clipRect);
    }

    int nx = bitmap.width() / tileSize;
    int ny = bitmap.height() / tileSize;
    for (int x = 0; x <= nx; x++) {
        for (int y = 0; y <= ny; y++) {
            SkRect tileR;
            tileR.set(SkIntToScalar(x * tileSize),
                      SkIntToScalar(y * tileSize),
                      SkIntToScalar((x + 1) * tileSize),
                      SkIntToScalar((y + 1) * tileSize));

            if (!SkRect::Intersects(tileR, clipRect)) {
                continue;
            }

            if (!tileR.intersect(srcRect)) {
                continue;
            }

            SkBitmap tmpB;
            SkIRect iTileR;
            tileR.roundOut(&iTileR);
            if (bitmap.extractSubset(&tmpB, iTileR)) {
                // now offset it to make it "local" to our tmp bitmap
                tileR.offset(SkIntToScalar(-iTileR.fLeft), SkIntToScalar(-iTileR.fTop));
                SkMatrix tmpM(m);
                tmpM.preTranslate(SkIntToScalar(iTileR.fLeft),
                                  SkIntToScalar(iTileR.fTop));

                this->internalDrawBitmap(tmpB, tileR, tmpM, params, grPaint);
            }
        }
    }
}

namespace {

bool hasAlignedSamples(const SkRect& srcRect, const SkRect& transformedRect) {
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

bool mayColorBleed(const SkRect& srcRect, const SkRect& transformedRect,
                   const SkMatrix& m) {
    // Only gets called if hasAlignedSamples returned false.
    // So we can assume that sampling is axis aligned but not texel aligned.
    GrAssert(!hasAlignedSamples(srcRect, transformedRect));
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

} // unnamed namespace

/*
 *  This is called by drawBitmap(), which has to handle images that may be too
 *  large to be represented by a single texture.
 *
 *  internalDrawBitmap assumes that the specified bitmap will fit in a texture
 *  and that non-texture portion of the GrPaint has already been setup.
 */
void SkGpuDevice::internalDrawBitmap(const SkBitmap& bitmap,
                                     const SkRect& srcRect,
                                     const SkMatrix& m,
                                     const GrTextureParams& params,
                                     GrPaint* grPaint) {
    SkASSERT(bitmap.width() <= fContext->getMaxTextureSize() &&
             bitmap.height() <= fContext->getMaxTextureSize());

    SkAutoLockPixels alp(bitmap, !bitmap.getTexture());
    if (!bitmap.getTexture() && !bitmap.readyToDraw()) {
        SkDebugf("nothing to draw\n");
        return;
    }

    GrTexture* texture;
    SkAutoCachedTexture act(this, bitmap, &params, &texture);
    if (NULL == texture) {
        return;
    }

    GrRect dstRect(srcRect);
    GrRect paintRect;
    SkScalar wInv = SkScalarInvert(SkIntToScalar(bitmap.width()));
    SkScalar hInv = SkScalarInvert(SkIntToScalar(bitmap.height()));
    paintRect.setLTRB(SkScalarMul(srcRect.fLeft,   wInv),
                      SkScalarMul(srcRect.fTop,    hInv),
                      SkScalarMul(srcRect.fRight,  wInv),
                      SkScalarMul(srcRect.fBottom, hInv));

    bool needsTextureDomain = false;
    if (params.isBilerp()) {
        // Need texture domain if drawing a sub rect.
        needsTextureDomain = srcRect.width() < bitmap.width() ||
                             srcRect.height() < bitmap.height();
        if (m.rectStaysRect() && fContext->getMatrix().rectStaysRect()) {
            // sampling is axis-aligned
            GrRect transformedRect;
            SkMatrix srcToDeviceMatrix(m);
            srcToDeviceMatrix.postConcat(fContext->getMatrix());
            srcToDeviceMatrix.mapRect(&transformedRect, srcRect);

            if (hasAlignedSamples(srcRect, transformedRect)) {
                // We could also turn off filtering here (but we already did a cache lookup with
                // params).
                needsTextureDomain = false;
            } else {
                needsTextureDomain = needsTextureDomain &&
                    mayColorBleed(srcRect, transformedRect, m);
            }
        }
    }

    GrRect textureDomain = GrRect::MakeEmpty();
    SkAutoTUnref<GrEffectRef> effect;
    if (needsTextureDomain) {
        // Use a constrained texture domain to avoid color bleeding
        SkScalar left, top, right, bottom;
        if (srcRect.width() > SK_Scalar1) {
            SkScalar border = SK_ScalarHalf / bitmap.width();
            left = paintRect.left() + border;
            right = paintRect.right() - border;
        } else {
            left = right = SkScalarHalf(paintRect.left() + paintRect.right());
        }
        if (srcRect.height() > SK_Scalar1) {
            SkScalar border = SK_ScalarHalf / bitmap.height();
            top = paintRect.top() + border;
            bottom = paintRect.bottom() - border;
        } else {
            top = bottom = SkScalarHalf(paintRect.top() + paintRect.bottom());
        }
        textureDomain.setLTRB(left, top, right, bottom);
        effect.reset(GrTextureDomainEffect::Create(texture,
                                                   SkMatrix::I(),
                                                   textureDomain,
                                                   GrTextureDomainEffect::kClamp_WrapMode,
                                                   params.isBilerp()));
    } else {
        effect.reset(GrSimpleTextureEffect::Create(texture, SkMatrix::I(), params));
    }
    grPaint->colorStage(kBitmapEffectIdx)->setEffect(effect);
    fContext->drawRectToRect(*grPaint, dstRect, paintRect, &m);
}

namespace {

void apply_effect(GrContext* context,
                  GrTexture* srcTexture,
                  GrTexture* dstTexture,
                  const GrRect& rect,
                  GrEffectRef* effect) {
    SkASSERT(srcTexture && srcTexture->getContext() == context);
    GrContext::AutoMatrix am;
    am.setIdentity(context);
    GrContext::AutoRenderTarget art(context, dstTexture->asRenderTarget());
    GrContext::AutoClip acs(context, rect);

    GrPaint paint;
    paint.colorStage(0)->setEffect(effect);
    context->drawRect(paint, rect);
}

};

static SkBitmap wrap_texture(GrTexture* texture) {
    SkBitmap result;
    bool dummy;
    SkBitmap::Config config = grConfig2skConfig(texture->config(), &dummy);
    result.setConfig(config, texture->width(), texture->height());
    result.setPixelRef(SkNEW_ARGS(SkGrPixelRef, (texture)))->unref();
    return result;
}

static bool filter_texture(SkDevice* device, GrContext* context,
                           GrTexture* texture, SkImageFilter* filter,
                           int w, int h, SkBitmap* result) {
    GrAssert(filter);
    SkDeviceImageFilterProxy proxy(device);

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit,
    desc.fWidth = w;
    desc.fHeight = h;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    GrEffectRef* effect;

    if (filter->canFilterImageGPU()) {
        // Save the render target and set it to NULL, so we don't accidentally draw to it in the
        // filter.  Also set the clip wide open and the matrix to identity.
        GrContext::AutoWideOpenIdentityDraw awo(context, NULL);
        return filter->filterImageGPU(&proxy, wrap_texture(texture), result);
    } else if (filter->asNewEffect(&effect, texture)) {
        GrAutoScratchTexture dst(context, desc);
        SkRect r = SkRect::MakeWH(SkIntToScalar(w), SkIntToScalar(h));
        apply_effect(context, texture, dst.texture(), r, effect);
        SkAutoTUnref<GrTexture> resultTex(dst.detach());
        effect->unref();
        *result = wrap_texture(resultTex.get());
        return true;
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

    GrPaint grPaint;
    if(!skPaint2GrPaintNoShader(this, paint, true, false, &grPaint)) {
        return;
    }

    GrEffectStage* stage = grPaint.colorStage(kBitmapEffectIdx);

    GrTexture* texture;
    stage->reset();
    // draw sprite uses the default texture params
    SkAutoCachedTexture act(this, bitmap, NULL, &texture);
    grPaint.colorStage(kBitmapEffectIdx)->setEffect(
        GrSimpleTextureEffect::Create(texture, SkMatrix::I()))->unref();

    SkImageFilter* filter = paint.getImageFilter();
    if (NULL != filter) {
        SkBitmap filterBitmap;
        if (filter_texture(this, fContext, texture, filter, w, h, &filterBitmap)) {
            grPaint.colorStage(kBitmapEffectIdx)->setEffect(
                GrSimpleTextureEffect::Create((GrTexture*) filterBitmap.getTexture(), SkMatrix::I()))->unref();
            texture = (GrTexture*) filterBitmap.getTexture();
            w = filterBitmap.width();
            h = filterBitmap.height();
        }
    }

    fContext->drawRectToRect(grPaint,
                            GrRect::MakeXYWH(SkIntToScalar(left),
                                            SkIntToScalar(top),
                                            SkIntToScalar(w),
                                            SkIntToScalar(h)),
                            GrRect::MakeWH(SK_Scalar1 * w / texture->width(),
                                        SK_Scalar1 * h / texture->height()));
}

void SkGpuDevice::drawBitmapRect(const SkDraw& draw, const SkBitmap& bitmap,
                                 const SkRect* src, const SkRect& dst,
                                 const SkPaint& paint) {
    SkMatrix    matrix;
    SkRect      bitmapBounds, tmpSrc;

    bitmapBounds.set(0, 0,
                     SkIntToScalar(bitmap.width()),
                     SkIntToScalar(bitmap.height()));

    // Compute matrix from the two rectangles
    if (NULL != src) {
        tmpSrc = *src;
    } else {
        tmpSrc = bitmapBounds;
    }
    matrix.setRectToRect(tmpSrc, dst, SkMatrix::kFill_ScaleToFit);

    // clip the tmpSrc to the bounds of the bitmap. No check needed if src==null.
    if (NULL != src) {
        if (!bitmapBounds.contains(tmpSrc)) {
            if (!tmpSrc.intersect(bitmapBounds)) {
                return; // nothing to draw
            }
        }
    }

    this->drawBitmapCommon(draw, bitmap, &tmpSrc, matrix, paint);
}

void SkGpuDevice::drawDevice(const SkDraw& draw, SkDevice* device,
                             int x, int y, const SkPaint& paint) {
    // clear of the source device must occur before CHECK_SHOULD_DRAW
    SkGpuDevice* dev = static_cast<SkGpuDevice*>(device);
    if (dev->fNeedClear) {
        // TODO: could check here whether we really need to draw at all
        dev->clear(0x0);
    }

    // drawDevice is defined to be in device coords.
    CHECK_SHOULD_DRAW(draw, true);

    GrPaint grPaint;
    grPaint.colorStage(kBitmapEffectIdx)->reset();
    if (!dev->bindDeviceAsTexture(&grPaint) ||
        !skPaint2GrPaintNoShader(this, paint, true, false, &grPaint)) {
        return;
    }

    GrTexture* devTex = (*grPaint.getColorStage(kBitmapEffectIdx).getEffect())->texture(0);
    SkASSERT(NULL != devTex);

    const SkBitmap& bm = dev->accessBitmap(false);
    int w = bm.width();
    int h = bm.height();

    SkImageFilter* filter = paint.getImageFilter();
    if (NULL != filter) {
        SkBitmap filterBitmap;
        if (filter_texture(this, fContext, devTex, filter, w, h, &filterBitmap)) {
            grPaint.colorStage(kBitmapEffectIdx)->setEffect(
                GrSimpleTextureEffect::Create((GrTexture*) filterBitmap.getTexture(), SkMatrix::I()))->unref();
            devTex = (GrTexture*) filterBitmap.getTexture();
            w = filterBitmap.width();
            h = filterBitmap.height();
        }
    }

    GrRect dstRect = GrRect::MakeXYWH(SkIntToScalar(x),
                                      SkIntToScalar(y),
                                      SkIntToScalar(w),
                                      SkIntToScalar(h));

    // The device being drawn may not fill up its texture (saveLayer uses
    // the approximate ).
    GrRect srcRect = GrRect::MakeWH(SK_Scalar1 * w / devTex->width(),
                                    SK_Scalar1 * h / devTex->height());

    fContext->drawRectToRect(grPaint, dstRect, srcRect);
}

bool SkGpuDevice::canHandleImageFilter(SkImageFilter* filter) {
    if (!filter->asNewEffect(NULL, NULL) &&
        !filter->canFilterImageGPU()) {
        return false;
    }
    return true;
}

bool SkGpuDevice::filterImage(SkImageFilter* filter, const SkBitmap& src,
                              const SkMatrix& ctm,
                              SkBitmap* result, SkIPoint* offset) {
    // want explicitly our impl, so guard against a subclass of us overriding it
    if (!this->SkGpuDevice::canHandleImageFilter(filter)) {
        return false;
    }

    SkAutoLockPixels alp(src, !src.getTexture());
    if (!src.getTexture() && !src.readyToDraw()) {
        return false;
    }

    GrPaint paint;

    GrTexture* texture;
    // We assume here that the filter will not attempt to tile the src. Otherwise, this cache lookup
    // must be pushed upstack.
    SkAutoCachedTexture act(this, src, NULL, &texture);

    return filter_texture(this, fContext, texture, filter, src.width(), src.height(), result);
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

    GrPaint grPaint;
    // we ignore the shader if texs is null.
    if (NULL == texs) {
        if (!skPaint2GrPaintNoShader(this, paint, false, NULL == colors, &grPaint)) {
            return;
        }
    } else {
        if (!skPaint2GrPaintShader(this, paint, NULL == colors, &grPaint)) {
            return;
        }
    }

    if (NULL != xmode && NULL != texs && NULL != colors) {
        if (!SkXfermode::IsMode(xmode, SkXfermode::kModulate_Mode)) {
            SkDebugf("Unsupported vertex-color/texture xfer mode.\n");
#if 0
            return
#endif
        }
    }

    SkAutoSTMalloc<128, GrColor> convertedColors(0);
    if (NULL != colors) {
        // need to convert byte order and from non-PM to PM
        convertedColors.reset(vertexCount);
        for (int i = 0; i < vertexCount; ++i) {
            convertedColors[i] = SkColor2GrColor(colors[i]);
        }
        colors = convertedColors.get();
    }
    fContext->drawVertices(grPaint,
                           gVertexMode2PrimitiveType[vmode],
                           vertexCount,
                           (GrPoint*) vertices,
                           (GrPoint*) texs,
                           colors,
                           indices,
                           indexCount);
}

///////////////////////////////////////////////////////////////////////////////

static void GlyphCacheAuxProc(void* data) {
    GrFontScaler* scaler = (GrFontScaler*)data;
    SkSafeUnref(scaler);
}

static GrFontScaler* get_gr_font_scaler(SkGlyphCache* cache) {
    void* auxData;
    GrFontScaler* scaler = NULL;
    if (cache->getAuxProcData(GlyphCacheAuxProc, &auxData)) {
        scaler = (GrFontScaler*)auxData;
    }
    if (NULL == scaler) {
        scaler = SkNEW_ARGS(SkGrFontScaler, (cache));
        cache->setAuxProc(GlyphCacheAuxProc, scaler);
    }
    return scaler;
}

static void SkGPU_Draw1Glyph(const SkDraw1Glyph& state,
                             SkFixed fx, SkFixed fy,
                             const SkGlyph& glyph) {
    SkASSERT(glyph.fWidth > 0 && glyph.fHeight > 0);

    GrSkDrawProcs* procs = static_cast<GrSkDrawProcs*>(state.fDraw->fProcs);

    if (NULL == procs->fFontScaler) {
        procs->fFontScaler = get_gr_font_scaler(state.fCache);
    }

    procs->fTextContext->drawPackedGlyph(GrGlyph::Pack(glyph.getGlyphID(),
                                                       glyph.getSubXFixed(),
                                                       glyph.getSubYFixed()),
                                         SkFixedFloorToFixed(fx),
                                         SkFixedFloorToFixed(fy),
                                         procs->fFontScaler);
}

SkDrawProcs* SkGpuDevice::initDrawForText(GrTextContext* context) {

    // deferred allocation
    if (NULL == fDrawProcs) {
        fDrawProcs = SkNEW(GrSkDrawProcs);
        fDrawProcs->fD1GProc = SkGPU_Draw1Glyph;
        fDrawProcs->fContext = fContext;
    }

    // init our (and GL's) state
    fDrawProcs->fTextContext = context;
    fDrawProcs->fFontScaler = NULL;
    return fDrawProcs;
}

void SkGpuDevice::drawText(const SkDraw& draw, const void* text,
                          size_t byteLength, SkScalar x, SkScalar y,
                          const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw, false);

    if (fContext->getMatrix().hasPerspective()) {
        // this guy will just call our drawPath()
        draw.drawText((const char*)text, byteLength, x, y, paint);
    } else {
        SkDraw myDraw(draw);

        GrPaint grPaint;
        if (!skPaint2GrPaintShader(this, paint, true, &grPaint)) {
            return;
        }
        GrTextContext context(fContext, grPaint);
        myDraw.fProcs = this->initDrawForText(&context);
        this->INHERITED::drawText(myDraw, text, byteLength, x, y, paint);
    }
}

void SkGpuDevice::drawPosText(const SkDraw& draw, const void* text,
                             size_t byteLength, const SkScalar pos[],
                             SkScalar constY, int scalarsPerPos,
                             const SkPaint& paint) {
    CHECK_SHOULD_DRAW(draw, false);

    if (fContext->getMatrix().hasPerspective()) {
        // this guy will just call our drawPath()
        draw.drawPosText((const char*)text, byteLength, pos, constY,
                         scalarsPerPos, paint);
    } else {
        SkDraw myDraw(draw);

        GrPaint grPaint;
        if (!skPaint2GrPaintShader(this, paint, true, &grPaint)) {
            return;
        }
        GrTextContext context(fContext, grPaint);
        myDraw.fProcs = this->initDrawForText(&context);
        this->INHERITED::drawPosText(myDraw, text, byteLength, pos, constY,
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
        // turn off lcd
        flags->fFlags = paint.getFlags() & ~SkPaint::kLCDRenderText_Flag;
        flags->fHinting = paint.getHinting();
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

SkDevice* SkGpuDevice::onCreateCompatibleDevice(SkBitmap::Config config,
                                                int width, int height,
                                                bool isOpaque,
                                                Usage usage) {
    GrTextureDesc desc;
    desc.fConfig = fRenderTarget->config();
    desc.fFlags = kRenderTarget_GrTextureFlagBit;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fSampleCnt = fRenderTarget->numSamples();

    SkAutoTUnref<GrTexture> texture;
    // Skia's convention is to only clear a device if it is non-opaque.
    bool needClear = !isOpaque;

#if CACHE_COMPATIBLE_DEVICE_TEXTURES
    // layers are never draw in repeat modes, so we can request an approx
    // match and ignore any padding.
    const GrContext::ScratchTexMatch match = (kSaveLayer_Usage == usage) ?
                                                GrContext::kApprox_ScratchTexMatch :
                                                GrContext::kExact_ScratchTexMatch;
    texture.reset(fContext->lockAndRefScratchTexture(desc, match));
#else
    texture.reset(fContext->createUncachedTexture(desc, NULL, 0));
#endif
    if (NULL != texture.get()) {
        return SkNEW_ARGS(SkGpuDevice,(fContext, texture, needClear));
    } else {
        GrPrintf("---- failed to create compatible device texture [%d %d]\n", width, height);
        return NULL;
    }
}

SkGpuDevice::SkGpuDevice(GrContext* context,
                         GrTexture* texture,
                         bool needClear)
    : SkDevice(make_bitmap(context, texture->asRenderTarget())) {

    GrAssert(texture && texture->asRenderTarget());
    // This constructor is called from onCreateCompatibleDevice. It has locked the RT in the texture
    // cache. We pass true for the third argument so that it will get unlocked.
    this->initFromRenderTarget(context, texture->asRenderTarget(), true);
    fNeedClear = needClear;
}
