/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDawnCaps.h"

GrDawnCaps::GrDawnCaps(const GrContextOptions& contextOptions) : INHERITED(contextOptions) {
    fBufferMapThreshold = SK_MaxS32;  // FIXME: get this from Dawn?
    fShaderCaps.reset(new GrShaderCaps(contextOptions));
    fMaxTextureSize = fMaxRenderTargetSize = 4096; // FIXME
    fMaxVertexAttributes = 16; // FIXME
    fClampToBorderSupport = false;
    fPerformPartialClearsAsDraws = true;

    fShaderCaps->fFlatInterpolationSupport = true;
    fShaderCaps->fIntegerSupport = true;
    // FIXME: each fragment sampler takes two binding slots in Dawn (sampler + texture). Limit to
    // 6 * 2 = 12, since kMaxBindingsPerGroup is 16 in Dawn, and we need to keep a few for
    // non-texture bindings. Eventually, we may be able to increase kMaxBindingsPerGroup in Dawn.
    fShaderCaps->fMaxFragmentSamplers = 6;
    fShaderCaps->fConfigTextureSwizzle[kAlpha_8_GrPixelConfig] = GrSwizzle::RRRR();
    fShaderCaps->fShaderDerivativeSupport = true;

    this->applyOptionsOverrides(contextOptions);
    fShaderCaps->applyOptionsOverrides(contextOptions);
}

bool GrDawnCaps::isConfigTexturable(GrPixelConfig config) const {
    switch (config) {
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kAlpha_8_GrPixelConfig:
            return true;
        default:
            return false;
    }
}

GrPixelConfig GrDawnCaps::getConfigFromBackendFormat(const GrBackendFormat& format,
                                                     SkColorType colorType) const {
    dawn::TextureFormat textureFormat = *format.getDawnFormat();
    switch (colorType) {
        case kUnknown_SkColorType:
            return kUnknown_GrPixelConfig;
        case kAlpha_8_SkColorType:
            if (dawn::TextureFormat::R8Unorm == textureFormat) {
                return kAlpha_8_as_Red_GrPixelConfig;
            }
            break;
        case kRGBA_8888_SkColorType:
            if (dawn::TextureFormat::R8G8B8A8Unorm == textureFormat) {
                return kRGBA_8888_GrPixelConfig;
            } else if (dawn::TextureFormat::B8G8R8A8Unorm == textureFormat) {
                // FIXME: This shouldn't be necessary, but on some platforms (Mac)
                // Skia byte order is RGBA, while preferred swap format is BGRA.
                return kBGRA_8888_GrPixelConfig;
            }
            break;
        case kRGB_888x_SkColorType:
            break;
        case kBGRA_8888_SkColorType:
            if (dawn::TextureFormat::B8G8R8A8Unorm == textureFormat) {
                return kBGRA_8888_GrPixelConfig;
            } else if (dawn::TextureFormat::R8G8B8A8Unorm == textureFormat) {
                return kRGBA_8888_GrPixelConfig;
            }
            break;
        case kGray_8_SkColorType:
        case kARGB_4444_SkColorType:
        case kRGB_565_SkColorType:
        case kRGBA_1010102_SkColorType:
        case kRGB_101010x_SkColorType:
        case kRGBA_F16_SkColorType:
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F32_SkColorType:
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
        case dawn::TextureFormat::R8G8B8A8Unorm:
            return kRGBA_8888_GrPixelConfig;
            break;
        case dawn::TextureFormat::B8G8R8A8Unorm:
            return kBGRA_8888_GrPixelConfig;
            break;
        default:
            return kUnknown_GrPixelConfig;
            break;
    }
}

