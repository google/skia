/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkGr.h"
#include "SkGrPriv.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "GrGpuResourcePriv.h"
#include "GrImageIDTextureAdjuster.h"
#include "GrTextureParamsAdjuster.h"
#include "GrTypes.h"
#include "GrXferProcessor.h"
#include "GrYUVProvider.h"

#include "SkColorFilter.h"
#include "SkConfig8888.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkErrorInternals.h"
#include "SkGrPixelRef.h"
#include "SkMessageBus.h"
#include "SkMipMap.h"
#include "SkPixelRef.h"
#include "SkResourceCache.h"
#include "SkTemplates.h"
#include "SkTextureCompressor.h"
#include "SkYUVPlanesCache.h"
#include "effects/GrBicubicEffect.h"
#include "effects/GrConstColorProcessor.h"
#include "effects/GrDitherEffect.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"
#include "effects/GrYUVEffect.h"

#ifndef SK_IGNORE_ETC1_SUPPORT
#  include "ktx.h"
#  include "etc1.h"
#endif

GrSurfaceDesc GrImageInfoToSurfaceDesc(const SkImageInfo& info, const GrCaps& caps) {
    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fWidth = info.width();
    desc.fHeight = info.height();
    desc.fConfig = SkImageInfo2GrPixelConfig(info, caps);
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
static void build_index8_data(void* buffer, const SkPixmap& pixmap) {
    SkASSERT(kIndex_8_SkColorType == pixmap.colorType());

    const SkColorTable* ctable = pixmap.ctable();
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

    if ((unsigned)pixmap.width() == pixmap.rowBytes()) {
        memcpy(dst, pixmap.addr(), pixmap.getSafeSize());
    } else {
        // need to trim off the extra bytes per row
        size_t width = pixmap.width();
        size_t rowBytes = pixmap.rowBytes();
        const uint8_t* src = pixmap.addr8();
        for (int y = 0; y < pixmap.height(); y++) {
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
    bool onQueryYUV8(SkYUVSizeInfo* sizeInfo, SkYUVColorSpace* colorSpace) const override {
        return fPR->queryYUV8(sizeInfo, colorSpace);
    }
    bool onGetYUV8Planes(const SkYUVSizeInfo& sizeInfo, void* planes[3]) override {
        return fPR->getYUV8Planes(sizeInfo, planes);
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

    return ctx->textureProvider()->createTexture(desc, SkBudgeted::kYes, startOfTexData, 0);
}

GrTexture* GrUploadBitmapToTexture(GrContext* ctx, const SkBitmap& bitmap) {
    GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(bitmap.info(), *ctx->caps());
    if (GrTexture *texture = load_etc1_texture(ctx, bitmap, desc)) {
        return texture;
    }

    if (GrTexture* texture = create_texture_from_yuv(ctx, bitmap, desc)) {
        return texture;
    }

    SkAutoLockPixels alp(bitmap);
    if (!bitmap.readyToDraw()) {
        return nullptr;
    }
    SkPixmap pixmap;
    if (!bitmap.peekPixels(&pixmap)) {
        return nullptr;
    }
    return GrUploadPixmapToTexture(ctx, pixmap, SkBudgeted::kYes);
}

GrTexture* GrUploadPixmapToTexture(GrContext* ctx, const SkPixmap& pixmap, SkBudgeted budgeted) {
    const SkPixmap* pmap = &pixmap;
    SkPixmap tmpPixmap;
    SkBitmap tmpBitmap;

    const GrCaps* caps = ctx->caps();
    GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(pixmap.info(), *caps);

    if (caps->srgbSupport() && !GrPixelConfigIsSRGB(desc.fConfig) &&
        kSRGB_SkColorProfileType == pixmap.info().profileType()) {
        // We were supplied sRGB as the profile type, but we don't have a suitable pixel config.
        // Convert to 8888 sRGB so we can handle the data correctly. The raster backend doesn't
        // handle sRGB Index8 -> sRGB 8888 correctly (yet), so lie about both the source and
        // destination (claim they're linear):
        SkImageInfo linSrcInfo = SkImageInfo::Make(pixmap.width(), pixmap.height(),
                                                   pixmap.colorType(), pixmap.alphaType());
        SkPixmap linSrcPixmap(linSrcInfo, pixmap.addr(), pixmap.rowBytes(), pixmap.ctable());

        SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(pixmap.width(), pixmap.height(),
                                                         kSRGB_SkColorProfileType);
        tmpBitmap.allocPixels(dstInfo);

        SkImageInfo linDstInfo = SkImageInfo::MakeN32Premul(pixmap.width(), pixmap.height());
        if (!linSrcPixmap.readPixels(linDstInfo, tmpBitmap.getPixels(), tmpBitmap.rowBytes())) {
            return nullptr;
        }
        if (!tmpBitmap.peekPixels(&tmpPixmap)) {
            return nullptr;
        }
        pmap = &tmpPixmap;
        // must rebuild desc, since we've forced the info to be N32
        desc = GrImageInfoToSurfaceDesc(pmap->info(), *caps);
    } else if (kGray_8_SkColorType == pixmap.colorType()) {
        // We don't have Gray8 support as a pixel config, so expand to 8888

        // We should have converted sRGB Gray8 above (if we have sRGB support):
        SkASSERT(!caps->srgbSupport() || kLinear_SkColorProfileType == pixmap.info().profileType());

        SkImageInfo info = SkImageInfo::MakeN32(pixmap.width(), pixmap.height(),
                                                kOpaque_SkAlphaType);
        tmpBitmap.allocPixels(info);
        if (!pixmap.readPixels(info, tmpBitmap.getPixels(), tmpBitmap.rowBytes())) {
            return nullptr;
        }
        if (!tmpBitmap.peekPixels(&tmpPixmap)) {
            return nullptr;
        }
        pmap = &tmpPixmap;
        // must rebuild desc, since we've forced the info to be N32
        desc = GrImageInfoToSurfaceDesc(pmap->info(), *caps);
    } else if (kIndex_8_SkColorType == pixmap.colorType()) {
        if (caps->isConfigTexturable(kIndex_8_GrPixelConfig)) {
            size_t imageSize = GrCompressedFormatDataSize(kIndex_8_GrPixelConfig,
                                                          pixmap.width(), pixmap.height());
            SkAutoMalloc storage(imageSize);
            build_index8_data(storage.get(), pixmap);

            // our compressed data will be trimmed, so pass width() for its
            // "rowBytes", since they are the same now.
            return ctx->textureProvider()->createTexture(desc, budgeted, storage.get(),
                                                         pixmap.width());
        } else {
            SkImageInfo info = SkImageInfo::MakeN32Premul(pixmap.width(), pixmap.height());
            tmpBitmap.allocPixels(info);
            if (!pixmap.readPixels(info, tmpBitmap.getPixels(), tmpBitmap.rowBytes())) {
                return nullptr;
            }
            if (!tmpBitmap.peekPixels(&tmpPixmap)) {
                return nullptr;
            }
            pmap = &tmpPixmap;
            // must rebuild desc, since we've forced the info to be N32
            desc = GrImageInfoToSurfaceDesc(pmap->info(), *caps);
        }
    }

    return ctx->textureProvider()->createTexture(desc, budgeted, pmap->addr(),
                                                 pmap->rowBytes());
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

GrTexture* GrGenerateMipMapsAndUploadToTexture(GrContext* ctx, const SkBitmap& bitmap)
{
    GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(bitmap.info(), *ctx->caps());
    if (kIndex_8_SkColorType != bitmap.colorType() && !bitmap.readyToDraw()) {
        GrTexture* texture = load_etc1_texture(ctx, bitmap, desc);
        if (texture) {
            return texture;
        }
    }

    GrTexture* texture = create_texture_from_yuv(ctx, bitmap, desc);
    if (texture) {
        return texture;
    }

    // SkMipMap::Build doesn't handle sRGB data correctly (yet).
    // Failover to the GL code-path for now.
    if (kLinear_SkColorProfileType != bitmap.profileType()) {
        return nullptr;
    }

    SkASSERT(sizeof(int) <= sizeof(uint32_t));
    if (bitmap.width() < 0 || bitmap.height() < 0) {
        return nullptr;
    }

    SkAutoPixmapUnlock srcUnlocker;
    if (!bitmap.requestLock(&srcUnlocker)) {
        return nullptr;
    }
    const SkPixmap& pixmap = srcUnlocker.pixmap();
    // Try to catch where we might have returned nullptr for src crbug.com/492818
    if (nullptr == pixmap.addr()) {
        sk_throw();
    }

    SkAutoTDelete<SkMipMap> mipmaps(SkMipMap::Build(pixmap, nullptr));
    if (!mipmaps) {
        return nullptr;
    }

    const int mipLevelCount = mipmaps->countLevels() + 1;
    if (mipLevelCount < 1) {
        return nullptr;
    }

    const bool isMipMapped = mipLevelCount > 1;
    desc.fIsMipMapped = isMipMapped;

    SkAutoTDeleteArray<GrMipLevel> texels(new GrMipLevel[mipLevelCount]);

    texels[0].fPixels = pixmap.addr();
    texels[0].fRowBytes = pixmap.rowBytes();

    for (int i = 1; i < mipLevelCount; ++i) {
        SkMipMap::Level generatedMipLevel;
        mipmaps->getLevel(i - 1, &generatedMipLevel);
        texels[i].fPixels = generatedMipLevel.fPixmap.addr();
        texels[i].fRowBytes = generatedMipLevel.fPixmap.rowBytes();
    }

    return ctx->textureProvider()->createMipMappedTexture(desc, SkBudgeted::kYes, texels.get(),
                                                          mipLevelCount);
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
GrPixelConfig SkImageInfo2GrPixelConfig(SkColorType ct, SkAlphaType, SkColorProfileType pt,
                                        const GrCaps& caps) {
    // We intentionally ignore profile type for non-8888 formats. Anything we can't support
    // in hardware will be expanded to sRGB 8888 in GrUploadPixmapToTexture.
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
            return (kSRGB_SkColorProfileType == pt && caps.srgbSupport())
                   ? kSRGBA_8888_GrPixelConfig : kRGBA_8888_GrPixelConfig;
        case kBGRA_8888_SkColorType:
            return (kSRGB_SkColorProfileType == pt && caps.srgbSupport())
                   ? kSBGRA_8888_GrPixelConfig : kBGRA_8888_GrPixelConfig;
        case kIndex_8_SkColorType:
            return kIndex_8_GrPixelConfig;
        case kGray_8_SkColorType:
            return kAlpha_8_GrPixelConfig; // TODO: gray8 support on gpu
        case kRGBA_F16_SkColorType:
            return kRGBA_half_GrPixelConfig;
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
        case kSBGRA_8888_GrPixelConfig:
            ct = kBGRA_8888_SkColorType;
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
                                           bool allowSRGBInputs,
                                           GrPaint* grPaint) {
    grPaint->setAntiAlias(skPaint.isAntiAlias());
    grPaint->setAllowSRGBInputs(allowSRGBInputs);

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

    // When the xfermode is null on the SkPaint (meaning kSrcOver) we need the XPFactory field on
    // the GrPaint to also be null (also kSrcOver).
    SkASSERT(!grPaint->getXPFactory());
    SkXfermode* xfermode = skPaint.getXfermode();
    if (xfermode) {
        // SafeUnref in case a new xfermode is added that returns null.
        // In such cases we will fall back to kSrcOver_Mode.
        SkSafeUnref(grPaint->setXPFactory(xfermode->asXPFactory()));
    }

#ifndef SK_IGNORE_GPU_DITHER
    if (skPaint.isDither() && grPaint->numColorFragmentProcessors() > 0) {
        grPaint->addColorFragmentProcessor(GrDitherEffect::Create())->unref();
    }
#endif
    return true;
}

bool SkPaintToGrPaint(GrContext* context, const SkPaint& skPaint, const SkMatrix& viewM,
                      bool allowSRGBInputs, GrPaint* grPaint) {
    return skpaint_to_grpaint_impl(context, skPaint, viewM, nullptr, nullptr, false,
                                   allowSRGBInputs, grPaint);
}

/** Replaces the SkShader (if any) on skPaint with the passed in GrFragmentProcessor. */
bool SkPaintToGrPaintReplaceShader(GrContext* context,
                                   const SkPaint& skPaint,
                                   const GrFragmentProcessor* shaderFP,
                                   bool allowSRGBInputs,
                                   GrPaint* grPaint) {
    if (!shaderFP) {
        return false;
    }
    return skpaint_to_grpaint_impl(context, skPaint, SkMatrix::I(), &shaderFP, nullptr, false,
                                   allowSRGBInputs, grPaint);
}

/** Ignores the SkShader (if any) on skPaint. */
bool SkPaintToGrPaintNoShader(GrContext* context,
                              const SkPaint& skPaint,
                              bool allowSRGBInputs,
                              GrPaint* grPaint) {
    // Use a ptr to a nullptr to to indicate that the SkShader is ignored and not replaced.
    static const GrFragmentProcessor* kNullShaderFP = nullptr;
    static const GrFragmentProcessor** kIgnoreShader = &kNullShaderFP;
    return skpaint_to_grpaint_impl(context, skPaint, SkMatrix::I(), kIgnoreShader, nullptr, false,
                                   allowSRGBInputs, grPaint);
}

/** Blends the SkPaint's shader (or color if no shader) with a per-primitive color which must
be setup as a vertex attribute using the specified SkXfermode::Mode. */
bool SkPaintToGrPaintWithXfermode(GrContext* context,
                                  const SkPaint& skPaint,
                                  const SkMatrix& viewM,
                                  SkXfermode::Mode primColorMode,
                                  bool primitiveIsSrc,
                                  bool allowSRGBInputs,
                                  GrPaint* grPaint) {
    return skpaint_to_grpaint_impl(context, skPaint, viewM, nullptr, &primColorMode, primitiveIsSrc,
                                   allowSRGBInputs, grPaint);
}

bool SkPaintToGrPaintWithTexture(GrContext* context,
                                 const SkPaint& paint,
                                 const SkMatrix& viewM,
                                 const GrFragmentProcessor* fp,
                                 bool textureIsAlphaOnly,
                                 bool allowSRGBInputs,
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

    return SkPaintToGrPaintReplaceShader(context, paint, shaderFP.get(), allowSRGBInputs, grPaint);
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
