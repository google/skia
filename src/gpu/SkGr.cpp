/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkGr.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "GrTextureParamsAdjuster.h"
#include "GrGpuResourcePriv.h"
#include "GrImageIDTextureAdjuster.h"
#include "GrXferProcessor.h"
#include "GrYUVProvider.h"

#include "SkColorFilter.h"
#include "SkConfig8888.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkErrorInternals.h"
#include "SkGrPixelRef.h"
#include "SkMessageBus.h"
#include "SkPixelRef.h"
#include "SkResourceCache.h"
#include "SkTextureCompressor.h"
#include "SkYUVPlanesCache.h"
#include "effects/GrBicubicEffect.h"
#include "effects/GrConstColorProcessor.h"
#include "effects/GrDitherEffect.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"
#include "effects/GrYUVtoRGBEffect.h"

#ifndef SK_IGNORE_ETC1_SUPPORT
#  include "ktx.h"
#  include "etc1.h"
#endif

GrSurfaceDesc GrImageInfoToSurfaceDesc(const SkImageInfo& info) {
    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fWidth = info.width();
    desc.fHeight = info.height();
    desc.fConfig = SkImageInfo2GrPixelConfig(info);
    desc.fSampleCnt = 0;
    return desc;
}

void GrMakeKeyFromImageID(GrUniqueKey* key, uint32_t imageID, const SkIRect& imageBounds) {
    SkASSERT(key);
    SkASSERT(imageID);
    SkASSERT(!imageBounds.isEmpty());
    static const GrUniqueKey::Domain kImageIDDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(key, kImageIDDomain, 5);
    builder[0] = imageID;
    builder[1] = imageBounds.fLeft;
    builder[2] = imageBounds.fTop;
    builder[3] = imageBounds.fRight;
    builder[4] = imageBounds.fBottom;
}

GrPixelConfig GrIsCompressedTextureDataSupported(GrContext* ctx, SkData* data,
                                                 int expectedW, int expectedH,
                                                 const void** outStartOfDataToUpload) {
    *outStartOfDataToUpload = nullptr;
#ifndef SK_IGNORE_ETC1_SUPPORT
    if (!ctx->caps()->isConfigTexturable(kETC1_GrPixelConfig)) {
        return kUnknown_GrPixelConfig;
    }

    const uint8_t* bytes = data->bytes();
    if (data->size() > ETC_PKM_HEADER_SIZE && etc1_pkm_is_valid(bytes)) {
        // Does the data match the dimensions of the bitmap? If not,
        // then we don't know how to scale the image to match it...
        if (etc1_pkm_get_width(bytes) != (unsigned)expectedW ||
            etc1_pkm_get_height(bytes) != (unsigned)expectedH)
        {
            return kUnknown_GrPixelConfig;
        }

        *outStartOfDataToUpload = bytes + ETC_PKM_HEADER_SIZE;
        return kETC1_GrPixelConfig;
    } else if (SkKTXFile::is_ktx(bytes)) {
        SkKTXFile ktx(data);

        // Is it actually an ETC1 texture?
        if (!ktx.isCompressedFormat(SkTextureCompressor::kETC1_Format)) {
            return kUnknown_GrPixelConfig;
        }

        // Does the data match the dimensions of the bitmap? If not,
        // then we don't know how to scale the image to match it...
        if (ktx.width() != expectedW || ktx.height() != expectedH) {
            return kUnknown_GrPixelConfig;
        }

        *outStartOfDataToUpload = ktx.pixelData();
        return kETC1_GrPixelConfig;
    }
#endif
    return kUnknown_GrPixelConfig;
}

//////////////////////////////////////////////////////////////////////////////

/**
 * Fill out buffer with the compressed format Ganesh expects from a colortable
 * based bitmap. [palette (colortable) + indices].
 *
 * At the moment Ganesh only supports 8bit version. If Ganesh allowed we others
 * we could detect that the colortable.count is <= 16, and then repack the
 * indices as nibbles to save RAM, but it would take more time (i.e. a lot
 * slower than memcpy), so skipping that for now.
 *
 * Ganesh wants a full 256 palette entry, even though Skia's ctable is only as big
 * as the colortable.count says it is.
 */
static void build_index8_data(void* buffer, const SkBitmap& bitmap) {
    SkASSERT(kIndex_8_SkColorType == bitmap.colorType());

    SkAutoLockPixels alp(bitmap);
    if (!bitmap.readyToDraw()) {
        SkDEBUGFAIL("bitmap not ready to draw!");
        return;
    }

    SkColorTable* ctable = bitmap.getColorTable();
    char* dst = (char*)buffer;

    const int count = ctable->count();

    SkDstPixelInfo dstPI;
    dstPI.fColorType = kRGBA_8888_SkColorType;
    dstPI.fAlphaType = kPremul_SkAlphaType;
    dstPI.fPixels = buffer;
    dstPI.fRowBytes = count * sizeof(SkPMColor);

    SkSrcPixelInfo srcPI;
    srcPI.fColorType = kN32_SkColorType;
    srcPI.fAlphaType = kPremul_SkAlphaType;
    srcPI.fPixels = ctable->readColors();
    srcPI.fRowBytes = count * sizeof(SkPMColor);

    srcPI.convertPixelsTo(&dstPI, count, 1);

    // always skip a full 256 number of entries, even if we memcpy'd fewer
    dst += 256 * sizeof(GrColor);

    if ((unsigned)bitmap.width() == bitmap.rowBytes()) {
        memcpy(dst, bitmap.getPixels(), bitmap.getSize());
    } else {
        // need to trim off the extra bytes per row
        size_t width = bitmap.width();
        size_t rowBytes = bitmap.rowBytes();
        const char* src = (const char*)bitmap.getPixels();
        for (int y = 0; y < bitmap.height(); y++) {
            memcpy(dst, src, width);
            src += rowBytes;
            dst += width;
        }
    }
}

/**
 *  Once we have made SkImages handle all lazy/deferred/generated content, the YUV apis will
 *  be gone from SkPixelRef, and we can remove this subclass entirely.
 */
class PixelRef_GrYUVProvider : public GrYUVProvider {
    SkPixelRef* fPR;

public:
    PixelRef_GrYUVProvider(SkPixelRef* pr) : fPR(pr) {}

    uint32_t onGetID() override { return fPR->getGenerationID(); }
    bool onGetYUVSizes(SkISize sizes[3]) override {
        return fPR->getYUV8Planes(sizes, nullptr, nullptr, nullptr);
    }
    bool onGetYUVPlanes(SkISize sizes[3], void* planes[3], size_t rowBytes[3],
                        SkYUVColorSpace* space) override {
        return fPR->getYUV8Planes(sizes, planes, rowBytes, space);
    }
};

static GrTexture* create_texture_from_yuv(GrContext* ctx, const SkBitmap& bm,
                                          const GrSurfaceDesc& desc) {
    // Subsets are not supported, the whole pixelRef is loaded when using YUV decoding
    SkPixelRef* pixelRef = bm.pixelRef();
    if ((nullptr == pixelRef) ||
        (pixelRef->info().width() != bm.info().width()) ||
        (pixelRef->info().height() != bm.info().height())) {
        return nullptr;
    }

    PixelRef_GrYUVProvider provider(pixelRef);

    return provider.refAsTexture(ctx, desc, !bm.isVolatile());
}

static GrTexture* load_etc1_texture(GrContext* ctx, const SkBitmap &bm, GrSurfaceDesc desc) {
    SkAutoTUnref<SkData> data(bm.pixelRef()->refEncodedData());
    if (!data) {
        return nullptr;
    }

    const void* startOfTexData;
    desc.fConfig = GrIsCompressedTextureDataSupported(ctx, data, bm.width(), bm.height(),
                                                      &startOfTexData);
    if (kUnknown_GrPixelConfig == desc.fConfig) {
        return nullptr;
    }

    return ctx->textureProvider()->createTexture(desc, true, startOfTexData, 0);
}

GrTexture* GrUploadBitmapToTexture(GrContext* ctx, const SkBitmap& bmp) {
    SkASSERT(!bmp.getTexture());

    SkBitmap tmpBitmap;
    const SkBitmap* bitmap = &bmp;

    GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(bitmap->info());
    const GrCaps* caps = ctx->caps();

    if (kIndex_8_SkColorType == bitmap->colorType()) {
        if (caps->isConfigTexturable(kIndex_8_GrPixelConfig)) {
            size_t imageSize = GrCompressedFormatDataSize(kIndex_8_GrPixelConfig,
                                                          bitmap->width(), bitmap->height());
            SkAutoMalloc storage(imageSize);
            build_index8_data(storage.get(), bmp);

            // our compressed data will be trimmed, so pass width() for its
            // "rowBytes", since they are the same now.
            return ctx->textureProvider()->createTexture(desc, true, storage.get(),
                                                         bitmap->width());
        } else {
            bmp.copyTo(&tmpBitmap, kN32_SkColorType);
            // now bitmap points to our temp, which has been promoted to 32bits
            bitmap = &tmpBitmap;
            desc.fConfig = SkImageInfo2GrPixelConfig(bitmap->info());
        }
    } else if (!bitmap->readyToDraw()) {
        // If the bitmap had compressed data and was then uncompressed, it'll still return
        // compressed data on 'refEncodedData' and upload it. Probably not good, since if
        // the bitmap has available pixels, then they might not be what the decompressed
        // data is.

        // Really?? We aren't doing this with YUV.

        GrTexture *texture = load_etc1_texture(ctx, *bitmap, desc);
        if (texture) {
            return texture;
        }
    }

    GrTexture *texture = create_texture_from_yuv(ctx, *bitmap, desc);
    if (texture) {
        return texture;
    }

    SkAutoLockPixels alp(*bitmap);
    if (!bitmap->readyToDraw()) {
        return nullptr;
    }

    return ctx->textureProvider()->createTexture(desc, true, bitmap->getPixels(),
                                                 bitmap->rowBytes());
}


////////////////////////////////////////////////////////////////////////////////

void GrInstallBitmapUniqueKeyInvalidator(const GrUniqueKey& key, SkPixelRef* pixelRef) {
    class Invalidator : public SkPixelRef::GenIDChangeListener {
    public:
        explicit Invalidator(const GrUniqueKey& key) : fMsg(key) {}
    private:
        GrUniqueKeyInvalidatedMessage fMsg;

        void onChange() override { SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(fMsg); }
    };

    pixelRef->addGenIDChangeListener(new Invalidator(key));
}

GrTexture* GrRefCachedBitmapTexture(GrContext* ctx, const SkBitmap& bitmap,
                                    const GrTextureParams& params) {
    if (bitmap.getTexture()) {
        return GrBitmapTextureAdjuster(&bitmap).refTextureSafeForParams(params, nullptr);
    }
    return GrBitmapTextureMaker(ctx, bitmap).refTextureForParams(params);
}

///////////////////////////////////////////////////////////////////////////////

// alphatype is ignore for now, but if GrPixelConfig is expanded to encompass
// alpha info, that will be considered.
GrPixelConfig SkImageInfo2GrPixelConfig(SkColorType ct, SkAlphaType, SkColorProfileType pt) {
    switch (ct) {
        case kUnknown_SkColorType:
            return kUnknown_GrPixelConfig;
        case kAlpha_8_SkColorType:
            return kAlpha_8_GrPixelConfig;
        case kRGB_565_SkColorType:
            return kRGB_565_GrPixelConfig;
        case kARGB_4444_SkColorType:
            return kRGBA_4444_GrPixelConfig;
        case kRGBA_8888_SkColorType:
            //if (kSRGB_SkColorProfileType == pt) {
            //    return kSRGBA_8888_GrPixelConfig;
            //}
            return kRGBA_8888_GrPixelConfig;
        case kBGRA_8888_SkColorType:
            return kBGRA_8888_GrPixelConfig;
        case kIndex_8_SkColorType:
            return kIndex_8_GrPixelConfig;
        case kGray_8_SkColorType:
            return kAlpha_8_GrPixelConfig; // TODO: gray8 support on gpu
    }
    SkASSERT(0);    // shouldn't get here
    return kUnknown_GrPixelConfig;
}

bool GrPixelConfig2ColorAndProfileType(GrPixelConfig config, SkColorType* ctOut,
                                       SkColorProfileType* ptOut) {
    SkColorType ct;
    SkColorProfileType pt = kLinear_SkColorProfileType;
    switch (config) {
        case kAlpha_8_GrPixelConfig:
            ct = kAlpha_8_SkColorType;
            break;
        case kIndex_8_GrPixelConfig:
            ct = kIndex_8_SkColorType;
            break;
        case kRGB_565_GrPixelConfig:
            ct = kRGB_565_SkColorType;
            break;
        case kRGBA_4444_GrPixelConfig:
            ct = kARGB_4444_SkColorType;
            break;
        case kRGBA_8888_GrPixelConfig:
            ct = kRGBA_8888_SkColorType;
            break;
        case kBGRA_8888_GrPixelConfig:
            ct = kBGRA_8888_SkColorType;
            break;
        case kSRGBA_8888_GrPixelConfig:
            ct = kRGBA_8888_SkColorType;
            pt = kSRGB_SkColorProfileType;
            break;
        default:
            return false;
    }
    if (ctOut) {
        *ctOut = ct;
    }
    if (ptOut) {
        *ptOut = pt;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////

static inline bool blend_requires_shader(const SkXfermode::Mode mode, bool primitiveIsSrc) {
    if (primitiveIsSrc) {
        return SkXfermode::kSrc_Mode != mode;
    } else {
        return SkXfermode::kDst_Mode != mode;
    }
}

static inline bool skpaint_to_grpaint_impl(GrContext* context,
                                           const SkPaint& skPaint,
                                           const SkMatrix& viewM,
                                           const GrFragmentProcessor** shaderProcessor,
                                           SkXfermode::Mode* primColorMode,
                                           bool primitiveIsSrc,
                                           GrPaint* grPaint) {
    grPaint->setAntiAlias(skPaint.isAntiAlias());

    // Setup the initial color considering the shader, the SkPaint color, and the presence or not
    // of per-vertex colors.
    SkAutoTUnref<const GrFragmentProcessor> aufp;
    const GrFragmentProcessor* shaderFP = nullptr;
    if (!primColorMode || blend_requires_shader(*primColorMode, primitiveIsSrc)) {
        if (shaderProcessor) {
            shaderFP = *shaderProcessor;
        } else if (const SkShader* shader = skPaint.getShader()) {
            aufp.reset(shader->asFragmentProcessor(context, viewM, nullptr,
                                                   skPaint.getFilterQuality()));
            shaderFP = aufp;
            if (!shaderFP) {
                return false;
            }
        }
    }

    // Set this in below cases if the output of the shader/paint-color/paint-alpha/primXfermode is
    // a known constant value. In that case we can simply apply a color filter during this
    // conversion without converting the color filter to a GrFragmentProcessor.
    bool applyColorFilterToPaintColor = false;
    if (shaderFP) {
        if (primColorMode) {
            // There is a blend between the primitive color and the shader color. The shader sees
            // the opaque paint color. The shader's output is blended using the provided mode by
            // the primitive color. The blended color is then modulated by the paint's alpha.

            // The geometry processor will insert the primitive color to start the color chain, so
            // the GrPaint color will be ignored.

            GrColor shaderInput = SkColorToOpaqueGrColor(skPaint.getColor());

            shaderFP = GrFragmentProcessor::OverrideInput(shaderFP, shaderInput);
            aufp.reset(shaderFP);

            if (primitiveIsSrc) {
                shaderFP = GrXfermodeFragmentProcessor::CreateFromDstProcessor(shaderFP,
                                                                               *primColorMode);
            } else {
                shaderFP = GrXfermodeFragmentProcessor::CreateFromSrcProcessor(shaderFP,
                                                                               *primColorMode);
            }
            aufp.reset(shaderFP);
            // The above may return null if compose results in a pass through of the prim color.
            if (shaderFP) {
                grPaint->addColorFragmentProcessor(shaderFP);
            }

            GrColor paintAlpha = SkColorAlphaToGrColor(skPaint.getColor());
            if (GrColor_WHITE != paintAlpha) {
                grPaint->addColorFragmentProcessor(GrConstColorProcessor::Create(
                    paintAlpha, GrConstColorProcessor::kModulateRGBA_InputMode))->unref();
            }
        } else {
            // The shader's FP sees the paint unpremul color
            grPaint->setColor(SkColorToUnpremulGrColor(skPaint.getColor()));
            grPaint->addColorFragmentProcessor(shaderFP);
        }
    } else {
        if (primColorMode) {
            // There is a blend between the primitive color and the paint color. The blend considers
            // the opaque paint color. The paint's alpha is applied to the post-blended color.
            SkAutoTUnref<const GrFragmentProcessor> processor(
                GrConstColorProcessor::Create(SkColorToOpaqueGrColor(skPaint.getColor()),
                                              GrConstColorProcessor::kIgnore_InputMode));
            if (primitiveIsSrc) {
                processor.reset(GrXfermodeFragmentProcessor::CreateFromDstProcessor(processor,
                                                                                *primColorMode));
            } else {
                processor.reset(GrXfermodeFragmentProcessor::CreateFromSrcProcessor(processor,
                                                                                *primColorMode));

            }
            if (processor) {
                grPaint->addColorFragmentProcessor(processor);
            }

            grPaint->setColor(SkColorToOpaqueGrColor(skPaint.getColor()));

            GrColor paintAlpha = SkColorAlphaToGrColor(skPaint.getColor());
            if (GrColor_WHITE != paintAlpha) {
                grPaint->addColorFragmentProcessor(GrConstColorProcessor::Create(
                    paintAlpha, GrConstColorProcessor::kModulateRGBA_InputMode))->unref();
            }
        } else {
            // No shader, no primitive color.
            grPaint->setColor(SkColorToPremulGrColor(skPaint.getColor()));
            applyColorFilterToPaintColor = true;
        }
    }

    SkColorFilter* colorFilter = skPaint.getColorFilter();
    if (colorFilter) {
        if (applyColorFilterToPaintColor) {
            grPaint->setColor(SkColorToPremulGrColor(colorFilter->filterColor(skPaint.getColor())));
        } else {
            SkAutoTUnref<const GrFragmentProcessor> cfFP(
                colorFilter->asFragmentProcessor(context));
            if (cfFP) {
                grPaint->addColorFragmentProcessor(cfFP);
            } else {
                return false;
            }
        }
    }

    SkXfermode* mode = skPaint.getXfermode();
    GrXPFactory* xpFactory = nullptr;
    SkXfermode::AsXPFactory(mode, &xpFactory);
    SkSafeUnref(grPaint->setXPFactory(xpFactory));

#ifndef SK_IGNORE_GPU_DITHER
    if (skPaint.isDither() && grPaint->numColorFragmentProcessors() > 0) {
        grPaint->addColorFragmentProcessor(GrDitherEffect::Create())->unref();
    }
#endif
    return true;
}

bool SkPaintToGrPaint(GrContext* context, const SkPaint& skPaint, const SkMatrix& viewM,
                      GrPaint* grPaint) {
    return skpaint_to_grpaint_impl(context, skPaint, viewM, nullptr, nullptr, false, grPaint);
}

/** Replaces the SkShader (if any) on skPaint with the passed in GrFragmentProcessor. */
bool SkPaintToGrPaintReplaceShader(GrContext* context,
                                   const SkPaint& skPaint,
                                   const GrFragmentProcessor* shaderFP,
                                   GrPaint* grPaint) {
    if (!shaderFP) {
        return false;
    }
    return skpaint_to_grpaint_impl(context, skPaint, SkMatrix::I(), &shaderFP, nullptr, false,
                                   grPaint);
}

/** Ignores the SkShader (if any) on skPaint. */
bool SkPaintToGrPaintNoShader(GrContext* context,
                              const SkPaint& skPaint,
                              GrPaint* grPaint) {
    // Use a ptr to a nullptr to to indicate that the SkShader is ignored and not replaced.
    static const GrFragmentProcessor* kNullShaderFP = nullptr;
    static const GrFragmentProcessor** kIgnoreShader = &kNullShaderFP;
    return skpaint_to_grpaint_impl(context, skPaint, SkMatrix::I(), kIgnoreShader, nullptr, false,
                                   grPaint);
}

/** Blends the SkPaint's shader (or color if no shader) with a per-primitive color which must
be setup as a vertex attribute using the specified SkXfermode::Mode. */
bool SkPaintToGrPaintWithXfermode(GrContext* context,
                                  const SkPaint& skPaint,
                                  const SkMatrix& viewM,
                                  SkXfermode::Mode primColorMode,
                                  bool primitiveIsSrc,
                                  GrPaint* grPaint) {
    return skpaint_to_grpaint_impl(context, skPaint, viewM, nullptr, &primColorMode, primitiveIsSrc,
                                   grPaint);
}

bool SkPaintToGrPaintWithTexture(GrContext* context,
                                 const SkPaint& paint,
                                 const SkMatrix& viewM,
                                 const GrFragmentProcessor* fp,
                                 bool textureIsAlphaOnly,
                                 GrPaint* grPaint) {
    SkAutoTUnref<const GrFragmentProcessor> shaderFP;
    if (textureIsAlphaOnly) {
        if (const SkShader* shader = paint.getShader()) {
            shaderFP.reset(shader->asFragmentProcessor(context,
                                                       viewM,
                                                       nullptr,
                                                       paint.getFilterQuality()));
            if (!shaderFP) {
                return false;
            }
            const GrFragmentProcessor* fpSeries[] = { shaderFP.get(), fp };
            shaderFP.reset(GrFragmentProcessor::RunInSeries(fpSeries, 2));
        } else {
            shaderFP.reset(GrFragmentProcessor::MulOutputByInputUnpremulColor(fp));
        }
    } else {
        shaderFP.reset(GrFragmentProcessor::MulOutputByInputAlpha(fp));
    }

    return SkPaintToGrPaintReplaceShader(context, paint, shaderFP.get(), grPaint);
}


////////////////////////////////////////////////////////////////////////////////////////////////

SkImageInfo GrMakeInfoFromTexture(GrTexture* tex, int w, int h, bool isOpaque) {
#ifdef SK_DEBUG
    const GrSurfaceDesc& desc = tex->desc();
    SkASSERT(w <= desc.fWidth);
    SkASSERT(h <= desc.fHeight);
#endif
    const GrPixelConfig config = tex->config();
    SkColorType ct;
    SkAlphaType at = isOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
    if (!GrPixelConfig2ColorAndProfileType(config, &ct, nullptr)) {
        ct = kUnknown_SkColorType;
    }
    return SkImageInfo::Make(w, h, ct, at);
}


void GrWrapTextureInBitmap(GrTexture* src, int w, int h, bool isOpaque, SkBitmap* dst) {
    const SkImageInfo info = GrMakeInfoFromTexture(src, w, h, isOpaque);
    dst->setInfo(info);
    dst->setPixelRef(new SkGrPixelRef(info, src))->unref();
}

GrTextureParams::FilterMode GrSkFilterQualityToGrFilterMode(SkFilterQuality paintFilterQuality,
                                                            const SkMatrix& viewM,
                                                            const SkMatrix& localM,
                                                            bool* doBicubic) {
    *doBicubic = false;
    GrTextureParams::FilterMode textureFilterMode;
    switch (paintFilterQuality) {
        case kNone_SkFilterQuality:
            textureFilterMode = GrTextureParams::kNone_FilterMode;
            break;
        case kLow_SkFilterQuality:
            textureFilterMode = GrTextureParams::kBilerp_FilterMode;
            break;
        case kMedium_SkFilterQuality: {
            SkMatrix matrix;
            matrix.setConcat(viewM, localM);
            if (matrix.getMinScale() < SK_Scalar1) {
                textureFilterMode = GrTextureParams::kMipMap_FilterMode;
            } else {
                // Don't trigger MIP level generation unnecessarily.
                textureFilterMode = GrTextureParams::kBilerp_FilterMode;
            }
            break;
        }
        case kHigh_SkFilterQuality: {
            SkMatrix matrix;
            matrix.setConcat(viewM, localM);
            *doBicubic = GrBicubicEffect::ShouldUseBicubic(matrix, &textureFilterMode);
            break;
        }
        default:
            SkErrorInternals::SetError( kInvalidPaint_SkError,
                                        "Sorry, I don't understand the filtering "
                                        "mode you asked for.  Falling back to "
                                        "MIPMaps.");
            textureFilterMode = GrTextureParams::kMipMap_FilterMode;
            break;

    }
    return textureFilterMode;
}
