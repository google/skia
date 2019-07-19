/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDawnCaps.h"

GrDawnCaps::GrDawnCaps(const GrContextOptions& contextOptions) : INHERITED(contextOptions) {
    fBufferMapThreshold = SK_MaxS32;  // FIXME: get this from Dawn?
    fShaderCaps.reset(new GrShaderCaps(contextOptions));
    fMaxTextureSize = 2048;
    fPerformPartialClearsAsDraws = true;
}

bool GrDawnCaps::isFormatSRGB(const GrBackendFormat& format) const {
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
    dawn::TextureFormat textureFormat = *format.getDawnFormat();
    switch (colorType) {
        case GrColorType::kUnknown:
            return kUnknown_GrPixelConfig;
        case GrColorType::kAlpha_8:
            if (dawn::TextureFormat::R8Unorm == textureFormat) {
                return kAlpha_8_as_Red_GrPixelConfig;
            }
            break;
        case GrColorType::kRGBA_8888:
            if (dawn::TextureFormat::RGBA8Unorm == textureFormat) {
                return kRGBA_8888_GrPixelConfig;
            }
            break;
        case GrColorType::kRGB_888x:
            break;
        case GrColorType::kBGRA_8888:
            if (dawn::TextureFormat::BGRA8Unorm == textureFormat) {
                return kBGRA_8888_GrPixelConfig;
            }
            break;
        default:
            break;
    }
    return kUnknown_GrPixelConfig;
}

GrPixelConfig GrDawnCaps::getYUVAConfigFromBackendFormat(const GrBackendFormat& backendFormat)
        const {
    const dawn::TextureFormat* format = backendFormat.getDawnFormat();
    if (!format) {
        return kUnknown_GrPixelConfig;
    }
    switch (*format) {
        case dawn::TextureFormat::R8Unorm:
            return kAlpha_8_as_Red_GrPixelConfig;
            break;
        case dawn::TextureFormat::RGBA8Unorm:
            return kRGBA_8888_GrPixelConfig;
            break;
        case dawn::TextureFormat::BGRA8Unorm:
            return kBGRA_8888_GrPixelConfig;
            break;
        default:
            return kUnknown_GrPixelConfig;
            break;
    }
}

size_t GrDawnCaps::onTransferFromOffsetAlignment(GrColorType bufferColorType) const {
    if (bufferColorType == GrColorType::kRGB_888x) {
        return false;
    }
    size_t bpp = GrColorTypeBytesPerPixel(bufferColorType);
    switch (bpp & 0b11) {
        case 0:     return bpp;
        case 2:     return 2 * bpp;
        default:    return 4 * bpp;
    }
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

bool GrDawnCaps::isFormatCopyable(GrColorType ct, const GrBackendFormat& format) const {
    return true;
}

int GrDawnCaps::getRenderTargetSampleCount(int requestedCount, GrColorType ct,
                                           const GrBackendFormat& format) const {
    GrPixelConfig config = this->getConfigFromBackendFormat(format, ct);
    if (kUnknown_GrPixelConfig == config) {
        return 0;
    }

    return this->getRenderTargetSampleCount(requestedCount, config);
}

GrBackendFormat GrDawnCaps::getBackendFormatFromColorType(GrColorType ct) const {
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
