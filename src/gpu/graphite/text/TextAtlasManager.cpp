/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/text/TextAtlasManager.h"

#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/base/SkAutoMalloc.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkMasks.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/sksl/SkSLUtil.h"
#include "src/text/gpu/Glyph.h"
#include "src/text/gpu/GlyphVector.h"
#include "src/text/gpu/StrikeCache.h"

using Glyph = sktext::gpu::Glyph;

namespace skgpu::graphite {

TextAtlasManager::TextAtlasManager(Recorder* recorder)
        : fRecorder(recorder)
        , fSupportBilerpAtlas{recorder->priv().caps()->supportBilerpFromGlyphAtlas()}
        , fAtlasConfig{recorder->priv().caps()->maxTextureSize(),
                       recorder->priv().caps()->glyphCacheTextureMaximumBytes()} {
    if (!recorder->priv().caps()->allowMultipleAtlasTextures() ||
        // multitexturing supported only if range can represent the index + texcoords fully
        !(recorder->priv().caps()->shaderCaps()->fFloatIs32Bits ||
          recorder->priv().caps()->shaderCaps()->fIntegerSupport)) {
        fAllowMultitexturing = DrawAtlas::AllowMultitexturing::kNo;
    } else {
        fAllowMultitexturing = DrawAtlas::AllowMultitexturing::kYes;
    }
}

TextAtlasManager::~TextAtlasManager() = default;

void TextAtlasManager::freeAll() {
    for (int i = 0; i < kMaskFormatCount; ++i) {
        fAtlases[i] = nullptr;
    }
}

bool TextAtlasManager::hasGlyph(MaskFormat format, Glyph* glyph) {
    SkASSERT(glyph);
    return this->getAtlas(format)->hasID(glyph->fAtlasLocator.plotLocator());
}

template <typename INT_TYPE>
static void expand_bits(INT_TYPE* dst,
                        const uint8_t* src,
                        int width,
                        int height,
                        int dstRowBytes,
                        int srcRowBytes) {
    for (int y = 0; y < height; ++y) {
        int rowWritesLeft = width;
        const uint8_t* s = src;
        INT_TYPE* d = dst;
        while (rowWritesLeft > 0) {
            unsigned mask = *s++;
            for (int x = 7; x >= 0 && rowWritesLeft; --x, --rowWritesLeft) {
                *d++ = (mask & (1 << x)) ? (INT_TYPE)(~0UL) : 0;
            }
        }
        dst = reinterpret_cast<INT_TYPE*>(reinterpret_cast<intptr_t>(dst) + dstRowBytes);
        src += srcRowBytes;
    }
}

static void get_packed_glyph_image(
        const SkGlyph& glyph, int dstRB, MaskFormat expectedMaskFormat, void* dst) {
    const int width = glyph.width();
    const int height = glyph.height();
    const void* src = glyph.image();
    SkASSERT(src != nullptr);

    MaskFormat maskFormat = Glyph::FormatFromSkGlyph(glyph.maskFormat());
    if (maskFormat == expectedMaskFormat) {
        int srcRB = glyph.rowBytes();
        // Notice this comparison is with the glyphs raw mask format, and not its MaskFormat.
        if (glyph.maskFormat() != SkMask::kBW_Format) {
            if (srcRB != dstRB) {
                const int bbp = MaskFormatBytesPerPixel(expectedMaskFormat);
                for (int y = 0; y < height; y++) {
                    memcpy(dst, src, width * bbp);
                    src = (const char*) src + srcRB;
                    dst = (char*) dst + dstRB;
                }
            } else {
                memcpy(dst, src, dstRB * height);
            }
        } else {
            // Handle 8-bit format by expanding the mask to the expected format.
            const uint8_t* bits = reinterpret_cast<const uint8_t*>(src);
            switch (expectedMaskFormat) {
                case MaskFormat::kA8: {
                    uint8_t* bytes = reinterpret_cast<uint8_t*>(dst);
                    expand_bits(bytes, bits, width, height, dstRB, srcRB);
                    break;
                }
                case MaskFormat::kA565: {
                    uint16_t* rgb565 = reinterpret_cast<uint16_t*>(dst);
                    expand_bits(rgb565, bits, width, height, dstRB, srcRB);
                    break;
                }
                default:
                    SK_ABORT("Invalid MaskFormat");
            }
        }
    } else if (maskFormat == MaskFormat::kA565 &&
               expectedMaskFormat == MaskFormat::kARGB) {
        // Convert if the glyph uses a 565 mask format since it is using LCD text rendering
        // but the expected format is 8888 (will happen on Intel MacOS with Metal since that
        // combination does not support 565).
        static constexpr SkMasks masks{
                {0b1111'1000'0000'0000, 11, 5},  // Red
                {0b0000'0111'1110'0000,  5, 6},  // Green
                {0b0000'0000'0001'1111,  0, 5},  // Blue
                {0, 0, 0}                        // Alpha
        };
        constexpr int a565Bpp = MaskFormatBytesPerPixel(MaskFormat::kA565);
        constexpr int argbBpp = MaskFormatBytesPerPixel(MaskFormat::kARGB);
        char* dstRow = (char*)dst;
        for (int y = 0; y < height; y++) {
            dst = dstRow;
            for (int x = 0; x < width; x++) {
                uint16_t color565 = 0;
                memcpy(&color565, src, a565Bpp);
                // TODO: create Graphite version of GrColorPackRGBA?
                uint32_t colorRGBA = masks.getRed(color565) |
                                     (masks.getGreen(color565) << 8) |
                                     (masks.getBlue(color565) << 16) |
                                     (0xFF << 24);
                memcpy(dst, &colorRGBA, argbBpp);
                src = (const char*)src + a565Bpp;
                dst = (      char*)dst + argbBpp;
            }
            dstRow += dstRB;
        }
    } else {
        SkUNREACHABLE;
    }
}

MaskFormat TextAtlasManager::resolveMaskFormat(MaskFormat format) const {
    if (MaskFormat::kA565 == format &&
        !fRecorder->priv().caps()->getDefaultSampledTextureInfo(kRGB_565_SkColorType,
                                                                /*mipmapped=*/Mipmapped::kNo,
                                                                Protected::kNo,
                                                                Renderable::kNo).isValid()) {
        format = MaskFormat::kARGB;
    }
    return format;
}

// Returns kSucceeded if glyph successfully added to texture atlas, kTryAgain if a RenderPassTask
// needs to be snapped before adding the glyph, and kError if it can't be added at all.
DrawAtlas::ErrorCode TextAtlasManager::addGlyphToAtlas(const SkGlyph& skGlyph,
                                                       Glyph* glyph,
                                                       int srcPadding) {
#if !defined(SK_DISABLE_SDF_TEXT)
    SkASSERT(0 <= srcPadding && srcPadding <= SK_DistanceFieldInset);
#else
    SkASSERT(0 <= srcPadding);
#endif

    if (skGlyph.image() == nullptr) {
        return DrawAtlas::ErrorCode::kError;
    }
    SkASSERT(glyph != nullptr);

    MaskFormat glyphFormat = Glyph::FormatFromSkGlyph(skGlyph.maskFormat());
    MaskFormat expectedMaskFormat = this->resolveMaskFormat(glyphFormat);
    int bytesPerPixel = MaskFormatBytesPerPixel(expectedMaskFormat);

    int padding;
    switch (srcPadding) {
        case 0:
            // The direct mask/image case.
            padding = 0;
            if (fSupportBilerpAtlas) {
                // Force direct masks (glyph with no padding) to have padding.
                padding = 1;
                srcPadding = 1;
            }
            break;
        case 1:
            // The transformed mask/image case.
            padding = 1;
            break;
#if !defined(SK_DISABLE_SDF_TEXT)
        case SK_DistanceFieldInset:
            // The SDFT case.
            // If the srcPadding == SK_DistanceFieldInset (SDFT case) then the padding is built
            // into the image on the glyph; no extra padding needed.
            // TODO: can the SDFT glyph image in the cache be reduced by the padding?
            padding = 0;
            break;
#endif
        default:
            // The padding is not one of the know forms.
            return DrawAtlas::ErrorCode::kError;
    }

    const int width = skGlyph.width() + 2*padding;
    const int height = skGlyph.height() + 2*padding;
    int rowBytes = width * bytesPerPixel;
    size_t size = height * rowBytes;

    // Temporary storage for normalizing glyph image.
    SkAutoSMalloc<1024> storage(size);
    void* dataPtr = storage.get();
    if (padding > 0) {
        sk_bzero(dataPtr, size);
        // Advance in one row and one column.
        dataPtr = (char*)(dataPtr) + rowBytes + bytesPerPixel;
    }

    get_packed_glyph_image(skGlyph, rowBytes, expectedMaskFormat, dataPtr);

    DrawAtlas* atlas = this->getAtlas(expectedMaskFormat);
    auto errorCode = atlas->addToAtlas(fRecorder,
                                       width,
                                       height,
                                       storage.get(),
                                       &glyph->fAtlasLocator);

    if (errorCode == DrawAtlas::ErrorCode::kSucceeded) {
        glyph->fAtlasLocator.insetSrc(srcPadding);
    }

    return errorCode;
}

bool TextAtlasManager::recordUploads(DrawContext* dc) {
    for (int i = 0; i < skgpu::kMaskFormatCount; i++) {
        if (fAtlases[i] && !fAtlases[i]->recordUploads(dc, fRecorder)) {
            return false;
        }
    }

    return true;
}

void TextAtlasManager::addGlyphToBulkAndSetUseToken(BulkUsePlotUpdater* updater,
                                                    MaskFormat format,
                                                    Glyph* glyph,
                                                    AtlasToken token) {
    SkASSERT(glyph);
    if (updater->add(glyph->fAtlasLocator)) {
        this->getAtlas(format)->setLastUseToken(glyph->fAtlasLocator, token);
    }
}

void TextAtlasManager::setAtlasDimensionsToMinimum_ForTesting() {
    // Delete any old atlases.
    // This should be safe to do as long as we are not in the middle of a flush.
    for (int i = 0; i < skgpu::kMaskFormatCount; i++) {
        fAtlases[i] = nullptr;
    }

    // Set all the atlas sizes to 1x1 plot each.
    new (&fAtlasConfig) DrawAtlasConfig{2048, 0};
}

bool TextAtlasManager::initAtlas(MaskFormat format) {
    int index = MaskFormatToAtlasIndex(format);
    if (fAtlases[index] == nullptr) {
        SkColorType colorType = MaskFormatToColorType(format);
        SkISize atlasDimensions = fAtlasConfig.atlasDimensions(format);
        SkISize plotDimensions = fAtlasConfig.plotDimensions(format);
        fAtlases[index] = DrawAtlas::Make(colorType,
                                          SkColorTypeBytesPerPixel(colorType),
                                          atlasDimensions.width(), atlasDimensions.height(),
                                          plotDimensions.width(), plotDimensions.height(),
                                          this,
                                          fAllowMultitexturing,
                                          nullptr,
                                          /*label=*/"TextAtlas");
        if (!fAtlases[index]) {
            return false;
        }
    }
    return true;
}

void TextAtlasManager::postFlush() {
    auto tokenTracker = fRecorder->priv().tokenTracker();
    for (int i = 0; i < kMaskFormatCount; ++i) {
        if (fAtlases[i]) {
            fAtlases[i]->compact(tokenTracker->nextFlushToken());
        }
    }
}

}  // namespace skgpu::graphite

////////////////////////////////////////////////////////////////////////////////////////////////

namespace sktext::gpu {

using DrawAtlas = skgpu::graphite::DrawAtlas;

std::tuple<bool, int> GlyphVector::regenerateAtlasForGraphite(int begin,
                                                              int end,
                                                              skgpu::MaskFormat maskFormat,
                                                              int srcPadding,
                                                              skgpu::graphite::Recorder* recorder) {
    auto atlasManager = recorder->priv().atlasProvider()->textAtlasManager();
    auto tokenTracker = recorder->priv().tokenTracker();

    // TODO: this is not a great place for this -- need a better way to init atlases when needed
    unsigned int numActiveProxies;
    const sk_sp<skgpu::graphite::TextureProxy>* proxies =
            atlasManager->getProxies(maskFormat, &numActiveProxies);
    if (!proxies) {
        SkDebugf("Could not allocate backing texture for atlas\n");
        return {false, 0};
    }

    uint64_t currentAtlasGen = atlasManager->atlasGeneration(maskFormat);

    this->packedGlyphIDToGlyph(recorder->priv().strikeCache());

    if (fAtlasGeneration != currentAtlasGen) {
        // Calculate the texture coordinates for the vertexes during first use (fAtlasGeneration
        // is set to kInvalidAtlasGeneration) or the atlas has changed in subsequent calls..
        fBulkUseUpdater.reset();

        SkBulkGlyphMetricsAndImages metricsAndImages{fTextStrike->strikeSpec()};

        // Update the atlas information in the GrStrike.
        auto glyphs = fGlyphs.subspan(begin, end - begin);
        int glyphsPlacedInAtlas = 0;
        bool success = true;
        for (const Variant& variant : glyphs) {
            Glyph* gpuGlyph = variant.glyph;
            SkASSERT(gpuGlyph != nullptr);

            if (!atlasManager->hasGlyph(maskFormat, gpuGlyph)) {
                const SkGlyph& skGlyph = *metricsAndImages.glyph(gpuGlyph->fPackedID);
                auto code = atlasManager->addGlyphToAtlas(skGlyph, gpuGlyph, srcPadding);
                if (code != DrawAtlas::ErrorCode::kSucceeded) {
                    success = code != DrawAtlas::ErrorCode::kError;
                    break;
                }
            }
            atlasManager->addGlyphToBulkAndSetUseToken(
                    &fBulkUseUpdater, maskFormat, gpuGlyph,
                    tokenTracker->nextFlushToken());
            glyphsPlacedInAtlas++;
        }

        // Update atlas generation if there are no more glyphs to put in the atlas.
        if (success && begin + glyphsPlacedInAtlas == SkCount(fGlyphs)) {
            // Need to get the freshest value of the atlas' generation because
            // updateTextureCoordinates may have changed it.
            fAtlasGeneration = atlasManager->atlasGeneration(maskFormat);
        }

        return {success, glyphsPlacedInAtlas};
    } else {
        // The atlas hasn't changed, so our texture coordinates are still valid.
        if (end == SkCount(fGlyphs)) {
            // The atlas hasn't changed and the texture coordinates are all still valid. Update
            // all the plots used to the new use token.
            atlasManager->setUseTokenBulk(fBulkUseUpdater,
                                          tokenTracker->nextFlushToken(),
                                          maskFormat);
        }
        return {true, end - begin};
    }
}

}  // namespace sktext::gpu
