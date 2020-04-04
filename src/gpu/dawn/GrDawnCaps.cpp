/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnCaps.h"

#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrStencilSettings.h"

GrDawnCaps::GrDawnCaps(const GrContextOptions& contextOptions) : INHERITED(contextOptions) {
    fMipMapSupport = true;
    fBufferMapThreshold = SK_MaxS32;  // FIXME: get this from Dawn?
    fShaderCaps.reset(new GrShaderCaps(contextOptions));
    fMaxTextureSize = fMaxRenderTargetSize = 4096; // FIXME
    fMaxVertexAttributes = 16; // FIXME
    fClampToBorderSupport = false;
    fPerformPartialClearsAsDraws = true;
    fDynamicStateArrayGeometryProcessorTextureSupport = true;

    fShaderCaps->fFlatInterpolationSupport = true;
    fShaderCaps->fIntegerSupport = true;
    // FIXME: each fragment sampler takes two binding slots in Dawn (sampler + texture). Limit to
    // 6 * 2 = 12, since kMaxBindingsPerGroup is 16 in Dawn, and we need to keep a few for
    // non-texture bindings. Eventually, we may be able to increase kMaxBindingsPerGroup in Dawn.
    fShaderCaps->fMaxFragmentSamplers = 6;
    fShaderCaps->fShaderDerivativeSupport = true;

    this->finishInitialization(contextOptions);
}

bool GrDawnCaps::isFormatSRGB(const GrBackendFormat& format) const {
    return false;
}

bool GrDawnCaps::isFormatCompressed(const GrBackendFormat& format,
                                    SkImage::CompressionType* compressionType) const {
    return false;
}

bool GrDawnCaps::isFormatTexturable(const GrBackendFormat& format) const {
    // Currently, all the formats in GrDawnFormatToPixelConfig are texturable.
    wgpu::TextureFormat dawnFormat;
    return format.asDawnFormat(&dawnFormat);
}

GrPixelConfig GrDawnCaps::onGetConfigFromBackendFormat(const GrBackendFormat& format,
                                                       GrColorType colorType) const {
    wgpu::TextureFormat dawnFormat;
    if (!format.asDawnFormat(&dawnFormat)) {
        return kUnknown_GrPixelConfig;
    }
    switch (colorType) {
        case GrColorType::kUnknown:
            return kUnknown_GrPixelConfig;
        case GrColorType::kAlpha_8:
            if (wgpu::TextureFormat::R8Unorm == dawnFormat) {
                return kAlpha_8_as_Red_GrPixelConfig;
            }
            break;
        case GrColorType::kRGBA_8888:
            if (wgpu::TextureFormat::RGBA8Unorm == dawnFormat) {
                return kRGBA_8888_GrPixelConfig;
            } else if (wgpu::TextureFormat::BGRA8Unorm == dawnFormat) {
                // FIXME: This shouldn't be necessary, but on some platforms (Mac)
                // Skia byte order is RGBA, while preferred swap format is BGRA.
                return kBGRA_8888_GrPixelConfig;
            }
            break;
        case GrColorType::kRGB_888x:
            break;
        case GrColorType::kBGRA_8888:
            if (wgpu::TextureFormat::BGRA8Unorm == dawnFormat) {
                return kBGRA_8888_GrPixelConfig;
            } else if (wgpu::TextureFormat::RGBA8Unorm == dawnFormat) {
                return kRGBA_8888_GrPixelConfig;
            }
            break;
        default:
            break;
    }
    return kUnknown_GrPixelConfig;
}

static GrSwizzle get_swizzle(const GrBackendFormat& format, GrColorType colorType,
                             bool forOutput) {
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

bool GrDawnCaps::isFormatTexturableAndUploadable(GrColorType ct,
                                                 const GrBackendFormat& format) const {
    wgpu::TextureFormat dawnFormat;
    if (!format.asDawnFormat(&dawnFormat)) {
        return false;
    }
    switch (ct) {
        case GrColorType::kAlpha_8:
            return wgpu::TextureFormat::R8Unorm == dawnFormat;
        case GrColorType::kRGBA_8888:
        case GrColorType::kRGB_888x:
        case GrColorType::kBGRA_8888:
            return wgpu::TextureFormat::RGBA8Unorm == dawnFormat ||
                   wgpu::TextureFormat::BGRA8Unorm == dawnFormat;
        default:
            return false;
    }
}

bool GrDawnCaps::isFormatRenderable(const GrBackendFormat& format,
                                    int sampleCount) const {
    wgpu::TextureFormat dawnFormat;
    if (!format.isValid() || sampleCount > 1 || !format.asDawnFormat(&dawnFormat)) {
        return false;
    }

    return GrDawnFormatIsRenderable(dawnFormat);
}

bool GrDawnCaps::isFormatAsColorTypeRenderable(GrColorType ct, const GrBackendFormat& format,
                                               int sampleCount) const {
    return isFormatRenderable(format, sampleCount);
}

size_t GrDawnCaps::bytesPerPixel(const GrBackendFormat& backendFormat) const {
    wgpu::TextureFormat dawnFormat;
    if (!backendFormat.asDawnFormat(&dawnFormat)) {
        return 0;
    }
    return GrDawnBytesPerPixel(dawnFormat);
}

int GrDawnCaps::getRenderTargetSampleCount(int requestedCount,
                                           const GrBackendFormat& backendFormat) const {
    wgpu::TextureFormat dawnFormat;
    if (!backendFormat.asDawnFormat(&dawnFormat)) {
        return 0;
    }
    return GrDawnFormatIsRenderable(dawnFormat) ? 1 : 0;
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
    wgpu::TextureFormat format;
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

GrSwizzle GrDawnCaps::getOutputSwizzle(const GrBackendFormat& format, GrColorType colorType) const
{
    return get_swizzle(format, colorType, true);
}

bool GrDawnCaps::onAreColorTypeAndFormatCompatible(GrColorType ct,
                                                   const GrBackendFormat& format) const {
    return true;
}

GrColorType GrDawnCaps::getYUVAColorTypeFromBackendFormat(const GrBackendFormat& backendFormat,
                                                          bool isAlphaChannel) const {
    wgpu::TextureFormat textureFormat;
    if (!backendFormat.asDawnFormat(&textureFormat)) {
        return GrColorType::kUnknown;
    }
    switch (textureFormat) {
        case wgpu::TextureFormat::R8Unorm:     return isAlphaChannel ? GrColorType::kAlpha_8
                                                                     : GrColorType::kGray_8;
        case wgpu::TextureFormat::RGBA8Unorm:  return GrColorType::kRGBA_8888;
        case wgpu::TextureFormat::BGRA8Unorm:  return GrColorType::kBGRA_8888;
        default:                               return GrColorType::kUnknown;
    }
}

// FIXME: taken from GrVkPipelineState; refactor.
static uint32_t get_blend_info_key(const GrPipeline& pipeline) {
    GrXferProcessor::BlendInfo blendInfo = pipeline.getXferProcessor().getBlendInfo();

    static const uint32_t kBlendWriteShift = 1;
    static const uint32_t kBlendCoeffShift = 5;
    GR_STATIC_ASSERT(kLast_GrBlendCoeff < (1 << kBlendCoeffShift));
    GR_STATIC_ASSERT(kFirstAdvancedGrBlendEquation - 1 < 4);

    uint32_t key = blendInfo.fWriteColor;
    key |= (blendInfo.fSrcBlend << kBlendWriteShift);
    key |= (blendInfo.fDstBlend << (kBlendWriteShift + kBlendCoeffShift));
    key |= (blendInfo.fEquation << (kBlendWriteShift + 2 * kBlendCoeffShift));

    return key;
}

GrProgramDesc GrDawnCaps::makeDesc(const GrRenderTarget* rt,
                                   const GrProgramInfo& programInfo) const {
    GrProgramDesc desc;
    if (!GrProgramDesc::Build(&desc, rt, programInfo, *this)) {
        SkASSERT(!desc.isValid());
        return desc;
    }

    wgpu::TextureFormat format;
    if (!programInfo.backendFormat().asDawnFormat(&format)) {
        desc.key().reset();
        SkASSERT(!desc.isValid());
        return desc;
    }

    GrProcessorKeyBuilder b(&desc.key());

    GrStencilSettings stencil = programInfo.nonGLStencilSettings();
    stencil.genKey(&b);

    // TODO: remove this reliance on the renderTarget
    bool hasDepthStencil = rt->renderTargetPriv().getStencilAttachment() != nullptr;

    b.add32(static_cast<uint32_t>(format));
    b.add32(static_cast<int32_t>(hasDepthStencil));
    b.add32(get_blend_info_key(programInfo.pipeline()));
    b.add32(static_cast<uint32_t>(programInfo.primitiveType()));
    return desc;
}

#if GR_TEST_UTILS
std::vector<GrCaps::TestFormatColorTypeCombination> GrDawnCaps::getTestingCombinations() const {
    std::vector<GrCaps::TestFormatColorTypeCombination> combos = {
        { GrColorType::kAlpha_8,   GrBackendFormat::MakeDawn(wgpu::TextureFormat::R8Unorm)    },
        { GrColorType::kRGBA_8888, GrBackendFormat::MakeDawn(wgpu::TextureFormat::RGBA8Unorm) },
        { GrColorType::kRGBA_8888, GrBackendFormat::MakeDawn(wgpu::TextureFormat::BGRA8Unorm) },
        { GrColorType::kRGB_888x,  GrBackendFormat::MakeDawn(wgpu::TextureFormat::RGBA8Unorm) },
        { GrColorType::kRGB_888x,  GrBackendFormat::MakeDawn(wgpu::TextureFormat::BGRA8Unorm) },
        { GrColorType::kBGRA_8888, GrBackendFormat::MakeDawn(wgpu::TextureFormat::BGRA8Unorm) },
        { GrColorType::kBGRA_8888, GrBackendFormat::MakeDawn(wgpu::TextureFormat::RGBA8Unorm) },
    };

#ifdef SK_DEBUG
    for (auto combo : combos) {
        SkASSERT(this->onAreColorTypeAndFormatCompatible(combo.fColorType, combo.fFormat));
    }
#endif
    return combos;
}
#endif
