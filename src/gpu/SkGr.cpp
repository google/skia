/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGr.h"

#include "GrCaps.h"
#include "GrDrawContext.h"
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

/*  Fill out buffer with the compressed format Ganesh expects from a colortable
 based bitmap. [palette (colortable) + indices].

 At the moment Ganesh only supports 8bit version. If Ganesh allowed we others
 we could detect that the colortable.count is <= 16, and then repack the
 indices as nibbles to save RAM, but it would take more time (i.e. a lot
 slower than memcpy), so skipping that for now.

 Ganesh wants a full 256 palette entry, even though Skia's ctable is only as big
 as the colortable.count says it is.
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

////////////////////////////////////////////////////////////////////////////////

struct Stretch {
    enum Type {
        kNone_Type,
        kBilerp_Type,
        kNearest_Type
    } fType;
    int fWidth;
    int fHeight;
};

static void get_stretch(const GrContext* ctx, int width, int height,
                        const GrTextureParams* params, Stretch* stretch) {
    stretch->fType = Stretch::kNone_Type;
    bool doStretch = false;
    if (params && params->isTiled() && !ctx->caps()->npotTextureTileSupport() &&
        (!SkIsPow2(width) || !SkIsPow2(height))) {
        doStretch = true;
        stretch->fWidth  = GrNextPow2(SkTMax(width, ctx->caps()->minTextureSize()));
        stretch->fHeight = GrNextPow2(SkTMax(height, ctx->caps()->minTextureSize()));
    } else if (width < ctx->caps()->minTextureSize() || height < ctx->caps()->minTextureSize()) {
        // The small texture issues appear to be with tiling. Hence it seems ok to scale them
        // up using the GPU. If issues persist we may need to CPU-stretch.
        doStretch = true;
        stretch->fWidth = SkTMax(width, ctx->caps()->minTextureSize());
        stretch->fHeight = SkTMax(height, ctx->caps()->minTextureSize());
    }
    if (doStretch) {
        if (params) {
            switch(params->filterMode()) {
                case GrTextureParams::kNone_FilterMode:
                    stretch->fType = Stretch::kNearest_Type;
                    break;
                case GrTextureParams::kBilerp_FilterMode:
                case GrTextureParams::kMipMap_FilterMode:
                    stretch->fType = Stretch::kBilerp_Type;
                    break;
            }
        } else {
            stretch->fType = Stretch::kBilerp_Type;
        }
    } else {
        stretch->fWidth  = -1;
        stretch->fHeight = -1;
        stretch->fType = Stretch::kNone_Type;
    }
}

static bool make_stretched_key(const GrUniqueKey& origKey, const Stretch& stretch,
                               GrUniqueKey* stretchedKey) {
    if (origKey.isValid() && Stretch::kNone_Type != stretch.fType) {
        uint32_t width = SkToU16(stretch.fWidth);
        uint32_t height = SkToU16(stretch.fHeight);
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(stretchedKey, origKey, kDomain, 2);
        builder[0] = stretch.fType;
        builder[1] = width | (height << 16);
        builder.finish();
        return true;
    }
    SkASSERT(!stretchedKey->isValid());
    return false;
}

static void make_unstretched_key(GrUniqueKey* key, uint32_t imageID, const SkIRect& subset) {
    SkASSERT(SkIsU16(subset.width()));
    SkASSERT(SkIsU16(subset.height()));

    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(key, kDomain, 4);
    builder[0] = imageID;
    builder[1] = subset.x();
    builder[2] = subset.y();
    builder[3] = subset.width() | (subset.height() << 16);
}

void GrMakeKeyFromImageID(GrUniqueKey* key, uint32_t imageID, const SkIRect& subset,
                          const GrCaps& caps, SkImageUsageType usage) {
    const Stretch::Type stretches[] = {
        Stretch::kNone_Type,        // kUntiled_SkImageUsageType
        Stretch::kNearest_Type,     // kTiled_Unfiltered_SkImageUsageType
        Stretch::kBilerp_Type,      // kTiled_Filtered_SkImageUsageType
    };

    const bool isPow2 = SkIsPow2(subset.width()) && SkIsPow2(subset.height());
    const bool needToStretch = !isPow2 &&
                               usage != kUntiled_SkImageUsageType &&
                               !caps.npotTextureTileSupport();

    if (needToStretch) {
        GrUniqueKey tmpKey;
        make_unstretched_key(&tmpKey, imageID, subset);

        Stretch stretch;
        stretch.fType = stretches[usage];
        stretch.fWidth = SkNextPow2(subset.width());
        stretch.fHeight = SkNextPow2(subset.height());
        if (!make_stretched_key(tmpKey, stretch, key)) {
            goto UNSTRETCHED;
        }
    } else {
        UNSTRETCHED:
        make_unstretched_key(key, imageID, subset);
    }
}

static void make_image_keys(uint32_t imageID, const SkIRect& subset, const Stretch& stretch,
                            GrUniqueKey* key, GrUniqueKey* stretchedKey) {
    make_unstretched_key(key, imageID, subset);
    if (Stretch::kNone_Type != stretch.fType) {
        make_stretched_key(*key, stretch, stretchedKey);
    }
}

GrSurfaceDesc GrImageInfoToSurfaceDesc(const SkImageInfo& info) {
    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fWidth = info.width();
    desc.fHeight = info.height();
    desc.fConfig = SkImageInfo2GrPixelConfig(info);
    desc.fSampleCnt = 0;
    return desc;
}

namespace {

// When the SkPixelRef genID changes, invalidate a corresponding GrResource described by key.
class BitmapInvalidator : public SkPixelRef::GenIDChangeListener {
public:
    explicit BitmapInvalidator(const GrUniqueKey& key) : fMsg(key) {}
private:
    GrUniqueKeyInvalidatedMessage fMsg;

    void onChange() override {
        SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(fMsg);
    }
};

}  // namespace


GrTexture* GrCreateTextureForPixels(GrContext* ctx,
                                    const GrUniqueKey& optionalKey,
                                    GrSurfaceDesc desc,
                                    SkPixelRef* pixelRefForInvalidationNotification,
                                    const void* pixels,
                                    size_t rowBytes) {
    GrTexture* result = ctx->textureProvider()->createTexture(desc, true, pixels, rowBytes);
    if (result && optionalKey.isValid()) {
        if (pixelRefForInvalidationNotification) {
            BitmapInvalidator* listener = new BitmapInvalidator(optionalKey);
            pixelRefForInvalidationNotification->addGenIDChangeListener(listener);
        }
        ctx->textureProvider()->assignUniqueKeyToTexture(optionalKey, result);
    }
    return result;
}

// creates a new texture that is the input texture scaled up. If optionalKey is valid it will be
// set on the new texture. stretch controls whether the scaling is done using nearest or bilerp
// filtering and the size to stretch the texture to.
GrTexture* stretch_texture(GrTexture* inputTexture, const Stretch& stretch,
                           SkPixelRef* pixelRef,
                           const GrUniqueKey& optionalKey) {
    SkASSERT(Stretch::kNone_Type != stretch.fType);

    GrContext* context = inputTexture->getContext();
    SkASSERT(context);
    const GrCaps* caps = context->caps();

    // Either it's a cache miss or the original wasn't cached to begin with.
    GrSurfaceDesc rtDesc = inputTexture->desc();
    rtDesc.fFlags =  rtDesc.fFlags | kRenderTarget_GrSurfaceFlag;
    rtDesc.fWidth  = stretch.fWidth;
    rtDesc.fHeight = stretch.fHeight;
    rtDesc.fConfig = GrMakePixelConfigUncompressed(rtDesc.fConfig);

    // If the config isn't renderable try converting to either A8 or an 32 bit config. Otherwise,
    // fail.
    if (!caps->isConfigRenderable(rtDesc.fConfig, false)) {
        if (GrPixelConfigIsAlphaOnly(rtDesc.fConfig)) {
            if (caps->isConfigRenderable(kAlpha_8_GrPixelConfig, false)) {
                rtDesc.fConfig = kAlpha_8_GrPixelConfig;
            } else if (caps->isConfigRenderable(kSkia8888_GrPixelConfig, false)) {
                rtDesc.fConfig = kSkia8888_GrPixelConfig;
            } else {
                return nullptr;
            }
        } else if (kRGB_GrColorComponentFlags ==
                   (kRGB_GrColorComponentFlags & GrPixelConfigComponentMask(rtDesc.fConfig))) {
            if (caps->isConfigRenderable(kSkia8888_GrPixelConfig, false)) {
                rtDesc.fConfig = kSkia8888_GrPixelConfig;
            } else {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }

    SkAutoTUnref<GrTexture> stretched(GrCreateTextureForPixels(context, optionalKey, rtDesc,
                                                               pixelRef, nullptr,0));
    if (!stretched) {
        return nullptr;
    }
    GrPaint paint;

    // If filtering is not desired then we want to ensure all texels in the resampled image are
    // copies of texels from the original.
    GrTextureParams params(SkShader::kClamp_TileMode,
                           Stretch::kBilerp_Type == stretch.fType ?
                              GrTextureParams::kBilerp_FilterMode :
                              GrTextureParams::kNone_FilterMode);
    paint.addColorTextureProcessor(inputTexture, SkMatrix::I(), params);

    SkRect rect = SkRect::MakeWH(SkIntToScalar(rtDesc.fWidth), SkIntToScalar(rtDesc.fHeight));
    SkRect localRect = SkRect::MakeWH(1.f, 1.f);

    SkAutoTUnref<GrDrawContext> drawContext(context->drawContext());
    if (!drawContext) {
        return nullptr;
    }

    drawContext->drawNonAARectToRect(stretched->asRenderTarget(), GrClip::WideOpen(), paint,
                                     SkMatrix::I(), rect, localRect);

    return stretched.detach();
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

static GrTexture* load_etc1_texture(GrContext* ctx, const GrUniqueKey& optionalKey,
                                    const SkBitmap &bm, GrSurfaceDesc desc) {
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

    return GrCreateTextureForPixels(ctx, optionalKey, desc, bm.pixelRef(), startOfTexData, 0);
}

/*
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

static GrTexture* load_yuv_texture(GrContext* ctx, const GrUniqueKey& optionalKey,
                                   const SkBitmap& bm, const GrSurfaceDesc& desc) {
    // Subsets are not supported, the whole pixelRef is loaded when using YUV decoding
    SkPixelRef* pixelRef = bm.pixelRef();
    if ((nullptr == pixelRef) ||
        (pixelRef->info().width()  != bm.info().width()) ||
        (pixelRef->info().height() != bm.info().height())) {
        return nullptr;
    }

    const bool useCache = optionalKey.isValid();
    PixelRef_GrYUVProvider provider(pixelRef);
    GrTexture* texture = provider.refAsTexture(ctx, desc, useCache);
    if (!texture) {
        return nullptr;
    }

    if (useCache) {
        BitmapInvalidator* listener = new BitmapInvalidator(optionalKey);
        pixelRef->addGenIDChangeListener(listener);
        ctx->textureProvider()->assignUniqueKeyToTexture(optionalKey, texture);
    }
    return texture;
}

static GrTexture* create_unstretched_bitmap_texture(GrContext* ctx,
                                                    const SkBitmap& origBitmap,
                                                    const GrUniqueKey& optionalKey) {
    if (origBitmap.width() < ctx->caps()->minTextureSize() ||
        origBitmap.height() < ctx->caps()->minTextureSize()) {
        return nullptr;
    }
    SkBitmap tmpBitmap;

    const SkBitmap* bitmap = &origBitmap;

    GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(bitmap->info());
    const GrCaps* caps = ctx->caps();

    if (kIndex_8_SkColorType == bitmap->colorType()) {
        if (caps->isConfigTexturable(kIndex_8_GrPixelConfig)) {
            size_t imageSize = GrCompressedFormatDataSize(kIndex_8_GrPixelConfig,
                                                          bitmap->width(), bitmap->height());
            SkAutoMalloc storage(imageSize);
            build_index8_data(storage.get(), origBitmap);

            // our compressed data will be trimmed, so pass width() for its
            // "rowBytes", since they are the same now.
            return GrCreateTextureForPixels(ctx, optionalKey, desc, origBitmap.pixelRef(),
                                            storage.get(), bitmap->width());
        } else {
            origBitmap.copyTo(&tmpBitmap, kN32_SkColorType);
            // now bitmap points to our temp, which has been promoted to 32bits
            bitmap = &tmpBitmap;
            desc.fConfig = SkImageInfo2GrPixelConfig(bitmap->info());
        }
    } else if (!bitmap->readyToDraw()) {
        // If the bitmap had compressed data and was then uncompressed, it'll still return
        // compressed data on 'refEncodedData' and upload it. Probably not good, since if
        // the bitmap has available pixels, then they might not be what the decompressed
        // data is.
        GrTexture *texture = load_etc1_texture(ctx, optionalKey, *bitmap, desc);
        if (texture) {
            return texture;
        }
    }

    GrTexture *texture = load_yuv_texture(ctx, optionalKey, *bitmap, desc);
    if (texture) {
        return texture;
    }

    SkAutoLockPixels alp(*bitmap);
    if (!bitmap->readyToDraw()) {
        return nullptr;
    }

    return GrCreateTextureForPixels(ctx, optionalKey, desc, origBitmap.pixelRef(),
                                    bitmap->getPixels(), bitmap->rowBytes());
}

static SkBitmap stretch_on_cpu(const SkBitmap& bmp, const Stretch& stretch) {
    SkBitmap stretched;
    stretched.allocN32Pixels(stretch.fWidth, stretch.fHeight);
    SkCanvas canvas(stretched);
    SkPaint paint;
    switch (stretch.fType) {
        case Stretch::kNearest_Type:
            paint.setFilterQuality(kNone_SkFilterQuality);
            break;
        case Stretch::kBilerp_Type:
            paint.setFilterQuality(kLow_SkFilterQuality);
            break;
        case Stretch::kNone_Type:
            SkDEBUGFAIL("Shouldn't get here.");
            break;
    }
    SkRect dstRect = SkRect::MakeWH(SkIntToScalar(stretch.fWidth), SkIntToScalar(stretch.fHeight));
    canvas.drawBitmapRect(bmp, dstRect, &paint);
    return stretched;
}

static GrTexture* create_bitmap_texture(GrContext* ctx,
                                        const SkBitmap& bmp,
                                        const Stretch& stretch,
                                        const GrUniqueKey& unstretchedKey,
                                        const GrUniqueKey& stretchedKey) {
    if (Stretch::kNone_Type != stretch.fType) {
        SkAutoTUnref<GrTexture> unstretched;
        // Check if we have the unstretched version in the cache, if not create it.
        if (unstretchedKey.isValid()) {
            unstretched.reset(ctx->textureProvider()->findAndRefTextureByUniqueKey(unstretchedKey));
        }
        if (!unstretched) {
            unstretched.reset(create_unstretched_bitmap_texture(ctx, bmp, unstretchedKey));
            if (!unstretched) {
                // We might not have been able to create a unstrecthed texture because it is smaller
                // than the min texture size. In that case do cpu stretching.
                SkBitmap stretchedBmp = stretch_on_cpu(bmp, stretch);
                return create_unstretched_bitmap_texture(ctx, stretchedBmp, stretchedKey);
            }
        }
        return stretch_texture(unstretched, stretch, bmp.pixelRef(), stretchedKey);
    }
    return create_unstretched_bitmap_texture(ctx, bmp, unstretchedKey);
}

bool GrIsImageInCache(const GrContext* ctx, uint32_t imageID, const SkIRect& subset,
                      GrTexture* nativeTexture, const GrTextureParams* params) {
    Stretch stretch;
    get_stretch(ctx, subset.width(), subset.height(), params, &stretch);

    // Handle the case where the bitmap/image is explicitly texture backed.
    if (nativeTexture) {
        if (Stretch::kNone_Type == stretch.fType) {
            return true;
        }
        const GrUniqueKey& key = nativeTexture->getUniqueKey();
        if (!key.isValid()) {
            return false;
        }
        GrUniqueKey stretchedKey;
        make_stretched_key(key, stretch, &stretchedKey);
        return ctx->textureProvider()->existsTextureWithUniqueKey(stretchedKey);
    }

    GrUniqueKey key, stretchedKey;
    make_image_keys(imageID, subset, stretch, &key, &stretchedKey);
    return ctx->textureProvider()->existsTextureWithUniqueKey(
        (Stretch::kNone_Type == stretch.fType) ? key : stretchedKey);
}

bool GrIsBitmapInCache(const GrContext* ctx, const SkBitmap& bitmap,
                       const GrTextureParams* params) {
    if (bitmap.isVolatile()) {
        return false;   // we don't cache volatile bitmaps.
    }
    return GrIsImageInCache(ctx, bitmap.getGenerationID(), bitmap.getSubset(), bitmap.getTexture(),
                            params);
}

GrTexture* GrRefCachedBitmapTexture(GrContext* ctx,
                                    const SkBitmap& bitmap,
                                    const GrTextureParams* params) {

    Stretch stretch;
    get_stretch(ctx, bitmap.width(), bitmap.height(), params, &stretch);

    GrTexture* result = bitmap.getTexture();
    if (result) {
        if (Stretch::kNone_Type == stretch.fType) {
            return SkRef(result);
        }
        GrUniqueKey stretchedKey;
        // Don't create a key for the resized version if the bmp is volatile.
        if (!bitmap.isVolatile()) {
            const GrUniqueKey& key = result->getUniqueKey();
            if (key.isValid()) {
                make_stretched_key(key, stretch, &stretchedKey);
                GrTexture* stretched =
                    ctx->textureProvider()->findAndRefTextureByUniqueKey(stretchedKey);
                if (stretched) {
                    return stretched;
                }
            }
        }
        return stretch_texture(result, stretch, bitmap.pixelRef(), stretchedKey);
    }

    GrUniqueKey key, resizedKey;

    if (!bitmap.isVolatile()) {
        // If the bitmap isn't changing try to find a cached copy first.
        make_image_keys(bitmap.getGenerationID(), bitmap.getSubset(), stretch, &key, &resizedKey);

        result = ctx->textureProvider()->findAndRefTextureByUniqueKey(
            resizedKey.isValid() ? resizedKey : key);
        if (result) {
            return result;
        }
    }

    result = create_bitmap_texture(ctx, bitmap, stretch, key, resizedKey);
    if (result) {
        return result;
    }

    SkErrorInternals::SetError( kInternalError_SkError,
                                "---- failed to create texture for cache [%d %d]\n",
                                bitmap.width(), bitmap.height());

    return nullptr;
}

// TODO: make this be the canonical signature, and turn the version that takes GrTextureParams*
//       into a wrapper that contains the inverse of these tables.
GrTexture* GrRefCachedBitmapTexture(GrContext* ctx,
                                    const SkBitmap& bitmap,
                                    SkImageUsageType usage) {
    // Just need a params that will trigger the correct cache key / etc, since the usage doesn't
    // tell us the specifics about filter level or specific tiling.

    const SkShader::TileMode tiles[] = {
        SkShader::kClamp_TileMode,      // kUntiled_SkImageUsageType
        SkShader::kRepeat_TileMode,     // kTiled_Unfiltered_SkImageUsageType
        SkShader::kRepeat_TileMode,     // kTiled_Filtered_SkImageUsageType
    };

    const GrTextureParams::FilterMode filters[] = {
        GrTextureParams::kNone_FilterMode,      // kUntiled_SkImageUsageType
        GrTextureParams::kNone_FilterMode,      // kTiled_Unfiltered_SkImageUsageType
        GrTextureParams::kBilerp_FilterMode,    // kTiled_Filtered_SkImageUsageType
    };

    GrTextureParams params(tiles[usage], filters[usage]);
    return GrRefCachedBitmapTexture(ctx, bitmap, &params);
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
    const GrFragmentProcessor* shaderFP = NULL;
    if (shaderProcessor) {
        shaderFP = *shaderProcessor;
    } else if (const SkShader* shader = skPaint.getShader()) {
        aufp.reset(shader->asFragmentProcessor(context, viewM, NULL, skPaint.getFilterQuality(),
                                               grPaint->getProcessorDataManager()));
        shaderFP = aufp;
        if (!shaderFP) {
            return false;
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

            grPaint->setColor(SkColorToUnpremulGrColor(skPaint.getColor()) | 0xFF000000);

            GrColor paintAlpha = SkColorAlphaToGrColor(skPaint.getColor());
            grPaint->addColorFragmentProcessor(GrConstColorProcessor::Create(
                paintAlpha, GrConstColorProcessor::kModulateRGBA_InputMode))->unref();
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
            SkTDArray<const GrFragmentProcessor*> array;
            if (colorFilter->asFragmentProcessors(context, grPaint->getProcessorDataManager(),
                                                  &array)) {
                for (int i = 0; i < array.count(); ++i) {
                    grPaint->addColorFragmentProcessor(array[i])->unref();
                }
            } else {
                return false;
            }
        }
    }

    SkXfermode* mode = skPaint.getXfermode();
    GrXPFactory* xpFactory = nullptr;
    if (!SkXfermode::AsXPFactory(mode, &xpFactory)) {
        // Fall back to src-over
        // return false here?
        xpFactory = GrPorterDuffXPFactory::Create(SkXfermode::kSrcOver_Mode);
    }
    SkASSERT(xpFactory);
    grPaint->setXPFactory(xpFactory)->unref();

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
