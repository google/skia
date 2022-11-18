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
#include "src/sksl/SkSLUtil.h"

namespace skgpu::graphite {

Caps::Caps()
        : fShaderCaps(std::make_unique<SkSL::ShaderCaps>())
        , fCapabilities(new SkCapabilities()) {}

Caps::~Caps() {}

void Caps::finishInitialization(const ContextOptions& options) {
    fCapabilities->initSkCaps(fShaderCaps.get());

    if (options.fShaderErrorHandler) {
        fShaderErrorHandler = options.fShaderErrorHandler;
    } else {
        fShaderErrorHandler = DefaultShaderErrorHandler();
    }

#if GRAPHITE_TEST_UTILS
    fMaxTextureAtlasSize = options.fMaxTextureAtlasSize;
#endif
    fGlyphCacheTextureMaximumBytes = options.fGlyphCacheTextureMaximumBytes;
    fMinDistanceFieldFontSize = options.fMinDistanceFieldFontSize;
    fGlyphsAsPathsFontSize = options.fGlyphsAsPathsFontSize;
    fAllowMultipleGlyphCacheTextures = options.fAllowMultipleGlyphCacheTextures;
    fSupportBilerpFromGlyphAtlas = options.fSupportBilerpFromGlyphAtlas;
}

sk_sp<SkCapabilities> Caps::capabilities() const { return fCapabilities; }

bool Caps::isTexturable(const TextureInfo& info) const {
    if (info.numSamples() > 1) {
        return false;
    }
    return this->onIsTexturable(info);
}

bool Caps::areColorTypeAndTextureInfoCompatible(SkColorType ct, const TextureInfo& info) const {
    // TODO: add SkImage::CompressionType handling
    // (can be handled by setting up the colorTypeInfo instead?)

    return SkToBool(this->getColorTypeInfo(ct, info));
}

skgpu::Swizzle Caps::getReadSwizzle(SkColorType ct, const TextureInfo& info) const {
    // TODO: add SkImage::CompressionType handling
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

sktext::gpu::SDFTControl Caps::getSDFTControl(bool useSDFTForSmallText) const {
#if !defined(SK_DISABLE_SDF_TEXT)
    return sktext::gpu::SDFTControl{
            this->shaderCaps()->supportsDistanceFieldText(),
            useSDFTForSmallText,
            true, /*ableToUsePerspectiveSDFT*/
            this->minDistanceFieldFontSize(),
            this->glyphsAsPathsFontSize()};
#else
    return sktext::gpu::SDFTControl{};
#endif
}

} // namespace skgpu::graphite
