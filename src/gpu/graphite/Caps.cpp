/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/Caps.h"

#include "include/core/SkCapabilities.h"
#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkTo.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/sksl/SkSLUtil.h"

#include <algorithm>

namespace skgpu::graphite {

Caps::Caps()
        : fShaderCaps(std::make_unique<SkSL::ShaderCaps>())
        , fCapabilities(new SkCapabilities()) {}

Caps::~Caps() {}

void Caps::finishInitialization(const ContextOptions& options) {
    fCapabilities->initSkCaps(fShaderCaps.get());

    fMaxInternalSampleCount = options.fInternalMultisampleCount;

    if (options.fShaderErrorHandler) {
        fShaderErrorHandler = options.fShaderErrorHandler;
    } else {
        fShaderErrorHandler = DefaultShaderErrorHandler();
    }

#if defined(GPU_TEST_UTILS)
    if (options.fOptionsPriv) {
        fMaxTextureSize = std::min(fMaxTextureSize, options.fOptionsPriv->fMaxTextureSizeOverride);
        fRequestedPathRendererStrategy = options.fOptionsPriv->fPathRendererStrategy;
        fDrawListLayer = options.fOptionsPriv->fDrawListLayer;
    }
#endif
    fGlyphCacheTextureMaximumBytes = options.fGlyphCacheTextureMaximumBytes;
    fMinMSAAPathSize = options.fMinimumPathSizeForMSAA;
    fMinDistanceFieldFontSize = options.fMinDistanceFieldFontSize;
    fGlyphsAsPathsFontSize = options.fGlyphsAsPathsFontSize;
    fMaxPathAtlasTextureSize = options.fMaxPathAtlasTextureSize;
    fAllowMultipleAtlasTextures = options.fAllowMultipleAtlasTextures;
    fSupportBilerpFromGlyphAtlas = options.fSupportBilerpFromGlyphAtlas;
    fRequireOrderedRecordings = options.fRequireOrderedRecordings;
    fSetBackendLabels = options.fSetBackendLabels;
}

sk_sp<SkCapabilities> Caps::capabilities() const { return fCapabilities; }

bool Caps::isSampleCountSupported(TextureFormat format, SampleCount sampleCount) const {
    // Assume optimal tiling
    auto [formatSupport, sampleCounts] = this->getTextureSupport(format, Tiling::kOptimal);
    return SkToBool(formatSupport & TextureUsage::kRender) && SkToBool(sampleCounts & sampleCount);
}

TextureFormat Caps::getDepthStencilFormat(SkEnumBitMask<DepthStencilFlags> dssFlags) const {
    auto canUse = [this](TextureFormat format) {
        auto [formatSupport, sampleCounts] = this->getTextureSupport(format, Tiling::kOptimal);
        // Check that the format can be rendered into and that it supports single-sampled rendering,
        // and if we aren't avoiding MSAA, that it also has some additional sample count.
        return SkToBool(formatSupport & TextureUsage::kRender) &&
               SkToBool(sampleCounts & SampleCount::k1) &&
               (this->avoidMSAA() || sampleCounts != SampleCount::k1);
    };

    if (dssFlags == DepthStencilFlags::kDepth) {
        // Prefer D16, but fallback to D32F or lastly a combined DS format if needed
        if (canUse(TextureFormat::kD16)) {
            return TextureFormat::kD16;
        } else if (canUse(TextureFormat::kD32F)) {
            return TextureFormat::kD32F;
        } else {
            return this->getDepthStencilFormat(DepthStencilFlags::kDepthStencil);
        }
    } else if (dssFlags == DepthStencilFlags::kStencil) {
        // Prefer S8, but fallback to a combined DS format if needed
        if (canUse(TextureFormat::kS8)) {
            return TextureFormat::kS8;
        } else {
            return this->getDepthStencilFormat(DepthStencilFlags::kDepthStencil);
        }
    } else if (dssFlags == DepthStencilFlags::kDepthStencil) {
        // Prefer D24_S8 over D32F_S8 for memory savings if it is available
        if (canUse(TextureFormat::kD24_S8)) {
            return TextureFormat::kD24_S8;
        } else {
            return TextureFormat::kD32F_S8;
        }
    }

    return TextureFormat::kUnsupported; // i.e. no attachment needed
}

bool Caps::isSupported(const TextureInfo& info,
                       SkEnumBitMask<TextureUsage> test,
                       bool allowMSAA,
                       bool allowExternal,
                       bool allowCompressed,
                       bool allowProtected) const {
    const TextureFormat format = TextureInfoPriv::ViewFormat(info);
    if (format == TextureFormat::kUnsupported) {
        return false;
    }
    SkASSERT(info.isValid());

    auto [textureUsage, tiling] = this->getTextureUsage(info);
    auto [formatSupport, supportedSampleCounts] = this->getTextureSupport(format, tiling);

    if (!allowMSAA) {
        // Remove everything but 1x if the operation requires non-MSAA
        supportedSampleCounts &= SampleCount::k1;
    }

    // Intersect what the format and the texture can do to see if `test` is available, and make
    // sure that the texture's sample count is supported.
    if ((formatSupport & textureUsage & test) == test &&
        SkToBool(supportedSampleCounts & info.sampleCount())) {
        // Basic rules that should be reflected in the supported operations bit masks
        SkASSERT((allowProtected  || info.isProtected() == Protected::kNo) &&
                 (allowMSAA       || info.sampleCount() == SampleCount::k1) &&
                 (allowCompressed || TextureFormatCompressionType(format) ==
                                            SkTextureCompressionType::kNone) &&
                 (allowExternal   || format != TextureFormat::kExternal));
        return true;
    } else {
        return false;
    }
}

bool Caps::isTexturable(const TextureInfo& info, bool allowMSAA) const {
    return this->isSupported(info, TextureUsage::kSample,
                             allowMSAA,
                             /*allowExternal=*/true,
                             /*allowCompressed=*/true,
                             /*allowProtected=*/true);
}

bool Caps::isRenderable(const TextureInfo& info) const {
    return this->isSupported(info, TextureUsage::kRender,
                             /*allowMSAA=*/true,
                             /*allowExternal=*/true,
                             /*allowCompressed=*/false,
                             /*allowProtected=*/true);
}

bool Caps::isCopyableSrc(const TextureInfo& info) const {
    return this->isSupported(info, TextureUsage::kCopySrc,
                             /*allowMSAA=*/false,
                             /*allowExternal=*/false,
                             /*allowCompressed=*/false,
                             /*allowProtected=*/false);
}

bool Caps::isCopyableDst(const TextureInfo& info) const {
    return this->isSupported(info, TextureUsage::kCopyDst,
                             /*allowMSAA=*/false,
                             /*allowExternal=*/false,
                             /*allowCompressed=*/true,
                             /*allowProtected=*/true);
}

bool Caps::isStorage(const TextureInfo& info) const {
    return this->isSupported(info, TextureUsage::kStorage,
                             /*allowMSAA=*/false,
                             /*allowExternal=*/false,
                             /*allowCompressed=*/false,
                             /*allowProtected=*/false);
}

bool Caps::isRenderableWithMSRTSS(const TextureInfo& info) const {
    return this->isSupported(info, TextureUsage::kMSRTSS | TextureUsage::kRender,
                             /*allowMSAA=*/true,
                             /*allowExternal=*/true,
                             /*allowCompressed=*/false,
                             /*allowProtected=*/true);
}

bool Caps::areColorTypeAndTextureInfoCompatible(SkColorType ct, const TextureInfo& info) const {
    // TODO: add SkTextureCompressionType handling
    // (can be handled by setting up the colorTypeInfo instead?)

    return SkToBool(this->getColorTypeInfo(ct, info));
}

static inline SkColorType color_type_fallback(SkColorType ct) {
    switch (ct) {
        // kRGBA_8888 is our default fallback for many color types that may not have renderable
        // backend formats.
        case kAlpha_8_SkColorType:
        case kRGB_565_SkColorType:
        case kARGB_4444_SkColorType:
        case kBGRA_8888_SkColorType:
        case kRGBA_1010102_SkColorType:
        case kBGRA_1010102_SkColorType:
        case kRGBA_F16_SkColorType:
        case kRGBA_F16Norm_SkColorType:
            return kRGBA_8888_SkColorType;
        case kA16_float_SkColorType:
            return kRGBA_F16_SkColorType;
        case kGray_8_SkColorType:
        case kRGB_F16F16F16x_SkColorType:
        case kRGB_101010x_SkColorType:
            return kRGB_888x_SkColorType;
        default:
            return kUnknown_SkColorType;
    }
}

const Caps::ColorTypeInfo* Caps::getColorTypeInfo(SkColorType ct, const TextureInfo& info) const {
    if (!info.isValid()) {
        return nullptr;
    }

    for (const ColorTypeInfo& colorInfo : this->getColorTypeInfos(info)) {
        if (colorInfo.fColorType == ct) {
            return &colorInfo;
        }
    }
    return nullptr;
}

SkColorType Caps::getDefaultColorType(const TextureInfo& info) const {
    if (!info.isValid()) {
        return kUnknown_SkColorType;
    }

    const bool isRenderable = this->isRenderable(info);
    for (const ColorTypeInfo& colorInfo : this->getColorTypeInfos(info)) {
        if (!isRenderable || (colorInfo.fFlags & ColorTypeInfo::kRenderable_Flag)) {
            return colorInfo.fColorType;
        }
    }
    return kUnknown_SkColorType;
}

SkColorType Caps::getRenderableColorType(SkColorType ct) const {
    do {
        auto texInfo = this->getDefaultSampledTextureInfo(ct,
                                                          Mipmapped::kNo,
                                                          Protected::kNo,
                                                          Renderable::kYes);
        // We continue to the fallback color type if there is no default renderable format
        if (texInfo.isValid() && this->isRenderable(texInfo)) {
            return ct;
        }
        ct = color_type_fallback(ct);
    } while (ct != kUnknown_SkColorType);
    return kUnknown_SkColorType;
}

SampleCount Caps::getCompatibleMSAASampleCount(const TextureInfo& info) const {
    if (info.sampleCount() > SampleCount::k1) {
        // Use the inherent sample count since it's already MSAA
        return info.sampleCount();
    } else if (!this->avoidMSAA()) {
        // The max internal sample count may be higher than what is universally supported for
        // every renderable TextureFormat, but unless avoidMSAA() was true, this should bottom out
        // at SampleCount::k4.
        TextureFormat format = TextureInfoPriv::ViewFormat(info);
        for (SampleCount s = fMaxInternalSampleCount;
             s > SampleCount::k1;
             s = static_cast<SampleCount>((uint8_t)s >> 1)) {
            if (this->isSampleCountSupported(format, s)) {
                return s;
            }
        }
    }

    // If we got here, MSAA has been disabled somehow (by ContextOption, driver workaround, or
    // no support for a particular TextureFormat).
    return SampleCount::k1;
}

skgpu::Swizzle Caps::getReadSwizzle(SkColorType ct, const TextureInfo& info) const {
    // TODO: add SkTextureCompressionType handling
    // (can be handled by setting up the colorTypeInfo instead?)

    auto colorTypeInfo = this->getColorTypeInfo(ct, info);
    if (!colorTypeInfo) {
        SkDEBUGFAILF("Illegal color type (%d) and format combination.", static_cast<int>(ct));
        return {};
    }

    return colorTypeInfo->fReadSwizzle;
}

skgpu::Swizzle Caps::getWriteSwizzle(SkColorType ct, const TextureInfo& info) const {
    auto colorTypeInfo = this->getColorTypeInfo(ct, info);
    if (!colorTypeInfo) {
        SkDEBUGFAILF("Illegal color type (%d) and format combination.", static_cast<int>(ct));
        return {};
    }

    return colorTypeInfo->fWriteSwizzle;
}

std::pair<SkColorType, bool /*isRGBFormat*/> Caps::supportedTransferColorType(
        SkColorType colorType,
        const TextureInfo& textureInfo) const {
    // NOTE: Compressed textures can't be read back, and external format textures can't be read or
    // written to. However, this is not checked here. Instead that is expected to be handled by
    // supports[Read|Write]Pixels().
    const ColorTypeInfo* colorInfo = this->getColorTypeInfo(colorType, textureInfo);
    if (colorInfo) {
        const TextureFormat format = TextureInfoPriv::ViewFormat(textureInfo);
        const bool rgbRequiresIntervention =
                TextureFormatChannelMask(format) == kRGB_SkColorChannelFlags &&
                colorInfo->fTransferColorType != kRGB_565_SkColorType;
        return {colorInfo->fTransferColorType, rgbRequiresIntervention};
    } else {
        return {kUnknown_SkColorType, false};
    }
}

DstReadStrategy Caps::getDstReadStrategy() const {
    // TODO(b/238757201; b/383769988): Dst reads are currently only supported by FB fetch and
    // texture copy.
    if (this->shaderCaps()->fFBFetchSupport) {
        return DstReadStrategy::kFramebufferFetch;
    } else {
        return DstReadStrategy::kTextureCopy;
    }
}

sktext::gpu::SubRunControl Caps::getSubRunControl(bool useSDFTForSmallText) const {
#if !defined(SK_DISABLE_SDF_TEXT)
    return sktext::gpu::SubRunControl{
            this->shaderCaps()->supportsDistanceFieldText(),
            useSDFTForSmallText,
            true, /*ableToUsePerspectiveSDFT*/
            this->minDistanceFieldFontSize(),
            this->glyphsAsPathsFontSize(),
            true /*forcePathAA*/};
#else
    return sktext::gpu::SubRunControl{/*forcePathAA=*/true};
#endif
}

} // namespace skgpu::graphite
