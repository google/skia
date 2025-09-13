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
#include "src/sksl/SkSLUtil.h"

#include <algorithm>

namespace skgpu::graphite {

Caps::Caps()
        : fShaderCaps(std::make_unique<SkSL::ShaderCaps>())
        , fCapabilities(new SkCapabilities()) {}

Caps::~Caps() {}

void Caps::finishInitialization(const ContextOptions& options) {
    fCapabilities->initSkCaps(fShaderCaps.get());

    fDefaultMSAASamples = options.fInternalMultisampleCount;

    if (options.fShaderErrorHandler) {
        fShaderErrorHandler = options.fShaderErrorHandler;
    } else {
        fShaderErrorHandler = DefaultShaderErrorHandler();
    }

#if defined(GPU_TEST_UTILS)
    if (options.fOptionsPriv) {
        fMaxTextureSize = std::min(fMaxTextureSize, options.fOptionsPriv->fMaxTextureSizeOverride);
        fMaxTextureAtlasSize = options.fOptionsPriv->fMaxTextureAtlasSize;
        fRequestedPathRendererStrategy = options.fOptionsPriv->fPathRendererStrategy;
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

SkISize Caps::getDepthAttachmentDimensions(const TextureInfo& textureInfo,
                                           const SkISize colorAttachmentDimensions) const {
    return colorAttachmentDimensions;
}

bool Caps::isTexturable(const TextureInfo& info) const {
    if (info.numSamples() > 1) {
        return false;
    }
    return this->onIsTexturable(info);
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
