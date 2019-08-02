/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnCaps.h"

GrDawnCaps::GrDawnCaps(const GrContextOptions& contextOptions) : INHERITED(contextOptions) {
    fBufferMapThreshold = SK_MaxS32;  // FIXME: get this from Dawn?
    fShaderCaps.reset(new GrShaderCaps(contextOptions));
    fMaxTextureSize = 2048;
    fPerformPartialClearsAsDraws = true;
}

bool GrDawnCaps::isFormatSRGB(const GrBackendFormat& format) const {
    return false;
}

bool GrDawnCaps::isFormatCompressed(const GrBackendFormat& format) const {
    return false;
}

bool GrDawnCaps::isConfigTexturable(GrPixelConfig config) const {
    switch (config) {
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kAlpha_8_GrPixelConfig:
            return true;
        default:
            return false;
    }
}

GrPixelConfig GrDawnCaps::onGetConfigFromBackendFormat(const GrBackendFormat& format,
                                                       GrColorType colorType) const {
    const dawn::TextureFormat* dawnFormat = format.getDawnFormat();
    if (!dawnFormat) {
        return kUnknown_GrPixelConfig;
    }
    switch (colorType) {
        case GrColorType::kUnknown:
            return kUnknown_GrPixelConfig;
        case GrColorType::kAlpha_8:
            if (dawn::TextureFormat::R8Unorm == *dawnFormat) {
                return kAlpha_8_as_Red_GrPixelConfig;
            }
            break;
        case GrColorType::kRGBA_8888:
            if (dawn::TextureFormat::RGBA8Unorm == *dawnFormat) {
                return kRGBA_8888_GrPixelConfig;
            }
            break;
        case GrColorType::kRGB_888x:
            break;
        case GrColorType::kBGRA_8888:
            if (dawn::TextureFormat::BGRA8Unorm == *dawnFormat) {
                return kBGRA_8888_GrPixelConfig;
            }
            break;
        default:
            break;
    }
    return kUnknown_GrPixelConfig;
}

static GrSwizzle get_swizzle(const GrBackendFormat& format, GrColorType colorType,
                             bool forOutput) {
    SkASSERT(format.getDawnFormat());

    switch (colorType) {
        case GrColorType::kAlpha_8: // fall through
        case GrColorType::kAlpha_F16:
            if (forOutput) {
                return GrSwizzle::AAAA();
            } else {
                return GrSwizzle::RRRR();
            }
        case GrColorType::kGray_8:
            if (!forOutput) {
                return GrSwizzle::RRRA();
            }
            break;
        case GrColorType::kRGB_888x:
            if (!forOutput) {
                return GrSwizzle::RGB1();
            }
        default:
            return GrSwizzle::RGBA();
    }
    return GrSwizzle::RGBA();
}

bool GrDawnCaps::isFormatTexturable(GrColorType ct, const GrBackendFormat& format) const {
    GrPixelConfig config = this->getConfigFromBackendFormat(format, ct);
    if (kUnknown_GrPixelConfig == config) {
        return false;
    }

    return this->isConfigTexturable(config);
}

bool GrDawnCaps::isFormatRenderable(GrColorType ct, const GrBackendFormat& format,
                                    int sampleCount) const {
    if (!format.isValid() || sampleCount > 1) {
        return false;
    }

    return GrDawnFormatIsRenderable(*format.getDawnFormat()) ? 1 : 0;
}

int GrDawnCaps::getRenderTargetSampleCount(int requestedCount, GrColorType ct,
                                           const GrBackendFormat& backendFormat) const {
    if (!backendFormat.getDawnFormat()) {
        return 0;
    }
    return GrDawnFormatIsRenderable(*backendFormat.getDawnFormat()) ? 1 : 0;
}

int GrDawnCaps::getRenderTargetSampleCount(int requestedCount, GrPixelConfig config) const {
    dawn::TextureFormat format;
    if (!GrPixelConfigToDawnFormat(config, &format)) {
        return 0;
    }
    return GrDawnFormatIsRenderable(format) ? 1 : 0;
}

int GrDawnCaps::maxRenderTargetSampleCount(const GrBackendFormat& format) const {
    return format.isValid() ? 1 : 0;
}

GrBackendFormat GrDawnCaps::onGetDefaultBackendFormat(GrColorType ct,
                                                      GrRenderable renderable) const {
    GrPixelConfig config = GrColorTypeToPixelConfig(ct);
    if (config == kUnknown_GrPixelConfig) {
        return GrBackendFormat();
    }
    dawn::TextureFormat format;
    if (!GrPixelConfigToDawnFormat(config, &format)) {
        return GrBackendFormat();
    }
    return GrBackendFormat::MakeDawn(format);
}

GrBackendFormat GrDawnCaps::getBackendFormatFromCompressionType(SkImage::CompressionType type) const
{
    return GrBackendFormat();
}

GrSwizzle GrDawnCaps::getTextureSwizzle(const GrBackendFormat& format, GrColorType colorType) const
{
    return get_swizzle(format, colorType, false);
}

bool GrDawnCaps::canClearTextureOnCreation() const {
    return true;
}

GrSwizzle GrDawnCaps::getOutputSwizzle(const GrBackendFormat& format, GrColorType colorType) const
{
    return get_swizzle(format, colorType, true);
}

bool GrDawnCaps::onAreColorTypeAndFormatCompatible(GrColorType ct,
                                                   const GrBackendFormat& format) const {
    return true;
}

GrColorType GrDawnCaps::getYUVAColorTypeFromBackendFormat(const GrBackendFormat&,
                                                          bool isAlphaChannel) const {
    return GrColorType::kUnknown;
}

#if GR_TEST_UTILS
std::vector<GrCaps::TestFormatColorTypeCombination> GrDawnCaps::getTestingCombinations() const {
    std::vector<GrCaps::TestFormatColorTypeCombination> combos = {
        { GrColorType::kAlpha_8,   GrBackendFormat::MakeDawn(dawn::TextureFormat::R8Unorm)    },
        { GrColorType::kRGBA_8888, GrBackendFormat::MakeDawn(dawn::TextureFormat::RGBA8Unorm) },
        { GrColorType::kRGB_888x,  GrBackendFormat::MakeDawn(dawn::TextureFormat::RGBA8Unorm) },
        { GrColorType::kBGRA_8888, GrBackendFormat::MakeDawn(dawn::TextureFormat::BGRA8Unorm) },
    };

#ifdef SK_DEBUG
    for (auto combo : combos) {
        SkASSERT(this->onAreColorTypeAndFormatCompatible(combo.fColorType, combo.fFormat));
    }
#endif
    return combos;
}
#endif
