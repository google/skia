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
#include "src/gpu/GrStencilSettings.h"

GrDawnCaps::GrDawnCaps(const GrContextOptions& contextOptions) : INHERITED(contextOptions) {
    fMipmapSupport = true;
    fBufferMapThreshold = SK_MaxS32;  // FIXME: get this from Dawn?
    fShaderCaps.reset(new GrShaderCaps(contextOptions));
    fMaxTextureSize = fMaxRenderTargetSize = 8192; // FIXME
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

bool GrDawnCaps::isFormatTexturable(const GrBackendFormat& format, GrTextureType) const {
    // Currently, all the formats in GrDawnFormatToPixelConfig are texturable.
    wgpu::TextureFormat dawnFormat;
    return format.asDawnFormat(&dawnFormat);
}

static GrSwizzle get_swizzle(const GrBackendFormat& format, GrColorType colorType,
                             bool forOutput) {
    switch (colorType) {
        case GrColorType::kAlpha_8: // fall through
        case GrColorType::kAlpha_F16:
            if (forOutput) {
                return GrSwizzle("a000");
            } else {
                return GrSwizzle("000r");
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
            break;
        default:
            return GrSwizzle::RGBA();
    }
    return GrSwizzle::RGBA();
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

GrCaps::SurfaceReadPixelsSupport GrDawnCaps::surfaceSupportsReadPixels(
      const GrSurface* surface) const {
    // We currently support readbacks only from Textures and TextureRenderTargets.
    return surface->asTexture() ? SurfaceReadPixelsSupport::kSupported
                                : SurfaceReadPixelsSupport::kUnsupported;
}

bool GrDawnCaps::onSurfaceSupportsWritePixels(const GrSurface* surface) const {
    // We currently support writePixels only to Textures and TextureRenderTargets.
    return surface->asTexture() != nullptr;
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

GrBackendFormat GrDawnCaps::onGetDefaultBackendFormat(GrColorType ct) const {
    wgpu::TextureFormat format;
    if (!GrColorTypeToDawnFormat(ct, &format)) {
        return {};
    }
    return GrBackendFormat::MakeDawn(format);
}

GrBackendFormat GrDawnCaps::getBackendFormatFromCompressionType(SkImage::CompressionType type) const
{
    return GrBackendFormat();
}

GrSwizzle GrDawnCaps::onGetReadSwizzle(const GrBackendFormat& format, GrColorType colorType) const
{
    return get_swizzle(format, colorType, false);
}

GrSwizzle GrDawnCaps::getWriteSwizzle(const GrBackendFormat& format, GrColorType colorType) const {
    return get_swizzle(format, colorType, true);
}

uint64_t GrDawnCaps::computeFormatKey(const GrBackendFormat& format) const {
    wgpu::TextureFormat dawnFormat;
    SkAssertResult(format.asDawnFormat(&dawnFormat));

    // Dawn max enum value should always fit in 32 bits.

    // disabled: no member named 'WGPUTextureFormat_Force32' in namespace 'wgpu'
    //SkASSERT(dawnFormat <= wgpu::WGPUTextureFormat_Force32);
    return (uint64_t)dawnFormat;
}

bool GrDawnCaps::onAreColorTypeAndFormatCompatible(GrColorType ct,
                                                   const GrBackendFormat& format) const {
    return true;
}

// FIXME: taken from GrVkPipelineState; refactor.
static uint32_t get_blend_info_key(const GrPipeline& pipeline) {
    GrXferProcessor::BlendInfo blendInfo = pipeline.getXferProcessor().getBlendInfo();

    static const uint32_t kBlendWriteShift = 1;
    static const uint32_t kBlendCoeffShift = 5;
    static_assert(kLast_GrBlendCoeff < (1 << kBlendCoeffShift));
    static_assert(kFirstAdvancedGrBlendEquation - 1 < 4);

    uint32_t key = blendInfo.fWriteColor;
    key |= (blendInfo.fSrcBlend << kBlendWriteShift);
    key |= (blendInfo.fDstBlend << (kBlendWriteShift + kBlendCoeffShift));
    key |= (blendInfo.fEquation << (kBlendWriteShift + 2 * kBlendCoeffShift));

    return key;
}

GrProgramDesc GrDawnCaps::makeDesc(GrRenderTarget* rt,
                                   const GrProgramInfo& programInfo,
                                   ProgramDescOverrideFlags overrideFlags) const {
    SkASSERT(overrideFlags == ProgramDescOverrideFlags::kNone);
    GrProgramDesc desc;
    GrProgramDesc::Build(&desc, programInfo, *this);

    wgpu::TextureFormat format;
    if (!programInfo.backendFormat().asDawnFormat(&format)) {
        desc.reset();
        SkASSERT(!desc.isValid());
        return desc;
    }

    GrProcessorKeyBuilder b(desc.key());
    GrStencilSettings stencil = programInfo.nonGLStencilSettings();
    stencil.genKey(&b, true);

    // TODO: remove this reliance on the renderTarget
    bool hasDepthStencil = rt->getStencilAttachment() != nullptr;

    b.add32(static_cast<uint32_t>(format));
    b.add32(static_cast<int32_t>(hasDepthStencil));
    b.add32(get_blend_info_key(programInfo.pipeline()));
    b.add32(programInfo.primitiveTypeKey());

    b.flush();
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
    for (const GrCaps::TestFormatColorTypeCombination& combo : combos) {
        SkASSERT(this->onAreColorTypeAndFormatCompatible(combo.fColorType, combo.fFormat));
    }
#endif
    return combos;
}
#endif
